import numpy as np
from gnuradio import gr
import os

try:
    from pkcs11 import lib as pkcs11_lib
    from pkcs11 import Token
    from pkcs11.constants import UserType, ObjectClass, KeyType, Mechanism
    PKCS11_AVAILABLE = True
except ImportError:
    PKCS11_AVAILABLE = False

try:
    from gnuradio import nacl
    NACL_AVAILABLE = True
except ImportError:
    NACL_AVAILABLE = False

class blk(gr.sync_block):
    def __init__(self):
        gr.sync_block.__init__(
            self,
            name='Burst Trigger Audio + Signed Frames',
            in_sig=[np.float32, np.uint8],
            out_sig=[np.float32, np.uint8]
        )
        self._key_buffer = bytearray()
        self._pkcs11_lib = None
        self._use_pkcs11 = True
        self._audio_eof_detected = False
        self._data_frames_bytes = None
        self._data_output_idx = 0
        self._silence_count = 0
        self._silence_threshold = 48000  # 1 second of silence
        self._total_audio_samples = 0
        self._min_audio_samples = 96000  # Require at least 2 seconds of audio
        
        # Track padding samples needed after EOF to ensure frames are fully transmitted
        self._samples_output_after_eof = 0
        self._required_padding_samples = 0
        self._frames_complete = False
        
        # Flush period: wait for audio to be completely flushed from pipeline before outputting data
        self._audio_flush_samples = 0
        self._audio_flush_needed = 5000  # Flush ~0.1 seconds to ensure all audio is out
        
        # Configuration: 2 AX.25 frames at 2400 baud with 20x repeat factor at 48kHz sample rate
        # Each byte becomes 20 samples after repeat block
        # Add buffer for pipeline delay (AX.25 encoder overhead, etc.)
        self._repeat_factor = 20
        self._sample_rate = 48000
        self._pipeline_buffer_samples = 10000  # Extra samples for pipeline delay

        if PKCS11_AVAILABLE:
            self._init_pkcs11()

    def _init_pkcs11(self):
        lib_paths = [
            '/usr/lib/x86_64-linux-gnu/opensc-pkcs11.so',
            '/usr/lib/opensc-pkcs11.so',
            '/usr/lib/x86_64-linux-gnu/p11-kit-proxy.so',
            '/usr/lib/p11-kit-proxy.so',
        ]
        for path in lib_paths:
            if os.path.exists(path):
                try:
                    self._pkcs11_lib = pkcs11_lib(path)
                    print(f"PKCS#11: Loaded {path}")
                    return True
                except Exception as e:
                    continue
        return False

    def _get_token(self, pin=None):
        if not self._pkcs11_lib:
            return None
        try:
            tokens = self._pkcs11_lib.get_tokens()
            if not tokens:
                return None
            token = tokens[0]
            if pin:
                token.open(UserType.USER, pin.encode())
            else:
                try:
                    token.open(UserType.USER)
                except:
                    return None
            return token
        except:
            return None

    def _sign_with_pkcs11(self, data, pin=None):
        if not PKCS11_AVAILABLE or not self._pkcs11_lib:
            return None
        try:
            token = self._get_token(pin)
            if not token:
                return None
            session = token.open(UserType.USER, pin.encode() if pin else None)
            private_keys = session.get_objects({ObjectClass.PRIVATE_KEY, KeyType.EC})
            if not private_keys:
                return None
            signature = private_keys[0].sign(data, mechanism=Mechanism.ECDSA)
            return signature
        except Exception as e:
            print(f"PKCS#11 signing error: {e}")
            return None

    def _generate_frames_bytes(self):
        try:
            import __main__
            if hasattr(__main__, 'message_text'):
                msg = getattr(__main__, 'message_text').value()
                if msg:
                    msg_bytes = msg.encode('utf-8')
                    signature = None

                    if self._use_pkcs11:
                        signature = self._sign_with_pkcs11(msg_bytes)

                    if not signature and len(self._key_buffer) >= 32 and NACL_AVAILABLE:
                        key = bytes(self._key_buffer[:32])
                        signature = nacl.sign_ed25519(msg_bytes, key)

                    frames = bytearray()

                    if signature:
                        frames.extend(b'SIG')
                        frames.extend(signature)

                    frames.extend(msg_bytes)

                    # Generate TWO frames by duplicating the data
                    # The AX.25 encoder will create a separate frame for each transmission
                    frames_duplicated = bytearray(frames)
                    frames_duplicated.extend(frames)  # Append second frame

                    return bytes(frames_duplicated)
        except Exception as e:
            print(f"Frame generation error: {e}")
            pass
        return None

    def work(self, input_items, output_items):
        try:
            import __main__
            if hasattr(__main__, 'use_pkcs11'):
                self._use_pkcs11 = getattr(__main__, 'use_pkcs11').value()
        except:
            pass

        audio_in = input_items[0]
        key_in = input_items[1]
        audio_out = output_items[0]
        data_out = output_items[1]
        n = len(output_items[0])  # Use output buffer size, not input size

        if len(key_in) > 0 and not self._use_pkcs11:
            self._key_buffer.extend(key_in.tolist())
            if len(self._key_buffer) > 32:
                self._key_buffer = self._key_buffer[-32:]

        # Track audio samples processed
        if len(audio_in) > 0 and not self._audio_eof_detected:
            self._total_audio_samples += len(audio_in)

        # Detect EOF when source stops (len(audio_in) == 0) AND we've processed audio
        if len(audio_in) == 0 and not self._audio_eof_detected and self._total_audio_samples > 0:
            self._audio_eof_detected = True
            self._audio_flush_samples = 0  # Start flush period
            frames_bytes = self._generate_frames_bytes()
            if frames_bytes:
                self._data_frames_bytes = bytearray(frames_bytes)
                self._data_output_idx = 0
                self._frames_complete = False
                self._samples_output_after_eof = 0
                
                # Calculate required padding duration:
                # 2 AX.25 frames at 2400 baud with 20x repeat factor at 48kHz sample rate
                # Each byte becomes 20 samples after repeat block
                # Add buffer for pipeline delay (AX.25 encoder adds overhead, etc.)
                total_bytes = len(frames_bytes)
                # Estimate AX.25 encoder overhead: ~18 bytes per frame (flags, address, control, FCS)
                # For 2 frames, add ~36 bytes overhead
                estimated_ax25_bytes = total_bytes + 36
                # Calculate samples needed: bytes * repeat_factor + pipeline buffer
                self._required_padding_samples = (estimated_ax25_bytes * self._repeat_factor) + self._pipeline_buffer_samples
                
                print(f"File source ended after {self._total_audio_samples} samples")
                print(f"Generated {len(frames_bytes)} bytes for 2 AX.25 frames")
                print(f"Estimated {estimated_ax25_bytes} bytes after AX.25 encoding")
                print(f"Required padding: {self._required_padding_samples} samples ({self._required_padding_samples/self._sample_rate:.2f} seconds)")
            else:
                print(f"File source ended after {self._total_audio_samples} samples, but no frames generated")
                self._frames_complete = True
                self._required_padding_samples = 0

        # Process audio or data frames
        if not self._audio_eof_detected:
            # Pass through audio while receiving it
            # Handle case where audio_in might be shorter than n
            audio_len = min(n, len(audio_in)) if len(audio_in) > 0 else 0
            if audio_len > 0:
                audio_out[:audio_len] = audio_in[:audio_len]
            # Zero-pad remaining audio output
            audio_out[audio_len:n] = 0.0
            data_out[:n] = 0
            # Always return n to keep flowgraph running
            return n
        else:
            # After EOF detected: flush audio first, then output ONLY data frames (no audio)
            # CRITICAL: audio_out must be zero when outputting data
            
            # Flush period: wait for all audio to be flushed from pipeline
            if self._audio_flush_samples < self._audio_flush_needed:
                audio_out[:n] = 0.0
                data_out[:n] = 0  # CRITICAL: No data output during flush
                self._audio_flush_samples += n
                return n
            
            # After flush period: output ONLY data frames (no audio)
            audio_out[:n] = 0.0
            
            # First, output data frames if available
            if not self._frames_complete and self._data_frames_bytes is not None:
                if self._data_output_idx < len(self._data_frames_bytes):
                    remaining = len(self._data_frames_bytes) - self._data_output_idx
                    n_output = min(n, remaining) if n > 0 else 0
                    
                    if n_output > 0:
                        data_out[:n_output] = np.frombuffer(
                            self._data_frames_bytes[self._data_output_idx:self._data_output_idx+n_output],
                            dtype=np.uint8
                        )
                        self._data_output_idx += n_output
                        self._samples_output_after_eof += n
                        
                        # Check if all frames are output
                        if self._data_output_idx >= len(self._data_frames_bytes):
                            print(f"All AX.25 frame data output ({self._data_output_idx} bytes), continuing with padding")
                        
                        # Zero-pad remaining output
                        if n_output < n:
                            data_out[n_output:n] = 0
                        
                        # Continue outputting until padding is complete
                        return n
                    else:
                        # n == 0 but we still have frames to output
                        data_out[:n] = 0
                        self._samples_output_after_eof += n
                        return n if n > 0 else 0
                else:
                    # All data frames output, now output silence padding
                    self._samples_output_after_eof += n
                    data_out[:n] = 0
                    
                    # Check if we've output enough padding samples
                    if self._samples_output_after_eof >= self._required_padding_samples:
                        if not self._frames_complete:
                            self._frames_complete = True
                            print(f"Padding complete: {self._samples_output_after_eof} samples output after EOF")
                        # Return 0 to signal WORK_DONE only after all padding is complete
                        return 0
                    else:
                        # Continue outputting silence padding
                        return n
            else:
                # No frames or frames already complete, but check if padding is needed
                if self._samples_output_after_eof < self._required_padding_samples:
                    self._samples_output_after_eof += n
                    data_out[:n] = 0
                    
                    if self._samples_output_after_eof >= self._required_padding_samples:
                        self._frames_complete = True
                        print(f"Padding complete: {self._samples_output_after_eof} samples output after EOF")
                        return 0
                    else:
                        return n
                else:
                    # All padding complete, signal WORK_DONE
                    data_out[:n] = 0
                    return 0
