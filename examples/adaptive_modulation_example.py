#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Adaptive Modulation Example

This example demonstrates the adaptive modulation features from gr-packet-protocols,
including link quality monitoring and automatic rate adaptation.

Features demonstrated:
- Link quality monitoring (SNR, BER, frame error rate)
- Adaptive rate control
- Modulation switching based on link quality
"""

import sys
import os

# Add module path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'modules', 'gr-packet-protocols', 'python'))

try:
    from gnuradio import gr, blocks
    from gnuradio import packet_protocols
    from gnuradio.packet_protocols import adaptive_modulator, modulation_switch
    from gnuradio.packet_protocols import link_quality_monitor, adaptive_rate_control
    from gnuradio.packet_protocols import modulation_mode_t
except ImportError as e:
    print(f"ERROR: Failed to import required modules: {e}")
    print("\nMake sure gr-packet-protocols is built and installed.")
    print("Run: cd modules/gr-packet-protocols && mkdir -p build && cd build && cmake .. && make")
    sys.exit(1)


class adaptive_modulation_demo(gr.top_block):
    """Example demonstrating adaptive modulation features"""

    def __init__(self, samp_rate=48000):
        gr.top_block.__init__(self, "Adaptive Modulation Demo", catch_exceptions=True)

        self.samp_rate = samp_rate

        print("=" * 70)
        print("Adaptive Modulation Features Demo")
        print("=" * 70)

        # Create link quality monitor
        # This monitors SNR, BER, and frame error rate in real-time
        try:
            self.link_quality_monitor = link_quality_monitor(
                window_size=1000,  # Samples per window
                snr_threshold_db=10.0,  # SNR threshold in dB
                ber_threshold=0.01  # BER threshold (1%)
            )
            print("✓ Link quality monitor created")
            print("  - Window size: 1000 samples")
            print("  - SNR threshold: 10.0 dB")
            print("  - BER threshold: 0.01 (1%)")
        except Exception as e:
            print(f"✗ Failed to create link quality monitor: {e}")
            self.link_quality_monitor = None

        # Create adaptive rate control
        # This automatically adjusts modulation mode based on link quality
        try:
            self.adaptive_rate_control = adaptive_rate_control(
                initial_mode=modulation_mode_t.MODE_2FSK,  # Start with 2FSK
                min_snr_db=5.0,  # Minimum SNR for any mode
                snr_margin_db=3.0  # SNR margin for mode switching
            )
            print("✓ Adaptive rate control created")
            print("  - Initial mode: 2FSK")
            print("  - Minimum SNR: 5.0 dB")
            print("  - SNR margin: 3.0 dB")
        except Exception as e:
            print(f"✗ Failed to create adaptive rate control: {e}")
            self.adaptive_rate_control = None

        # Create modulation switch
        # This switches between multiple modulation inputs
        try:
            self.modulation_switch = modulation_switch(
                num_inputs=3,  # Support 3 different modulations
                default_input=0  # Default to first input
            )
            print("✓ Modulation switch created")
            print("  - Number of inputs: 3")
            print("  - Default input: 0")
        except Exception as e:
            print(f"✗ Failed to create modulation switch: {e}")
            self.modulation_switch = None

        # Create adaptive modulator (Python hierarchical block)
        # This is a high-level block that handles all adaptive logic
        try:
            self.adaptive_modulator = adaptive_modulator(
                samp_rate=samp_rate,
                initial_mode=modulation_mode_t.MODE_2FSK
            )
            print("✓ Adaptive modulator created")
            print("  - Sample rate: {} Hz".format(samp_rate))
            print("  - Initial mode: 2FSK")
        except Exception as e:
            print(f"✗ Failed to create adaptive modulator: {e}")
            self.adaptive_modulator = None

        # Create test sources and sinks (for demonstration)
        # In a real flowgraph, these would be your actual data sources/sinks
        self.null_source = blocks.null_source(gr.sizeof_char)
        self.null_sink = blocks.null_sink(gr.sizeof_char)

        print("\n" + "=" * 70)
        print("Setup Complete")
        print("=" * 70)
        print("\nAvailable features:")
        print("  1. Link Quality Monitoring")
        print("     - Real-time SNR, BER, and frame error rate")
        print("     - Configurable thresholds")
        print("\n  2. Adaptive Rate Control")
        print("     - Automatic modulation mode adjustment")
        print("     - Based on link quality metrics")
        print("\n  3. Modulation Switching")
        print("     - Switch between multiple modulation inputs")
        print("     - Based on control signal or quality metrics")
        print("\n  4. Adaptive Modulator")
        print("     - High-level hierarchical block")
        print("     - Handles all adaptive logic automatically")
        print("\nFor complete examples, see:")
        print("  modules/gr-packet-protocols/examples/adaptive_*.py")
        print("  modules/gr-packet-protocols/examples/adaptive_*.grc")

    def get_link_quality_info(self):
        """Get current link quality metrics"""
        if self.link_quality_monitor is None:
            return None

        try:
            # Note: Actual implementation would query the monitor for metrics
            # This is a demonstration of the concept
            return {
                'snr_db': 15.5,  # Example values
                'ber': 0.001,
                'frame_error_rate': 0.002
            }
        except Exception as e:
            print(f"Error getting link quality: {e}")
            return None


def main():
    """Main function"""
    import argparse

    parser = argparse.ArgumentParser(
        description='Adaptive Modulation Features Demo',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
This example demonstrates the adaptive modulation features available in
gr-packet-protocols. It creates the necessary blocks and shows their
configuration.

For complete working examples, see:
  modules/gr-packet-protocols/examples/adaptive_full_duplex_example.py
  modules/gr-packet-protocols/examples/adaptive_half_duplex_example.py
  modules/gr-packet-protocols/examples/adaptive_modulation_example.py
        """
    )
    parser.add_argument(
        '--samp-rate', '-s',
        type=int,
        default=48000,
        help='Sample rate in Hz (default: 48000)'
    )

    args = parser.parse_args()

    try:
        tb = adaptive_modulation_demo(samp_rate=args.samp_rate)

        # Display link quality info if available
        quality_info = tb.get_link_quality_info()
        if quality_info:
            print("\n" + "=" * 70)
            print("Link Quality Metrics (Example)")
            print("=" * 70)
            print(f"SNR: {quality_info['snr_db']:.2f} dB")
            print(f"BER: {quality_info['ber']:.4f} ({quality_info['ber']*100:.2f}%)")
            print(f"Frame Error Rate: {quality_info['frame_error_rate']:.4f}")

        print("\n" + "=" * 70)
        print("Demo Complete")
        print("=" * 70)
        print("\nNote: This is a setup demonstration.")
        print("In a real flowgraph, you would:")
        print("  1. Connect data sources to modulation inputs")
        print("  2. Connect link quality monitor to receive path")
        print("  3. Connect adaptive rate control to link quality monitor")
        print("  4. Connect modulation switch control to adaptive rate control")
        print("  5. Connect RF sink to modulation switch output")

        return 0

    except Exception as e:
        print(f"\nERROR: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())

