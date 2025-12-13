#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
KISS TNC with PTT Control Example

This example demonstrates how to use the KISS TNC block with built-in PTT control
for packet radio transmission. The PTT control automatically keys the transmitter
before sending data and unkeys after transmission.

Features demonstrated:
- KISS TNC block creation
- PTT control enable/disable
- Manual PTT control
- Serial port configuration
"""

import sys
import time
from gnuradio import gr, blocks
from gnuradio import packet_protocols


class kiss_tnc_ptt_example(gr.top_block):
    """Example flowgraph demonstrating KISS TNC with PTT control"""

    def __init__(self, device="/dev/ttyUSB0", baud_rate=9600, enable_ptt=True):
        gr.top_block.__init__(self, "KISS TNC PTT Example", catch_exceptions=True)

        # Parameters
        self.device = device
        self.baud_rate = baud_rate
        self.enable_ptt = enable_ptt

        # Create KISS TNC block
        # Note: In a real flowgraph, you would connect this to your data source
        # and RF sink. This example shows the PTT control setup.
        self.kiss_tnc = packet_protocols.kiss_tnc(
            device=device,
            baud_rate=baud_rate,
            tx_delay=10,  # TX delay in 10ms units (100ms)
            tx_tail=10    # TX tail in 10ms units (100ms)
        )

        # Enable PTT control
        if enable_ptt:
            self.kiss_tnc.set_ptt_enabled(True)
            # Use RTS line for PTT (default)
            self.kiss_tnc.set_ptt_use_dtr(False)
            print(f"PTT control enabled on {device}")
            print("  - Using RTS line for PTT control")
            print("  - TX delay: 100ms")
            print("  - TX tail: 100ms")
        else:
            self.kiss_tnc.set_ptt_enabled(False)
            print(f"PTT control disabled on {device}")

        # Create a simple test data source (in real usage, connect your actual data source)
        # This is just for demonstration - in practice you'd connect to your encoder
        self.null_source = blocks.null_source(gr.sizeof_char)
        self.null_sink = blocks.null_sink(gr.sizeof_char)

        # Connect blocks (example connection)
        # In a real flowgraph, you would connect:
        # [Data Source] -> [AX.25 Encoder] -> [KISS TNC] -> [Serial Port]
        # For this example, we just show the setup
        # self.connect((self.null_source, 0), (self.kiss_tnc, 0))
        # self.connect((self.kiss_tnc, 0), (self.null_sink, 0))

    def manual_ptt_demo(self):
        """Demonstrate manual PTT control"""
        print("\n" + "=" * 70)
        print("Manual PTT Control Demo")
        print("=" * 70)

        if not self.enable_ptt:
            print("PTT control is disabled. Enable it first.")
            return

        print("\n1. Keying transmitter manually...")
        self.kiss_tnc.set_ptt(True)
        ptt_state = self.kiss_tnc.get_ptt()
        print(f"   PTT state: {ptt_state} (should be True)")

        print("\n2. Waiting 1 second...")
        time.sleep(1)

        print("\n3. Unkeying transmitter...")
        self.kiss_tnc.set_ptt(False)
        ptt_state = self.kiss_tnc.get_ptt()
        print(f"   PTT state: {ptt_state} (should be False)")

        print("\nManual PTT control demo complete.")

    def get_ptt_info(self):
        """Display PTT configuration information"""
        print("\n" + "=" * 70)
        print("PTT Configuration Information")
        print("=" * 70)
        print(f"Device: {self.device}")
        print(f"Baud Rate: {self.baud_rate}")
        print(f"PTT Enabled: {self.kiss_tnc.get_ptt() if hasattr(self.kiss_tnc, 'get_ptt') else 'N/A'}")
        print(f"PTT State: {self.kiss_tnc.get_ptt() if hasattr(self.kiss_tnc, 'get_ptt') else 'N/A'}")


def main():
    """Main function"""
    import argparse

    parser = argparse.ArgumentParser(
        description='KISS TNC with PTT Control Example',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic usage with PTT enabled
  python3 kiss_tnc_ptt_example.py --device /dev/ttyUSB0

  # Disable PTT control
  python3 kiss_tnc_ptt_example.py --device /dev/ttyUSB0 --no-ptt

  # Use DTR line instead of RTS
  python3 kiss_tnc_ptt_example.py --device /dev/ttyUSB0 --use-dtr

  # Custom baud rate
  python3 kiss_tnc_ptt_example.py --device /dev/ttyUSB0 --baud 19200
        """
    )
    parser.add_argument(
        '--device', '-d',
        default='/dev/ttyUSB0',
        help='Serial port device (default: /dev/ttyUSB0)'
    )
    parser.add_argument(
        '--baud', '-b',
        type=int,
        default=9600,
        help='Serial port baud rate (default: 9600)'
    )
    parser.add_argument(
        '--no-ptt',
        action='store_true',
        help='Disable PTT control'
    )
    parser.add_argument(
        '--use-dtr',
        action='store_true',
        help='Use DTR line for PTT instead of RTS'
    )
    parser.add_argument(
        '--demo',
        action='store_true',
        help='Run manual PTT control demo'
    )

    args = parser.parse_args()

    print("=" * 70)
    print("KISS TNC with PTT Control Example")
    print("=" * 70)
    print(f"Device: {args.device}")
    print(f"Baud Rate: {args.baud}")
    print(f"PTT Enabled: {not args.no_ptt}")
    print()

    # Check if device exists (optional check)
    import os
    if not os.path.exists(args.device):
        print(f"WARNING: Device {args.device} does not exist.")
        print("This example will still run, but the KISS TNC may fail to initialize.")
        print()

    # Create flowgraph
    try:
        tb = kiss_tnc_ptt_example(
            device=args.device,
            baud_rate=args.baud,
            enable_ptt=not args.no_ptt
        )

        # Configure DTR if requested
        if args.use_dtr and not args.no_ptt:
            tb.kiss_tnc.set_ptt_use_dtr(True)
            print("Using DTR line for PTT control")

        # Display configuration
        tb.get_ptt_info()

        # Run manual PTT demo if requested
        if args.demo and not args.no_ptt:
            tb.manual_ptt_demo()

        print("\n" + "=" * 70)
        print("Example setup complete.")
        print("=" * 70)
        print("\nIn a real flowgraph, you would:")
        print("  1. Connect your data source to the KISS TNC input")
        print("  2. Connect the KISS TNC output to your RF sink")
        print("  3. The PTT will automatically key/unkey during transmission")
        print("\nFor manual PTT control, use:")
        print("  tb.kiss_tnc.set_ptt(True)   # Key transmitter")
        print("  tb.kiss_tnc.set_ptt(False)  # Unkey transmitter")
        print("  state = tb.kiss_tnc.get_ptt()  # Get current state")

        # Note: We don't start the flowgraph in this example since we don't
        # have actual data sources/sinks connected. In a real application,
        # you would call tb.start() and tb.wait()

        return 0

    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())

