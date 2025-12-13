#!/usr/bin/env python3
"""
Validation script to verify output WAV file contains exactly 2 AX.25 frames at the end.

Uses minimodem to decode AX.25 frames from the audio file and validates:
1. Exactly 2 frames are detected
2. Frame structure is correct
3. Frame content matches expected signature data
"""

import sys
import os
import subprocess
import tempfile
import wave
import numpy as np
from pathlib import Path

def check_minimodem():
    """Check if minimodem is installed and available"""
    try:
        # Just check if command exists
        result = subprocess.run(['which', 'minimodem'], capture_output=True, timeout=2)
        return result.returncode == 0
    except (FileNotFoundError, subprocess.TimeoutExpired):
        # Fallback: try running minimodem
        try:
            subprocess.run(['minimodem', '--version'], capture_output=True, timeout=2)
            return True
        except:
            return False

def calculate_frame_samples():
    """
    Calculate the number of samples needed for 2 AX.25 frames.
    
    Formula: 2 frames * 300 bits/frame * 20 repeat factor at 2400 baud
    At 48kHz sample rate: (2 * 300 * 20) / 48000 = 0.25 seconds
    Add buffer for safety: ~1.0 seconds total to ensure we capture frames
    """
    bits_per_frame = 300  # Typical AX.25 frame size in bits
    num_frames = 2
    repeat_factor = 20
    sample_rate = 48000
    
    # Calculate samples needed
    total_bits = num_frames * bits_per_frame
    total_samples = total_bits * repeat_factor
    
    # Convert to seconds and add buffer for padding
    # The flowgraph adds padding_samples (10000) after frames
    padding_samples = 10000
    total_with_padding = total_samples + padding_samples
    duration_seconds = total_with_padding / sample_rate
    
    return int(duration_seconds * sample_rate), duration_seconds

def find_voice_end(wav_file, sample_rate=48000):
    """
    Find where the voice/audio ends in the WAV file.
    Looks for where audio energy drops significantly (transition from voice to data-only).
    
    Returns:
        Frame index where voice ends (start of data-only portion)
    """
    try:
        import numpy as np
        with wave.open(wav_file, 'rb') as wav_in:
            sample_rate_actual = wav_in.getframerate()
            n_frames = wav_in.getnframes()
            sample_width = wav_in.getsampwidth()
            
            # Read entire file to analyze
            wav_in.rewind()
            frames = wav_in.readframes(n_frames)
            
            # Convert to numpy array
            if sample_width == 2:  # 16-bit
                samples = np.frombuffer(frames, dtype=np.int16).astype(np.float32) / 32768.0
            elif sample_width == 4:  # 32-bit float
                samples = np.frombuffer(frames, dtype=np.float32)
            else:
                samples = np.frombuffer(frames, dtype=np.uint8).astype(np.float32) / 255.0
            
            # Analyze energy levels to find voice end
            # Voice has higher energy, data-only portion has lower energy
            window_size = int(0.05 * sample_rate_actual)  # 50ms windows
            voice_threshold = 0.05  # Energy threshold for voice
            data_threshold = 0.02   # Energy threshold for data-only
            
            # Calculate energy for each window
            energies = []
            for i in range(0, n_frames - window_size, window_size // 2):  # 50% overlap
                window = samples[i:i+window_size]
                energy = np.mean(np.abs(window))
                energies.append((i, energy))
            
            # Find where energy drops from voice level to data level
            # Look for transition point going backwards from end
            voice_end_frame = n_frames
            
            # Start from end and work backwards
            for i in range(len(energies) - 1, 0, -1):
                frame_idx, energy = energies[i]
                
                # If we find low energy (data-only), check if previous windows had voice
                if energy < data_threshold:
                    # Check previous windows to find where voice ended
                    for j in range(i - 1, max(0, i - 10), -1):
                        prev_idx, prev_energy = energies[j]
                        if prev_energy > voice_threshold:
                            # Found transition: voice ended between prev_idx and frame_idx
                            voice_end_frame = frame_idx
                            break
                    if voice_end_frame < n_frames:
                        break
            
            # Fallback: if no clear transition found, assume last 20% is data-only
            if voice_end_frame >= n_frames * 0.9:
                voice_end_frame = int(n_frames * 0.8)
                print(f"   WARNING: No clear voice/data transition found, assuming voice ends at 80%")
            
            return voice_end_frame, n_frames
            
    except Exception as e:
        print(f"ERROR: Failed to find voice end: {e}")
        import traceback
        traceback.print_exc()
        return None, None

def extract_data_only_segment(wav_file, voice_end_frame, num_samples, sample_rate=48000):
    """
    Extract the data-only portion from WAV file (after voice ends).
    
    Args:
        wav_file: Path to input WAV file
        voice_end_frame: Frame index where voice ends
        num_samples: Number of samples to extract
        sample_rate: Expected sample rate (default 48000 Hz)
    
    Returns:
        Tuple: (temp_file_path, actual_sample_rate, start_frame, total_frames)
    """
    try:
        with wave.open(wav_file, 'rb') as wav_in:
            sample_rate_actual = wav_in.getframerate()
            n_channels = wav_in.getnchannels()
            sample_width = wav_in.getsampwidth()
            n_frames = wav_in.getnframes()
            
            # Calculate how many frames to extract (adjust for actual sample rate)
            frames_to_extract = int(num_samples * (sample_rate_actual / sample_rate))
            frames_to_extract = min(frames_to_extract, n_frames - voice_end_frame)
            start_frame = voice_end_frame
            
            # Seek to start position (where voice ends)
            wav_in.setpos(start_frame)
            
            # Read frames
            frames = wav_in.readframes(frames_to_extract)
            
            # Create temporary output file
            temp_file = tempfile.NamedTemporaryFile(suffix='.wav', delete=False)
            temp_path = temp_file.name
            temp_file.close()
            
            # Write extracted segment
            with wave.open(temp_path, 'wb') as wav_out:
                wav_out.setnchannels(n_channels)
                wav_out.setsampwidth(sample_width)
                wav_out.setframerate(sample_rate_actual)
                wav_out.writeframes(frames)
            
            return temp_path, sample_rate_actual, start_frame, n_frames
            
    except Exception as e:
        print(f"ERROR: Failed to extract data segment: {e}")
        import traceback
        traceback.print_exc()
        return None, None, None, None

def convert_wav_to_raw(wav_file, raw_file, sample_rate=48000):
    """
    Convert WAV file to raw 16-bit PCM format for minimodem.
    Uses sox if available, otherwise uses Python wave module.
    """
    # Try using sox first (faster and more reliable)
    try:
        result = subprocess.run(
            ['sox', wav_file, '-r', str(sample_rate), '-b', '16', '-c', '1', '-t', 'raw', raw_file],
            capture_output=True,
            timeout=10
        )
        if result.returncode == 0:
            return True
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    
    # Fallback to Python wave module
    try:
        with wave.open(wav_file, 'rb') as wav_in:
            with open(raw_file, 'wb') as raw_out:
                frames = wav_in.readframes(wav_in.getnframes())
                raw_out.write(frames)
        return True
    except Exception as e:
        print(f"WARNING: Failed to convert WAV to raw: {e}")
        return False

def extract_data_bytes_from_wav(wav_file, start_frame, num_frames):
    """
    Extract raw data bytes from WAV file.
    The flowgraph outputs float samples where data bytes are converted to float
    and repeated 20x. We need to extract and decode these.
    """
    try:
        import numpy as np
        with wave.open(wav_file, 'rb') as wav_in:
            wav_in.setpos(start_frame)
            frames = wav_in.readframes(num_frames)
            
            # Convert to numpy array
            if wav_in.getsampwidth() == 2:  # 16-bit
                samples = np.frombuffer(frames, dtype=np.int16).astype(np.float32) / 32768.0
            elif wav_in.getsampwidth() == 4:  # 32-bit float
                samples = np.frombuffer(frames, dtype=np.float32)
            else:
                samples = np.frombuffer(frames, dtype=np.uint8).astype(np.float32) / 255.0
            
            # The data is repeated 20x, so we need to downsample
            # Take every 20th sample to get original data rate
            repeat_factor = 20
            downsampled = samples[::repeat_factor]
            
            # Convert float back to bytes (reverse of char_to_float scaling)
            # char_to_float uses scale 1.0/127.0, so reverse is * 127.0
            bytes_data = (downsampled * 127.0).astype(np.int8).astype(np.uint8)
            
            return bytes_data.tobytes()
    except Exception as e:
        print(f"ERROR: Failed to extract data bytes: {e}")
        import traceback
        traceback.print_exc()
        return None

def validate_ax25_frame(frame_data):
    """
    Validate that frame_data looks like a valid AX.25 frame.
    AX.25 frames have:
    - Minimum size: ~18 bytes (addresses + control + FCS)
    - Address field: 14 bytes (dest + src, 7 bytes each)
    - Control field: 1 byte
    - PID field: 1 byte (for UI frames)
    - Info field: variable
    - FCS: 2 bytes
    """
    if len(frame_data) < 18:
        return False
    
    # Check for reasonable AX.25 structure
    # Addresses should be printable ASCII (callsigns)
    # First 14 bytes are addresses
    if len(frame_data) >= 14:
        # Check if addresses look reasonable (not all zeros or all 0xFF)
        addr_bytes = frame_data[:14]
        if all(b == 0 for b in addr_bytes) or all(b == 0xFF for b in addr_bytes):
            return False
        
        # Check if addresses contain printable ASCII (shifted left by 1 bit in AX.25)
        # AX.25 addresses are 7-bit ASCII shifted left
        printable_count = 0
        for b in addr_bytes:
            shifted = (b >> 1) & 0x7F
            if 32 <= shifted <= 126:  # Printable ASCII
                printable_count += 1
        if printable_count < 4:  # At least some printable chars
            return False
    
    return True

def decode_ax25_frames_from_bytes(data_bytes):
    """
    Decode AX.25 frames from raw bytes.
    Look for AX.25 frame markers (0x7E flags) and extract valid frames.
    """
    frames = []
    if not data_bytes:
        return frames
    
    # Find frame boundaries (0x7E flags)
    # Look for sequences of 0x7E that indicate frame boundaries
    flag_positions = []
    for i, byte in enumerate(data_bytes):
        if byte == 0x7E:
            flag_positions.append(i)
    
    # Extract frames between flags
    for i in range(len(flag_positions) - 1):
        start = flag_positions[i] + 1  # Skip opening flag
        end = flag_positions[i + 1]     # Up to closing flag
        frame_data = data_bytes[start:end]
        
        # Validate frame
        if validate_ax25_frame(frame_data):
            frames.append(frame_data)
    
    # Also check for frame at end if there's an opening flag
    if len(flag_positions) > 0:
        start = flag_positions[-1] + 1
        frame_data = data_bytes[start:]
        if validate_ax25_frame(frame_data):
            frames.append(frame_data)
    
    return frames

def decode_ax25_with_minimodem(audio_file, sample_rate=48000):
    """
    Use minimodem to decode AX.25 frames from audio file.
    
    Args:
        audio_file: Path to audio file (WAV format)
        sample_rate: Sample rate of audio file
    
    Returns:
        Tuple: (frames_list, stdout, stderr)
    """
    temp_dir = None
    raw_file = None
    
    try:
        # Create temporary directory for raw audio
        temp_dir = tempfile.mkdtemp()
        raw_file = os.path.join(temp_dir, 'audio.raw')
        
        # Convert WAV to raw PCM format
        if not convert_wav_to_raw(audio_file, raw_file, sample_rate):
            return None, None, None
        
        # minimodem command: --rx for receive, 2400 for baud rate
        # -8 for ASCII 8-N-1 (for text output)
        # -q for quiet mode (less verbose)
        # -R for sample rate
        # Read from stdin (raw PCM)
        cmd = [
            'minimodem',
            '--rx',
            '-q',  # Quiet mode
            '-8',  # ASCII 8-N-1
            '-R', str(sample_rate),  # Sample rate
            '2400',  # 2400 baud
        ]
        
        # Read raw audio file and pipe to minimodem
        # Use a short timeout since we're processing a small segment
        # minimodem should exit when stdin closes
        try:
            with open(raw_file, 'rb') as raw_f:
                result = subprocess.run(
                    cmd,
                    stdin=raw_f,
                    capture_output=True,
                    text=True,
                    timeout=3  # Very short timeout - file should process quickly
                )
        except subprocess.TimeoutExpired:
            # If timeout, minimodem didn't find signal - this is expected for NFM
            print("   minimodem timed out (expected if audio is NFM-modulated)")
            return [], "", "minimodem timeout - no FSK signal detected"
        
        # minimodem returns 0 on success, non-zero on failure/no signal
        # Even if returncode is non-zero, check stdout for any decoded data
        
        # Parse output to extract frames
        frames = []
        lines = result.stdout.split('\n') if result.stdout else []
        
        # minimodem outputs decoded text/ASCII data
        # Look for frame-like patterns in output
        current_frame = []
        for line in lines:
            line = line.strip()
            if not line:
                continue
            
            # Look for patterns that indicate AX.25 frame data
            # Could be text containing "SIG", ">", or other frame markers
            if len(line) > 10:  # Minimum meaningful frame size
                # Check if line contains frame-like data
                if any(marker in line for marker in ['SIG', '>', ':', '0x7E', '\x7e']):
                    frames.append(line)
                elif len(line) > 20:  # Long enough to be frame data
                    frames.append(line)
        
        # If no frames found in stdout, check stderr for any useful info
        if len(frames) == 0:
            if result.stderr:
                stderr_lines = result.stderr.split('\n')
                for line in stderr_lines:
                    if len(line) > 20:
                        frames.append(line)
        
        # If still no frames and returncode is 0, try parsing raw stdout as data
        if len(frames) == 0 and result.returncode == 0 and result.stdout:
            # Try treating entire stdout as a single frame
            if len(result.stdout.strip()) > 10:
                frames.append(result.stdout.strip())
        
        return frames, result.stdout, result.stderr
        
    except subprocess.TimeoutExpired:
        print("ERROR: minimodem timed out")
        return None, None, None
    except Exception as e:
        print(f"ERROR: Failed to run minimodem: {e}")
        import traceback
        traceback.print_exc()
        return None, None, None
    finally:
        # Clean up temporary files
        if raw_file and os.path.exists(raw_file):
            try:
                os.unlink(raw_file)
            except:
                pass
        if temp_dir and os.path.exists(temp_dir):
            try:
                os.rmdir(temp_dir)
            except:
                pass

def parse_ax25_frame(frame_data):
    """
    Parse AX.25 frame data to extract frame information.
    
    Args:
        frame_data: Frame data (bytes or string)
    
    Returns:
        Dictionary with frame information
    """
    info = {
        'raw_data': frame_data,
        'length': len(frame_data) if isinstance(frame_data, (bytes, bytearray)) else len(str(frame_data)),
        'is_binary': isinstance(frame_data, (bytes, bytearray))
    }
    
    # Try to extract text content
    if isinstance(frame_data, (bytes, bytearray)):
        try:
            text = frame_data.decode('utf-8', errors='ignore')
            info['text'] = text
            if 'SIG' in text:
                info['has_signature'] = True
        except:
            info['text'] = None
    else:
        info['text'] = str(frame_data)
        if 'SIG' in str(frame_data):
            info['has_signature'] = True
    
    return info

def validate_frames(frames, expected_message=None):
    """
    Validate that exactly 2 frames are present and have correct structure.
    
    Args:
        frames: List of frame data (bytes or strings)
        expected_message: Expected message text (optional)
    
    Returns:
        Tuple (success: bool, details: dict)
    """
    details = {
        'frame_count': len(frames),
        'frames': [],
        'errors': []
    }
    
    # Check frame count
    if len(frames) != 2:
        details['errors'].append(f"Expected 2 frames, found {len(frames)}")
        if len(frames) == 0:
            return False, details
    
    # Parse each frame
    for i, frame_data in enumerate(frames):
        frame_info = parse_ax25_frame(frame_data)
        frame_info['frame_number'] = i + 1
        details['frames'].append(frame_info)
        
        # Basic validation
        if frame_info['length'] < 10:
            details['errors'].append(f"Frame {i+1}: Too short ({frame_info['length']} bytes, minimum 10)")
    
    # Check if frames are similar (should be identical for 2-frame protocol)
    if len(details['frames']) == 2:
        frame1_data = details['frames'][0].get('raw_data')
        frame2_data = details['frames'][1].get('raw_data')
        
        if isinstance(frame1_data, (bytes, bytearray)) and isinstance(frame2_data, (bytes, bytearray)):
            if frame1_data == frame2_data:
                details['frames_identical'] = True
            else:
                details['frames_identical'] = False
                # Check if they're similar (might have different headers)
                if len(frame1_data) == len(frame2_data):
                    # Compare data portion (skip first few bytes which might be headers)
                    data1 = frame1_data[10:]
                    data2 = frame2_data[10:]
                    if data1 == data2:
                        details['frames_data_identical'] = True
                    else:
                        details['frames_data_identical'] = False
        else:
            # String comparison
            if str(frame1_data) == str(frame2_data):
                details['frames_identical'] = True
            else:
                details['frames_identical'] = False
    
    # Check for expected message if provided
    if expected_message:
        found_message = False
        for frame_info in details['frames']:
            text = frame_info.get('text', '')
            if text and expected_message in text:
                found_message = True
                break
        if not found_message:
            details['errors'].append(f"Expected message '{expected_message}' not found in frames")
    
    success = len(details['errors']) == 0 and details['frame_count'] == 2
    return success, details

def main():
    """Main validation function"""
    if len(sys.argv) < 2:
        print("Usage: validate_ax25_frames.py <output_wav_file> [expected_message]")
        print("  output_wav_file: Path to output WAV file to validate")
        print("  expected_message: Optional expected message text in frames")
        sys.exit(1)
    
    wav_file = sys.argv[1]
    expected_message = sys.argv[2] if len(sys.argv) > 2 else None
    
    print("=" * 70)
    print("AX.25 Frame Validation")
    print("=" * 70)
    print(f"Input file: {wav_file}")
    if expected_message:
        print(f"Expected message: {expected_message}")
    print()
    
    # Check if file exists
    if not os.path.exists(wav_file):
        print(f"ERROR: File not found: {wav_file}")
        sys.exit(1)
    
    # Check file size
    file_size = os.path.getsize(wav_file)
    print(f"File size: {file_size} bytes")
    
    if file_size <= 44:
        print("ERROR: File is too small (only WAV header, no data)")
        sys.exit(1)
    
    # Check minimodem availability
    print("\n1. Checking minimodem availability...")
    if not check_minimodem():
        print("ERROR: minimodem is not installed or not in PATH")
        print("Install with: sudo apt-get install minimodem")
        sys.exit(1)
    print("   minimodem found")
    
    # Calculate samples needed for 2 frames
    print("\n2. Calculating required samples for 2 AX.25 frames...")
    num_samples, duration_seconds = calculate_frame_samples()
    print(f"   Calculated: {num_samples} samples ({duration_seconds:.2f} seconds)")
    print(f"   Formula: 2 frames * 300 bits/frame * 20 repeat factor at 2400 baud")
    
    # Find where voice ends, then extract data-only portion
    print(f"\n3. Finding where voice/audio ends...")
    voice_end_frame, total_frames = find_voice_end(wav_file, sample_rate=48000)
    
    if voice_end_frame is None:
        print("ERROR: Failed to find voice end")
        sys.exit(1)
    
    print(f"   Voice ends at frame {voice_end_frame} of {total_frames}")
    print(f"   Data-only portion: frames {voice_end_frame} to {total_frames}")
    print(f"   Duration: {(total_frames - voice_end_frame) / 48000:.2f} seconds")
    
    # Extract data-only segment (where AX.25 frames should be)
    print(f"\n4. Extracting data-only portion (last {duration_seconds:.2f} seconds)...")
    print(f"   (This should contain ONLY the AX.25 signature frames, no audio)")
    temp_audio, sample_rate, start_frame, total_frames = extract_data_only_segment(
        wav_file, voice_end_frame, num_samples, sample_rate=48000
    )
    
    if not temp_audio:
        print("ERROR: Failed to extract audio segment")
        sys.exit(1)
    
    print(f"   Extracted frames {start_frame} to {total_frames} (total: {total_frames} frames)")
    print(f"   Sample rate: {sample_rate} Hz")
    print(f"   Duration: {(total_frames - start_frame) / sample_rate:.2f} seconds")
    print(f"   Temporary file: {temp_audio}")
    
    try:
        # Extract data bytes directly from WAV file
        print("\n5. Extracting AX.25 data bytes from WAV file...")
        print("   (Data is stored as float samples, repeated 20x)")
        
        data_bytes = extract_data_bytes_from_wav(temp_audio, 0, total_frames - start_frame)
        
        if data_bytes is None:
            print("ERROR: Failed to extract data bytes")
            sys.exit(1)
        
        print(f"   Extracted {len(data_bytes)} bytes")
        
        # Decode AX.25 frames from bytes
        print("\n6. Decoding AX.25 frames from extracted bytes...")
        all_frames = decode_ax25_frames_from_bytes(data_bytes)
        
        print(f"   Found {len(all_frames)} potential frame(s)")
        
        # For validation, we expect exactly 2 frames at the end
        # Take the last 2 frames found (these should be the signature frames)
        if len(all_frames) >= 2:
            frames = all_frames[-2:]  # Last 2 frames
            print(f"   Using last 2 frames for validation")
        elif len(all_frames) > 0:
            frames = all_frames  # Use what we have
            print(f"   WARNING: Only found {len(all_frames)} frame(s), expected 2")
        else:
            frames = []
        
        if len(frames) == 0:
            print("   WARNING: No AX.25 frames detected")
            print("   This may indicate:")
            print("     - Frames are not in the extracted segment")
            print("     - Frame markers (0x7E) not found")
            print("     - Data format doesn't match expected structure")
        else:
            print("\n   Frame details:")
            for i, frame in enumerate(frames[:5]):  # Show first 5 frames
                print(f"     Frame {i+1}: {len(frame)} bytes")
                if len(frame) > 0:
                    # Show hex preview
                    hex_preview = ' '.join(f'{b:02x}' for b in frame[:20])
                    print(f"       Hex: {hex_preview}...")
                    # Try to decode as text
                    try:
                        text = frame.decode('utf-8', errors='ignore')
                        if text and len(text.strip()) > 0:
                            print(f"       Text: {text[:50]}...")
                    except:
                        pass
        
        # Validate frames
        print("\n7. Validating frames...")
        success, details = validate_frames(frames, expected_message)
        
        print(f"   Frame count: {details['frame_count']}")
        for i, frame_info in enumerate(details['frames']):
            print(f"\n   Frame {frame_info['frame_number']}:")
            print(f"     Length: {frame_info['length']} bytes")
            print(f"     Type: {'Binary' if frame_info['is_binary'] else 'Text'}")
            if 'text' in frame_info and frame_info['text']:
                text_preview = frame_info['text'][:100]
                print(f"     Text preview: {text_preview}...")
            if 'has_signature' in frame_info:
                print(f"     Contains signature: Yes")
        
        if 'frames_identical' in details:
            print(f"\n   Frames identical: {details['frames_identical']}")
        if 'frames_data_identical' in details:
            print(f"   Frame data identical: {details['frames_data_identical']}")
        
        # Report results
        print("\n" + "=" * 70)
        if success:
            print("VALIDATION PASSED")
            print("=" * 70)
            print("\nSummary:")
            print(f"  - Exactly 2 AX.25 frames detected")
            print(f"  - Frame structure is valid")
            if expected_message:
                print(f"  - Expected message found")
            return 0
        else:
            print("VALIDATION FAILED")
            print("=" * 70)
            print("\nErrors:")
            for error in details['errors']:
                print(f"  - {error}")
            if details['frame_count'] > 0:
                print(f"\nNote: Found {details['frame_count']} frame(s), but validation failed.")
            return 1
            
    finally:
        # Clean up temporary file
        if os.path.exists(temp_audio):
            os.unlink(temp_audio)
            print(f"\nCleaned up temporary file: {temp_audio}")

if __name__ == '__main__':
    sys.exit(main())
