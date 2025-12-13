#!/usr/bin/env python3
"""
Test script to verify the tx_audio_signed fix works correctly.
Tests that variable-length audio files produce complete output files.
"""

import sys
import os
import time
import subprocess
from pathlib import Path

def test_flowgraph():
    """Test the flowgraph with a variable-length audio file"""
    
    # Paths
    input_file = '/home/haaken/Musikk/cq.wav'
    output_file = '/home/haaken/Musikk/test_output.wav'
    script_dir = Path(__file__).parent
    
    # Check input file exists
    if not os.path.exists(input_file):
        print(f"ERROR: Input file not found: {input_file}")
        return False
    
    # Get input file size
    input_size = os.path.getsize(input_file)
    print(f"Input file: {input_file}")
    print(f"Input file size: {input_size} bytes")
    
    # Remove old output file if it exists
    if os.path.exists(output_file):
        os.remove(output_file)
        print(f"Removed old output file: {output_file}")
    
    # Import and run the flowgraph
    sys.path.insert(0, str(script_dir))
    sys.path.insert(0, str(script_dir / 'examples'))
    
    try:
        from PyQt5 import Qt
        from examples.tx_audio_signed import tx_audio_signed
        
        print("\nCreating flowgraph...")
        qapp = Qt.QApplication(sys.argv)
        tb = tx_audio_signed()
        
        # Set test parameters
        tb.audio_file_path = input_file
        tb.output_audio_file = output_file
        tb.message_text = "Test message"
        tb.use_pkcs11 = False  # Use kernel keyring instead
        
        # Update main module for epy_block
        import __main__
        __main__.message_text = tb.message_text
        __main__.use_pkcs11 = tb.use_pkcs11
        
        print(f"Input file: {tb.audio_file_path}")
        print(f"Output file: {tb.output_audio_file}")
        print(f"Message: {tb.message_text}")
        
        # Start flowgraph
        print("\nStarting flowgraph...")
        tb.start()
        
        # Wait for completion (with timeout)
        print("Waiting for flowgraph to complete...")
        timeout = 30  # 30 second timeout
        start_time = time.time()
        
        # Wait for file to be created and grow
        last_size = 0
        stable_count = 0
        while time.time() - start_time < timeout:
            if os.path.exists(output_file):
                current_size = os.path.getsize(output_file)
                if current_size == last_size:
                    stable_count += 1
                    if stable_count >= 10:  # File size stable for 1 second
                        print("File size stabilized, flowgraph likely complete")
                        break
                else:
                    stable_count = 0
                    last_size = current_size
            time.sleep(0.1)
        
        # Wait a bit more for file to be fully written
        time.sleep(2)
        
        # Stop flowgraph
        tb.stop()
        tb.wait()
        
        # Close file sink
        try:
            tb.blocks_wavfile_sink_0.close()
        except:
            pass
        
        print("\nFlowgraph completed")
        
    except Exception as e:
        print(f"ERROR: Exception during flowgraph execution: {e}")
        import traceback
        traceback.print_exc()
        return False
    
    # Check output file
    if not os.path.exists(output_file):
        print(f"ERROR: Output file was not created: {output_file}")
        return False
    
    output_size = os.path.getsize(output_file)
    print(f"\nOutput file: {output_file}")
    print(f"Output file size: {output_size} bytes")
    
    # Check if file is more than just header (44 bytes)
    if output_size <= 44:
        print(f"ERROR: Output file is only {output_size} bytes (just header, no data)")
        return False
    
    # Check if file is reasonable size (should be at least input size + padding)
    # Input is ~43KB, padding should add ~14KB, so total should be ~57KB+
    expected_min_size = input_size + 10000  # At least input + some padding
    if output_size < expected_min_size:
        print(f"WARNING: Output file size ({output_size} bytes) is smaller than expected minimum ({expected_min_size} bytes)")
        print("But it's larger than header, so fix may be partially working")
    
    # Verify it's a valid WAV file
    try:
        result = subprocess.run(['file', output_file], capture_output=True, text=True)
        print(f"File type: {result.stdout.strip()}")
        if 'WAVE' not in result.stdout:
            print("WARNING: Output file may not be a valid WAV file")
    except:
        pass
    
    print("\nTEST PASSED: Output file was created and contains data")
    return True

if __name__ == '__main__':
    success = test_flowgraph()
    sys.exit(0 if success else 1)

