#!/usr/bin/env python3
"""
Run the GRC flowgraph and validate the output WAV file.
"""

import sys
import os
import subprocess
import time
import signal
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.absolute()
EXAMPLES_DIR = SCRIPT_DIR / "examples"
INPUT_AUDIO = "/home/haaken/Musikk/cq.wav"
OUTPUT_AUDIO = "/tmp/test_output_ax25.wav"

def main():
    print("=" * 70)
    print("Running GRC Flowgraph Test")
    print("=" * 70)
    print(f"Input audio: {INPUT_AUDIO}")
    print(f"Output audio: {OUTPUT_AUDIO}")
    print()
    
    # Check if input file exists
    if not os.path.exists(INPUT_AUDIO):
        print(f"ERROR: Input audio file not found: {INPUT_AUDIO}")
        return 1
    
    # Remove old output file if it exists
    if os.path.exists(OUTPUT_AUDIO):
        os.remove(OUTPUT_AUDIO)
        print(f"Removed old output file: {OUTPUT_AUDIO}")
    
    # Change to examples directory
    os.chdir(EXAMPLES_DIR)
    
    print("Starting flowgraph...")
    print("Note: A GUI window will open. Please:")
    print("  1. Set 'Audio File Path' to:", INPUT_AUDIO)
    print("  2. Set 'Output Audio File Path' to:", OUTPUT_AUDIO)
    print("  3. Click 'Start' button")
    print("  4. Wait for completion (flowgraph will stop automatically)")
    print("  5. Close the window when done")
    print()
    
    # Run the flowgraph
    try:
        process = subprocess.Popen(
            [sys.executable, "tx_audio_signed.py", "--input", INPUT_AUDIO, "--output", OUTPUT_AUDIO],
            cwd=EXAMPLES_DIR
        )
        
        print(f"Flowgraph process started (PID: {process.pid})")
        print("Waiting for flowgraph to complete...")
        print("(Press Ctrl+C to cancel)")
        print()
        
        # Wait for process to complete
        process.wait()
        
        print()
        print("Flowgraph completed.")
        
    except KeyboardInterrupt:
        print("\nInterrupted by user. Stopping flowgraph...")
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
        return 1
    
    # Check if output file was created
    print()
    print("Checking output file...")
    if not os.path.exists(OUTPUT_AUDIO):
        print(f"ERROR: Output file was not created: {OUTPUT_AUDIO}")
        return 1
    
    file_size = os.path.getsize(OUTPUT_AUDIO)
    print(f"Output file created: {OUTPUT_AUDIO}")
    print(f"File size: {file_size} bytes")
    
    if file_size <= 44:
        print("ERROR: Output file is too small (only WAV header, no data)")
        return 1
    
    # Now validate the output
    print()
    print("=" * 70)
    print("Validating Output WAV File")
    print("=" * 70)
    
    os.chdir(SCRIPT_DIR)
    result = subprocess.run(
        [sys.executable, "validate_ax25_flowgraph.py", OUTPUT_AUDIO],
        capture_output=False
    )
    
    if result.returncode == 0:
        print()
        print("=" * 70)
        print("TEST PASSED")
        print("=" * 70)
        return 0
    else:
        print()
        print("=" * 70)
        print("TEST FAILED")
        print("=" * 70)
        return 1

if __name__ == '__main__':
    sys.exit(main())

