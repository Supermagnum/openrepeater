#!/usr/bin/env python3
"""
M17 Deframer Attack Vector Tests
Tests the actual M17 deframer implementation with various attack vectors
"""

import struct
import sys
import os
import numpy as np

# Add the project root to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

try:
    from gnuradio import gr, blocks, qradiolink
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    print("Make sure GNU Radio is installed and the module is built")
    sys.exit(1)

# M17 sync words
SYNC_LSF = 0xDF55      # Link Setup Frame
SYNC_STREAM = 0xDF55   # Stream frame (same as LSF)
SYNC_PACKET = 0x9FF6   # Packet frame

# M17 frame sizes
LSF_FRAME_SIZE = 48    # LSF/Stream frame total size (including sync)
PACKET_FRAME_MIN = 2   # Minimum packet frame (just sync word)


class M17AttackVectors:
    """Generate M17 protocol attack vectors and test cases"""

    @staticmethod
    def create_lsf_frame(payload=None):
        """Create a valid LSF frame"""
        if payload is None:
            payload = b'\x00' * 46
        frame = struct.pack('>H', SYNC_LSF) + payload[:46]
        return frame

    @staticmethod
    def create_stream_frame(payload=None):
        """Create a valid Stream frame"""
        if payload is None:
            payload = b'\x00' * 46
        frame = struct.pack('>H', SYNC_STREAM) + payload[:46]
        return frame

    @staticmethod
    def create_packet_frame(payload=None, length=100):
        """Create a valid Packet frame"""
        if payload is None:
            payload = b'\x01' * (length - 2)
        frame = struct.pack('>H', SYNC_PACKET) + payload[:length-2]
        return frame

    @staticmethod
    def generate_attack_vectors():
        """Generate various attack vectors for M17 deframer"""
        vectors = []

        # 1. Valid frames (baseline)
        vectors.append(("valid_lsf", M17AttackVectors.create_lsf_frame(), True))
        vectors.append(("valid_stream", M17AttackVectors.create_stream_frame(), True))
        vectors.append(("valid_packet", M17AttackVectors.create_packet_frame(), True))

        # 2. Truncated frames (should be rejected - frame length validation)
        vectors.append(("truncated_lsf", struct.pack('>H', SYNC_LSF) + b'\x00' * 10, False))
        vectors.append(("truncated_packet", struct.pack('>H', SYNC_PACKET) + b'\x01', False))

        # 3. Oversized frames (deframer extracts first valid frame, which is correct)
        vectors.append(("oversized_lsf", M17AttackVectors.create_lsf_frame() + b'\xFF' * 100, True))
        vectors.append(("oversized_packet", M17AttackVectors.create_packet_frame(length=500), True))

        # 4. Invalid sync words (should be rejected)
        vectors.append(("invalid_sync_1", struct.pack('>H', 0x0000) + b'\x00' * 46, False))
        vectors.append(("invalid_sync_2", struct.pack('>H', 0xFFFF) + b'\xFF' * 46, False))
        vectors.append(("invalid_sync_3", struct.pack('>H', 0x1234) + b'\xAA' * 46, False))

        # 5. Sync word in payload (false positive - should find sync but may extract wrong frame)
        vectors.append(("sync_in_payload", b'\x00' * 20 + struct.pack('>H', SYNC_LSF) + b'\x00' * 20, True))

        # 6. Multiple sync words (should extract first valid frame)
        vectors.append(("multiple_sync", struct.pack('>H', SYNC_LSF) + b'\x00' * 20 +
                       struct.pack('>H', SYNC_LSF) + b'\x00' * 20, True))

        # 7. Mixed frame types (should extract first frame)
        vectors.append(("mixed_frames", M17AttackVectors.create_lsf_frame() +
                       M17AttackVectors.create_packet_frame(), True))

        # 8. Empty frame (just sync - should extract empty payload)
        vectors.append(("empty_frame", struct.pack('>H', SYNC_LSF), True))

        # 9. Maximum size packet frame
        vectors.append(("max_size_packet", struct.pack('>H', SYNC_PACKET) + b'\xFF' * 328, True))

        # 10. All zeros (no sync - should be rejected)
        vectors.append(("all_zeros", b'\x00' * 100, False))

        # 11. All ones (no sync - should be rejected)
        vectors.append(("all_ones", b'\xFF' * 100, False))

        # 12. Alternating pattern (no sync - should be rejected)
        vectors.append(("alternating", b'\xAA' * 50 + b'\x55' * 50, False))

        # 13. Incremental pattern (no sync - should be rejected)
        vectors.append(("incremental", bytes(range(100)), False))

        # 14. Decremental pattern (no sync - should be rejected)
        vectors.append(("decremental", bytes(range(99, -1, -1)), False))

        # 15. Frame with null bytes in payload (valid frame)
        vectors.append(("null_payload", struct.pack('>H', SYNC_LSF) + b'\x00' * 46, True))

        # 16. Frame with maximum values (valid frame)
        vectors.append(("max_values", struct.pack('>H', SYNC_LSF) + b'\xFF' * 46, True))

        # 17. Frame with sync word at end (should find it)
        vectors.append(("sync_at_end", b'\x00' * 46 + struct.pack('>H', SYNC_LSF), True))

        # 18. Incomplete sync word (1 byte - should be rejected)
        vectors.append(("incomplete_sync", b'\xDF', False))

        # 19. Sync word split across boundaries (should handle correctly)
        vectors.append(("split_sync_1", b'\xDF' + b'\x55' + b'\x00' * 46, True))
        vectors.append(("split_sync_2", b'\x9F' + b'\xF6' + b'\x01' * 46, True))

        # 20. Very long frame without sync (should be rejected)
        vectors.append(("long_no_sync", b'\x42' * 1000, False))

        # 21. Frame with special bytes (valid frame)
        vectors.append(("special_bytes", struct.pack('>H', SYNC_LSF) +
                       bytes([0x00, 0xFF, 0x80, 0x7F, 0x01, 0xFE]) + b'\x00' * 40, True))

        # 22. Packet frame with minimal payload
        vectors.append(("minimal_packet", struct.pack('>H', SYNC_PACKET) + b'\x01', True))

        # 23. Frame with repeated sync words (should extract first frame)
        vectors.append(("repeated_sync", struct.pack('>H', SYNC_LSF) * 10, True))

        # 24. Frame with sync word variations (bit flips - should be rejected)
        vectors.append(("sync_bitflip_1", struct.pack('>H', SYNC_LSF ^ 0x0001) + b'\x00' * 46, False))
        vectors.append(("sync_bitflip_2", struct.pack('>H', SYNC_LSF ^ 0x0100) + b'\x00' * 46, False))
        vectors.append(("sync_bitflip_3", struct.pack('>H', SYNC_LSF ^ 0x8000) + b'\x00' * 46, False))

        # 25. Frame with payload containing sync-like patterns (valid frame)
        vectors.append(("sync_like_payload", struct.pack('>H', SYNC_LSF) +
                       struct.pack('>H', SYNC_PACKET) + b'\x00' * 44, True))

        # 26. Preamble before frame (should find sync and extract frame)
        vectors.append(("preamble_frame", b'\x00' * 4 + struct.pack('>H', SYNC_LSF) + b'\x00' * 46, True))

        return vectors


def test_deframer_with_vector(vector_name, vector_data, should_extract):
    """Test M17 deframer with a single attack vector"""
    try:
        # Import pmt for tag handling
        try:
            from gnuradio import pmt
        except ImportError:
            pmt = None

        # Create a top block
        tb = gr.top_block()

        # Create blocks - convert bytes to list of integers
        vector_list = list(vector_data)
        source = blocks.vector_source_b(vector_list, False, 1, [])
        deframer = qradiolink.m17_deframer(330)  # Max frame length
        sink = blocks.vector_sink_b(1)

        # Connect blocks
        tb.connect(source, deframer)
        tb.connect(deframer, sink)

        # Run the flowgraph
        # The deframer is a sync_block that may return 0 output when no sync is found
        # We need to manually stop it after processing
        import time
        tb.start()

        # Give it time to process the data (small vectors should process quickly)
        # For larger vectors, we may need more time
        time.sleep(0.2 if len(vector_data) < 100 else 0.5)

        # Stop the flowgraph manually
        tb.stop()
        tb.wait()
        tb = None

        # Get output
        output_data = sink.data()
        tags = sink.tags()

        # Check if frame was extracted
        frame_extracted = len(output_data) > 0

        # Analyze result
        if should_extract:
            if frame_extracted:
                # Check frame type tag if available
                frame_type = None
                if pmt and tags:
                    for tag in tags:
                        try:
                            key = pmt.to_python(tag.key) if pmt else None
                            if key == 'frame_type':
                                frame_type = pmt.to_python(tag.value) if pmt else None
                                break
                        except:
                            pass

                type_str = f", type: {frame_type}" if frame_type else ""
                return True, f"Frame extracted ({len(output_data)} bytes{type_str})"
            else:
                return False, "Expected frame extraction but got none"
        else:
            if frame_extracted:
                return False, f"Unexpected frame extraction ({len(output_data)} bytes)"
            else:
                return True, "Correctly rejected invalid frame"

    except Exception as e:
        return False, f"ERROR: {e}"


def test_m17_deframer_with_vectors():
    """Test M17 deframer with attack vectors"""
    print("=" * 70)
    print("M17 Deframer Attack Vector Tests")
    print("=" * 70)
    print()

    # Generate attack vectors
    vectors = M17AttackVectors.generate_attack_vectors()

    print(f"Generated {len(vectors)} attack vectors")
    print()

    # Test each vector
    results = {
        'total': len(vectors),
        'passed': 0,
        'failed': 0,
        'errors': 0
    }

    for name, vector_data, should_extract in vectors:
        print(f"Testing: {name:30s} ({len(vector_data):4d} bytes)", end=" ... ")

        try:
            passed, message = test_deframer_with_vector(name, vector_data, should_extract)
            if passed:
                print(f"✓ {message}")
                results['passed'] += 1
            else:
                print(f"✗ {message}")
                results['failed'] += 1
        except Exception as e:
            print(f"✗ EXCEPTION: {e}")
            results['errors'] += 1

    print()
    print("=" * 70)
    print("Test Summary")
    print("=" * 70)
    print(f"Total vectors:    {results['total']}")
    print(f"Passed:           {results['passed']}")
    print(f"Failed:           {results['failed']}")
    print(f"Errors:           {results['errors']}")
    print("=" * 70)

    return results


if __name__ == "__main__":
    print("M17 Deframer Attack Vector Tests")
    print()

    # Run tests
    results = test_m17_deframer_with_vectors()

    print()
    print("=" * 70)
    print("Attack vector testing complete")
    print("=" * 70)

    sys.exit(0 if results['failed'] == 0 and results['errors'] == 0 else 1)

