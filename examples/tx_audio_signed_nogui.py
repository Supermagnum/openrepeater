#!/usr/bin/env python3
"""
Non-GUI version of tx_audio_signed flowgraph that runs directly without GUI blocking.
"""

import sys
import os

# Add current directory to path for epy_block imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from gnuradio import gr
from gnuradio import blocks
from gnuradio import packet_protocols
from gnuradio import linux_crypto

# Import epy_block_2 - it should be in the same directory
import tx_audio_signed_epy_block_2 as epy_block_2

class tx_audio_signed_nogui(gr.top_block):
    def __init__(self, input_file, output_file, message_text="Test message"):
        gr.top_block.__init__(self, "Audio File with Signed Frames")
        
        # Parameters
        self.samp_rate = 48000
        self.src_callsign = 'N0CALL'
        self.dest_callsign = 'N0CALL'
        
        # Set message text for epy_block_2
        import __main__
        class MessageText:
            def __init__(self, text):
                self._text = text
            def value(self):
                return self._text
        __main__.message_text = MessageText(message_text)
        __main__.use_pkcs11 = False
        
        # Blocks
        self.linux_crypto_kernel_keyring_source_0 = linux_crypto.kernel_keyring_source(0, False)
        self.epy_block_2 = epy_block_2.blk()
        self.blocks_wavfile_source_0 = blocks.wavfile_source(input_file, False)
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink(
            output_file,
            1,
            self.samp_rate,
            blocks.FORMAT_WAV,
            blocks.FORMAT_PCM_16,
            False
        )
        self.blocks_throttle2_0 = blocks.throttle(gr.sizeof_char*1, 2400, True, 0)
        self.blocks_repeat_0 = blocks.repeat(gr.sizeof_float*1, 20)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, (1.0/127.0))
        self.blocks_add_xx_0 = blocks.add_vff(1)
        self.packet_protocols_ax25_encoder_0 = packet_protocols.ax25_encoder(
            dest_callsign=self.dest_callsign,
            dest_ssid='0',
            src_callsign=self.src_callsign,
            src_ssid='0'
        )
        
        # Connections
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_wavfile_sink_0, 0))
        self.connect((self.blocks_char_to_float_0, 0), (self.blocks_repeat_0, 0))
        self.connect((self.blocks_repeat_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_throttle2_0, 0), (self.packet_protocols_ax25_encoder_0, 0))
        self.connect((self.blocks_wavfile_source_0, 0), (self.epy_block_2, 0))
        self.connect((self.epy_block_2, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.epy_block_2, 1), (self.blocks_throttle2_0, 0))
        self.connect((self.linux_crypto_kernel_keyring_source_0, 0), (self.epy_block_2, 1))
        self.connect((self.packet_protocols_ax25_encoder_0, 0), (self.blocks_char_to_float_0, 0))

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Transmit audio file with signed AX.25 frames (non-GUI)')
    parser.add_argument('--input', '-i', required=True, help='Input audio file path')
    parser.add_argument('--output', '-o', required=True, help='Output WAV file path')
    parser.add_argument('--message', '-m', default='Test message', help='Message text to sign')
    args = parser.parse_args()
    
    print("=" * 70)
    print("Running tx_audio_signed (non-GUI)")
    print("=" * 70)
    print(f"Input file: {args.input}")
    print(f"Output file: {args.output}")
    print(f"Message: {args.message}")
    print()
    
    # Check input file exists
    if not os.path.exists(args.input):
        print(f"ERROR: Input file not found: {args.input}")
        return 1
    
    # Remove old output file if exists
    if os.path.exists(args.output):
        os.remove(args.output)
        print(f"Removed old output file: {args.output}")
    
    print("Starting flowgraph...")
    tb = tx_audio_signed_nogui(args.input, args.output, args.message)
    
    try:
        tb.start()
        print("Flowgraph started. Processing...")
        tb.wait()
        print("Flowgraph completed.")
        
        # Check output file
        if os.path.exists(args.output):
            file_size = os.path.getsize(args.output)
            print(f"Output file created: {args.output} ({file_size} bytes)")
            if file_size > 44:
                print("SUCCESS: Output file contains data")
                return 0
            else:
                print("ERROR: Output file is too small (only header)")
                return 1
        else:
            print("ERROR: Output file was not created")
            return 1
            
    except KeyboardInterrupt:
        print("\nInterrupted by user")
        tb.stop()
        tb.wait()
        return 1
    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()
        tb.stop()
        tb.wait()
        return 1

if __name__ == '__main__':
    sys.exit(main())

