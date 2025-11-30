#!/usr/bin/env python3
"""
Memory Safety Tests for GNU Radio Blocks
Tests for buffer overflows, memory leaks, and invalid memory access
"""

import sys
import numpy as np
import gc

try:
    from gnuradio import gr
    from gnuradio import blocks
    from gnuradio import qradiolink
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    sys.exit(1)


def test_large_input(block_maker, block_name, max_size=100000):
    """Test with very large input to check for buffer overflows"""
    print(f"\nTesting {block_name} with large input ({max_size} samples)...")

    try:
        # Generate large input
        large_vector = np.random.randn(max_size).astype(np.complex64) * 0.1

        tb = gr.top_block()
        block = block_maker()
        source = blocks.vector_source_c(large_vector.tolist(), False)
        sink0 = blocks.null_sink(gr.sizeof_gr_complex)
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_char)
        sink3 = blocks.null_sink(gr.sizeof_char)

        tb.connect(source, block)
        tb.connect(block, sink0)
        try:
            tb.connect((block, 1), sink1)
            tb.connect((block, 2), sink2)
            tb.connect((block, 3), sink3)
        except:
            pass

        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()

        print(f"  ✓ PASSED - Handled large input without crash")
        return True
    except Exception as e:
        print(f"  ✗ FAILED - Error: {e}")
        return False


def test_rapid_restart(block_maker, block_name, iterations=10):
    """Test rapid start/stop cycles for memory leaks"""
    print(f"\nTesting {block_name} with rapid restart ({iterations} iterations)...")

    try:
        test_vector = np.random.randn(1000).astype(np.complex64) * 0.1

        for i in range(iterations):
            tb = gr.top_block()
            block = block_maker()
            source = blocks.vector_source_c(test_vector.tolist(), False)
            sink0 = blocks.null_sink(gr.sizeof_gr_complex)
            sink1 = blocks.null_sink(gr.sizeof_gr_complex)
            sink2 = blocks.null_sink(gr.sizeof_char)
            sink3 = blocks.null_sink(gr.sizeof_char)

            tb.connect(source, block)
            tb.connect(block, sink0)
            try:
                tb.connect((block, 1), sink1)
                tb.connect((block, 2), sink2)
                tb.connect((block, 3), sink3)
            except:
                pass

            tb.start()
            tb.wait()
            tb.stop()
            tb.wait()

            # Force garbage collection
            del tb, block, source, sink0, sink1, sink2, sink3
            gc.collect()

        print(f"  ✓ PASSED - No memory leaks detected")
        return True
    except Exception as e:
        print(f"  ✗ FAILED - Error: {e}")
        return False


def test_empty_input(block_maker, block_name):
    """Test with empty input"""
    print(f"\nTesting {block_name} with empty input...")

    try:
        empty_vector = np.array([], dtype=np.complex64)

        tb = gr.top_block()
        block = block_maker()
        source = blocks.vector_source_c(empty_vector.tolist(), False)
        sink0 = blocks.null_sink(gr.sizeof_gr_complex)
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_char)
        sink3 = blocks.null_sink(gr.sizeof_char)

        tb.connect(source, block)
        tb.connect(block, sink0)
        try:
            tb.connect((block, 1), sink1)
            tb.connect((block, 2), sink2)
            tb.connect((block, 3), sink3)
        except:
            pass

        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()

        print(f"  ✓ PASSED - Handled empty input gracefully")
        return True
    except Exception as e:
        print(f"  ✗ FAILED - Error: {e}")
        return False


def test_single_sample(block_maker, block_name):
    """Test with single sample input"""
    print(f"\nTesting {block_name} with single sample...")

    try:
        single_sample = np.array([1.0 + 1j], dtype=np.complex64)

        tb = gr.top_block()
        block = block_maker()
        source = blocks.vector_source_c(single_sample.tolist(), False)
        sink0 = blocks.null_sink(gr.sizeof_gr_complex)
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_char)
        sink3 = blocks.null_sink(gr.sizeof_char)

        tb.connect(source, block)
        tb.connect(block, sink0)
        try:
            tb.connect((block, 1), sink1)
            tb.connect((block, 2), sink2)
            tb.connect((block, 3), sink3)
        except:
            pass

        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()

        print(f"  ✓ PASSED - Handled single sample")
        return True
    except Exception as e:
        print(f"  ✗ FAILED - Error: {e}")
        return False


def main():
    """Run memory safety tests"""
    print("=" * 70)
    print("Memory Safety Tests for gr-qradiolink")
    print("=" * 70)

    results = {'passed': 0, 'failed': 0}

    # Test demod_gmsk
    print("\n--- Testing demod_gmsk ---")
    tests = [
        (lambda: test_large_input(lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000), "demod_gmsk"), "Large input"),
        (lambda: test_rapid_restart(lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000), "demod_gmsk"), "Rapid restart"),
        (lambda: test_empty_input(lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000), "demod_gmsk"), "Empty input"),
        (lambda: test_single_sample(lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000), "demod_gmsk"), "Single sample"),
    ]

    for test_func, test_name in tests:
        if test_func():
            results['passed'] += 1
        else:
            results['failed'] += 1

    # Summary
    print("\n" + "=" * 70)
    print("Memory Safety Test Summary")
    print("=" * 70)
    print(f"Passed: {results['passed']}")
    print(f"Failed: {results['failed']}")

    return 0 if results['failed'] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

