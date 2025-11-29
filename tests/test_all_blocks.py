#!/usr/bin/env python3
"""
Comprehensive test for all qradiolink blocks
Tests all modulation/demodulation blocks with edge cases
"""

import sys
import numpy as np
import math

try:
    from gnuradio import gr
    from gnuradio import blocks
    from gnuradio import qradiolink
    from gnuradio import vocoder
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    sys.exit(1)


def generate_test_vector(samples=1000):
    """Generate a simple test vector"""
    signal = np.zeros(samples, dtype=np.complex64)
    for i in range(samples):
        phase = 2 * math.pi * 1700 * i / 250000
        signal[i] = np.exp(1j * phase) * 0.5
    return signal


def test_block(block_maker, block_name, test_vector, has_multiple_outputs=True):
    """Test a block with a test vector"""
    print(f"Testing {block_name}...", end=" ")
    
    try:
        tb = gr.top_block()
        block = block_maker()
        source = blocks.vector_source_c(test_vector.tolist(), False)
        
        # Connect all possible outputs
        sinks = []
        for i in range(4):
            try:
                if i == 0:
                    sink = blocks.null_sink(gr.sizeof_gr_complex)
                elif i == 1:
                    sink = blocks.null_sink(gr.sizeof_gr_complex)
                else:
                    sink = blocks.null_sink(gr.sizeof_char)
                sinks.append(sink)
                if i == 0:
                    tb.connect(block, sink)
                else:
                    tb.connect((block, i), sink)
            except:
                break
        
        tb.connect(source, block)
        
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
    """Test all blocks"""
    print("=" * 70)
    print("Comprehensive Block Coverage Test")
    print("=" * 70)
    print()
    
    test_vector = generate_test_vector(1000)
    results = {'passed': 0, 'failed': 0, 'skipped': 0}
    
    # List of all blocks to test
    blocks_to_test = [
        # Modulation blocks
        ("mod_2fsk", lambda: qradiolink.mod_2fsk(125, 250000, 1700, 8000, False)),
        ("mod_4fsk", lambda: qradiolink.mod_4fsk(125, 250000, 1700, 8000, True)),
        ("mod_gmsk", lambda: qradiolink.mod_gmsk(125, 250000, 1700, 8000)),
        ("mod_bpsk", lambda: qradiolink.mod_bpsk(125, 250000, 1700, 8000)),
        ("mod_qpsk", lambda: qradiolink.mod_qpsk(125, 250000, 1700, 8000)),
        ("mod_am", lambda: qradiolink.mod_am(125, 250000, 1700, 8000)),
        ("mod_ssb", lambda: qradiolink.mod_ssb(125, 250000, 1700, 8000, 0)),
        ("mod_nbfm", lambda: qradiolink.mod_nbfm(125, 250000, 1700, 8000)),
        ("mod_dsss", lambda: qradiolink.mod_dsss(25, 250000, 1700, 8000)),
        ("mod_m17", lambda: qradiolink.mod_m17(125, 250000, 1700, 8000)),
        ("mod_dmr", lambda: qradiolink.mod_dmr(125, 250000, 1700, 8000)),
        ("mod_mmdvm", lambda: qradiolink.mod_mmdvm(125, 250000, 1700, 8000)),
        
        # Demodulation blocks
        ("demod_2fsk", lambda: qradiolink.demod_2fsk(125, 250000, 1700, 8000, False)),
        ("demod_4fsk", lambda: qradiolink.demod_4fsk(125, 250000, 1700, 8000, True)),
        ("demod_gmsk", lambda: qradiolink.demod_gmsk(10, 250000, 1700, 8000)),
        ("demod_bpsk", lambda: qradiolink.demod_bpsk(125, 250000, 1700, 8000)),
        ("demod_qpsk", lambda: qradiolink.demod_qpsk(125, 250000, 1700, 8000)),
        ("demod_am", lambda: qradiolink.demod_am(125, 250000, 1700, 8000)),
        ("demod_ssb", lambda: qradiolink.demod_ssb(125, 250000, 1700, 8000, 0)),
        ("demod_nbfm", lambda: qradiolink.demod_nbfm(125, 250000, 1700, 8000)),
        ("demod_dsss", lambda: qradiolink.demod_dsss(25, 250000, 1700, 8000)),
        ("demod_m17", lambda: qradiolink.demod_m17(125, 250000, 1700, 8000)),
        ("demod_dmr", lambda: qradiolink.demod_dmr(125, 250000, 1700, 8000)),
    ]
    
    # FreeDV blocks (may require vocoder)
    try:
        blocks_to_test.append(("mod_freedv", lambda: qradiolink.mod_freedv(125, 8000, 1700, 2000, 200, vocoder.freedv_api.MODE_1600, 0)))
        blocks_to_test.append(("demod_freedv", lambda: qradiolink.demod_freedv(125, 8000, 1700, 2000, 200, vocoder.freedv_api.MODE_1600, 0)))
    except:
        print("Note: FreeDV blocks not available (vocoder not installed)")
    
    # MMDVM multi blocks
    try:
        blocks_to_test.append(("demod_mmdvm_multi", lambda: qradiolink.demod_mmdvm_multi(125, 250000, 1700, 8000)))
        blocks_to_test.append(("demod_mmdvm_multi2", lambda: qradiolink.demod_mmdvm_multi2(125, 250000, 1700, 8000)))
        blocks_to_test.append(("mod_mmdvm_multi2", lambda: qradiolink.mod_mmdvm_multi2(125, 250000, 1700, 8000)))
    except:
        pass
    
    # RSSI block
    try:
        blocks_to_test.append(("rssi_tag_block", lambda: qradiolink.rssi_tag_block(1000)))
    except:
        pass
    
    # M17 deframer
    try:
        blocks_to_test.append(("m17_deframer", lambda: qradiolink.m17_deframer(330)))
    except:
        pass
    
    print(f"Testing {len(blocks_to_test)} blocks...\n")
    
    for block_name, block_maker in blocks_to_test:
        if test_block(block_maker, block_name, test_vector):
            results['passed'] += 1
        else:
            results['failed'] += 1
    
    # Summary
    print()
    print("=" * 70)
    print("Test Summary")
    print("=" * 70)
    print(f"Passed: {results['passed']}")
    print(f"Failed: {results['failed']}")
    print(f"Total: {results['passed'] + results['failed']}")
    
    return 0 if results['failed'] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

