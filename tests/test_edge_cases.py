#!/usr/bin/env python3
"""
Edge Case Tests for GNU Radio Blocks
Tests specific conditions like zero amplitude, NaN/infinity, extreme frequency offsets, phase discontinuities
"""

import sys
import numpy as np
import math

try:
    from gnuradio import gr
    from gnuradio import blocks
    from gnuradio import qradiolink
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    sys.exit(1)


def test_edge_case(block_maker, block_name, test_name, test_vector):
    """Test a block with an edge case vector"""
    print(f"Testing {block_name} - {test_name}...", end=" ")

    try:
        tb = gr.top_block()
        block = block_maker()

        # Create source and sinks
        source = blocks.vector_source_c(test_vector.tolist(), False)

        # Connect all possible outputs
        sinks = []
        for i in range(4):  # Most blocks have up to 4 outputs
            try:
                if i == 0:
                    sink = blocks.null_sink(gr.sizeof_gr_complex)
                elif i == 1:
                    sink = blocks.null_sink(gr.sizeof_gr_complex)
                else:
                    sink = blocks.null_sink(gr.sizeof_char)
                sinks.append(sink)
                tb.connect((block, i), sink)
            except:
                break

        tb.connect(source, block)

        # Run with timeout
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()

        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        return False


def main():
    """Run edge case tests"""
    print("=" * 70)
    print("Edge Case Tests for gr-qradiolink")
    print("=" * 70)
    print()

    results = {'passed': 0, 'failed': 0}

    # Test vectors
    test_cases = [
        ("Zero amplitude", np.zeros(1000, dtype=np.complex64)),
        ("NaN values", np.full(1000, np.nan + 1j * np.nan, dtype=np.complex64)),
        ("Infinity values", np.full(1000, np.inf + 1j * np.inf, dtype=np.complex64)),
        ("Negative infinity", np.full(1000, -np.inf + 1j * (-np.inf), dtype=np.complex64)),
        ("Extreme positive", np.full(1000, 1e10 + 1j * 1e10, dtype=np.complex64)),
        ("Extreme negative", np.full(1000, -1e10 + 1j * (-1e10), dtype=np.complex64)),
        ("Very small values", np.full(1000, 1e-10 + 1j * 1e-10, dtype=np.complex64)),
    ]

    # Phase discontinuity
    phase_jump = np.zeros(1000, dtype=np.complex64)
    for i in range(1000):
        phase = (i * 2 * math.pi / 100) % (2 * math.pi)
        if i == 500:
            phase += math.pi
        phase_jump[i] = np.exp(1j * phase)
    test_cases.append(("Phase discontinuity (180° jump)", phase_jump))

    # Frequency offset
    freq_offset = np.zeros(1000, dtype=np.complex64)
    for i in range(1000):
        phase = 2 * math.pi * 10000 * i / 250000  # 10kHz offset
        freq_offset[i] = np.exp(1j * phase)
    test_cases.append(("Large frequency offset (10kHz)", freq_offset))

    # Test demod_gmsk
    print("Testing demod_gmsk:")
    for name, vector in test_cases:
        if test_edge_case(
            lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000),
            "demod_gmsk",
            name,
            vector
        ):
            results['passed'] += 1
        else:
            results['failed'] += 1

    # Summary
    print()
    print("=" * 70)
    print("Edge Case Test Summary")
    print("=" * 70)
    print(f"Passed: {results['passed']}")
    print(f"Failed: {results['failed']}")
    print(f"Total: {results['passed'] + results['failed']}")

    return 0 if results['failed'] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

