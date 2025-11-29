#!/usr/bin/env python3
"""
Coverage Analysis - Check which blocks are available and tested
"""

import sys

try:
    from gnuradio import qradiolink
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    sys.exit(1)


def main():
    """Analyze block coverage"""
    print("=" * 70)
    print("Block Coverage Analysis")
    print("=" * 70)
    print()
    
    # Get all available blocks
    available_blocks = []
    for attr in dir(qradiolink):
        if not attr.startswith('_'):
            try:
                obj = getattr(qradiolink, attr)
                if callable(obj):
                    available_blocks.append(attr)
            except:
                pass
    
    available_blocks = sorted(available_blocks)
    
    # Expected blocks from user's list
    expected_blocks = {
        'Modulation': ['mod_2fsk', 'mod_4fsk', 'mod_gmsk', 'mod_bpsk', 'mod_qpsk', 
                       'mod_am', 'mod_ssb', 'mod_nbfm', 'mod_dsss', 'mod_freedv',
                       'mod_m17', 'mod_dmr', 'mod_mmdvm'],
        'Demodulation': ['demod_2fsk', 'demod_4fsk', 'demod_gmsk', 'demod_bpsk', 'demod_qpsk',
                       'demod_am', 'demod_ssb', 'demod_nbfm', 'demod_dsss', 'demod_freedv',
                       'demod_m17', 'demod_dmr', 'demod_mmdvm'],
        'Supporting': ['rssi_tag_block', 'm17_deframer']
    }
    
    # Currently tested blocks (from test_modulation_vectors.py)
    tested_blocks = {
        'mod_gmsk', 'mod_2fsk',
        'demod_gmsk', 'demod_2fsk'
    }
    
    # M17 deframer tested separately
    tested_blocks.add('m17_deframer')
    
    print("Available Blocks in Python Bindings:")
    print("-" * 70)
    for block in available_blocks:
        status = "✓ TESTED" if block in tested_blocks else "✗ NOT TESTED"
        print(f"  {block:30s} {status}")
    
    print()
    print("Expected Blocks (from user list):")
    print("-" * 70)
    
    all_expected = set()
    for category, blocks in expected_blocks.items():
        print(f"\n{category}:")
        for block in blocks:
            all_expected.add(block)
            if block in available_blocks:
                status = "✓ AVAILABLE" + (" (TESTED)" if block in tested_blocks else " (NOT TESTED)")
            else:
                status = "✗ NOT IN PYTHON BINDINGS"
            print(f"  {block:30s} {status}")
    
    print()
    print("=" * 70)
    print("Summary")
    print("=" * 70)
    print(f"Available in Python: {len(available_blocks)}")
    print(f"Expected blocks: {len(all_expected)}")
    print(f"Currently tested: {len(tested_blocks)}")
    print(f"Missing from Python bindings: {len(all_expected - set(available_blocks))}")
    print(f"Available but not tested: {len(set(available_blocks) - tested_blocks)}")
    
    print()
    print("Missing from Python bindings:")
    missing = sorted(all_expected - set(available_blocks))
    for block in missing:
        print(f"  - {block}")
    
    print()
    print("Available but not tested:")
    not_tested = sorted(set(available_blocks) - tested_blocks)
    for block in not_tested:
        print(f"  - {block}")
    
    print()
    print("Note: FFT is part of GNU Radio core, not qradiolink")
    print("Note: CESSB is integrated into SSB blocks, not a separate block")


if __name__ == "__main__":
    main()

