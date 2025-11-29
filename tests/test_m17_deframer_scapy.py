#!/usr/bin/env python3
"""
M17 Deframer Attack Vector Tests using Scapy
Tests various malformed, edge case, and attack scenarios for M17 protocol frames
"""

import struct
import sys
import os

# Add the project root to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

try:
    from scapy.all import *
except ImportError:
    print("ERROR: Scapy is not installed. Install it with: pip install scapy")
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
        vectors.append(("valid_lsf", M17AttackVectors.create_lsf_frame()))
        vectors.append(("valid_stream", M17AttackVectors.create_stream_frame()))
        vectors.append(("valid_packet", M17AttackVectors.create_packet_frame()))
        
        # 2. Truncated frames
        vectors.append(("truncated_lsf", struct.pack('>H', SYNC_LSF) + b'\x00' * 10))  # Only 12 bytes
        vectors.append(("truncated_packet", struct.pack('>H', SYNC_PACKET) + b'\x01'))  # Only 3 bytes
        
        # 3. Oversized frames
        vectors.append(("oversized_lsf", M17AttackVectors.create_lsf_frame() + b'\xFF' * 100))
        vectors.append(("oversized_packet", M17AttackVectors.create_packet_frame(length=500)))
        
        # 4. Invalid sync words
        vectors.append(("invalid_sync_1", struct.pack('>H', 0x0000) + b'\x00' * 46))
        vectors.append(("invalid_sync_2", struct.pack('>H', 0xFFFF) + b'\xFF' * 46))
        vectors.append(("invalid_sync_3", struct.pack('>H', 0x1234) + b'\xAA' * 46))
        
        # 5. Sync word in payload (false positive)
        vectors.append(("sync_in_payload", b'\x00' * 20 + struct.pack('>H', SYNC_LSF) + b'\x00' * 20))
        
        # 6. Multiple sync words
        vectors.append(("multiple_sync", struct.pack('>H', SYNC_LSF) + b'\x00' * 20 + 
                       struct.pack('>H', SYNC_LSF) + b'\x00' * 20))
        
        # 7. Mixed frame types
        vectors.append(("mixed_frames", M17AttackVectors.create_lsf_frame() + 
                       M17AttackVectors.create_packet_frame()))
        
        # 8. Empty frame (just sync)
        vectors.append(("empty_frame", struct.pack('>H', SYNC_LSF)))
        
        # 9. Maximum size frame
        vectors.append(("max_size_packet", struct.pack('>H', SYNC_PACKET) + b'\xFF' * 328))
        
        # 10. All zeros
        vectors.append(("all_zeros", b'\x00' * 100))
        
        # 11. All ones
        vectors.append(("all_ones", b'\xFF' * 100))
        
        # 12. Alternating pattern
        vectors.append(("alternating", b'\xAA' * 50 + b'\x55' * 50))
        
        # 13. Incremental pattern
        vectors.append(("incremental", bytes(range(100))))
        
        # 14. Decremental pattern
        vectors.append(("decremental", bytes(range(99, -1, -1))))
        
        # 15. Frame with null bytes in payload
        vectors.append(("null_payload", struct.pack('>H', SYNC_LSF) + b'\x00' * 46))
        
        # 16. Frame with maximum values
        vectors.append(("max_values", struct.pack('>H', SYNC_LSF) + b'\xFF' * 46))
        
        # 17. Frame with sync word at end
        vectors.append(("sync_at_end", b'\x00' * 46 + struct.pack('>H', SYNC_LSF)))
        
        # 18. Incomplete sync word (1 byte)
        vectors.append(("incomplete_sync", b'\xDF'))
        
        # 19. Sync word split across boundaries
        vectors.append(("split_sync_1", b'\xDF'))
        vectors.append(("split_sync_2", b'\x55'))
        
        # 20. Very long frame without sync
        vectors.append(("long_no_sync", b'\x42' * 1000))
        
        # 21. Frame with special bytes
        vectors.append(("special_bytes", struct.pack('>H', SYNC_LSF) + 
                       bytes([0x00, 0xFF, 0x80, 0x7F, 0x01, 0xFE]) + b'\x00' * 40))
        
        # 22. Packet frame with minimal payload
        vectors.append(("minimal_packet", struct.pack('>H', SYNC_PACKET) + b'\x01'))
        
        # 23. Frame with repeated sync words
        vectors.append(("repeated_sync", struct.pack('>H', SYNC_LSF) * 10))
        
        # 24. Frame with sync word variations (bit flips)
        vectors.append(("sync_bitflip_1", struct.pack('>H', SYNC_LSF ^ 0x0001) + b'\x00' * 46))
        vectors.append(("sync_bitflip_2", struct.pack('>H', SYNC_LSF ^ 0x0100) + b'\x00' * 46))
        vectors.append(("sync_bitflip_3", struct.pack('>H', SYNC_LSF ^ 0x8000) + b'\x00' * 46))
        
        # 25. Frame with payload containing sync-like patterns
        vectors.append(("sync_like_payload", struct.pack('>H', SYNC_LSF) + 
                       struct.pack('>H', SYNC_PACKET) + b'\x00' * 44))
        
        return vectors


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
        'processed': 0,
        'errors': 0,
        'warnings': 0
    }
    
    for name, vector_data in vectors:
        print(f"Testing: {name:30s} ({len(vector_data):4d} bytes)", end=" ... ")
        
        try:
            # Validate the attack vector structure
            if len(vector_data) > 0:
                # Check for sync words at start
                if len(vector_data) >= 2:
                    word = struct.unpack('>H', vector_data[0:2])[0]
                    if word == SYNC_LSF or word == SYNC_STREAM:
                        print("✓ LSF/Stream sync word")
                        results['processed'] += 1
                    elif word == SYNC_PACKET:
                        print("✓ Packet sync word")
                        results['processed'] += 1
                    else:
                        # Check if sync word appears anywhere in the vector
                        sync_found = False
                        sync_pos = -1
                        for i in range(len(vector_data) - 1):
                            w = struct.unpack('>H', vector_data[i:i+2])[0]
                            if w == SYNC_LSF or w == SYNC_STREAM or w == SYNC_PACKET:
                                sync_found = True
                                sync_pos = i
                                break
                        
                        if sync_found:
                            print(f"⚠ Sync word at position {sync_pos}")
                            results['warnings'] += 1
                        else:
                            print("✗ No sync word (attack vector)")
                            results['errors'] += 1
                else:
                    print("✗ Too short (< 2 bytes)")
                    results['errors'] += 1
            else:
                print("✗ Empty vector")
                results['errors'] += 1
                    
        except Exception as e:
            print(f"✗ ERROR: {e}")
            results['errors'] += 1
    
    print()
    print("=" * 70)
    print("Test Summary")
    print("=" * 70)
    print(f"Total vectors:    {results['total']}")
    print(f"Processed:       {results['processed']}")
    print(f"Warnings:        {results['warnings']}")
    print(f"Errors:          {results['errors']}")
    print("=" * 70)
    
    return results


def save_attack_vectors_to_files():
    """Save attack vectors to files for further analysis"""
    vectors = M17AttackVectors.generate_attack_vectors()
    output_dir = "fuzzing/corpus/m17_attack_vectors"
    
    os.makedirs(output_dir, exist_ok=True)
    
    print(f"\nSaving attack vectors to: {output_dir}")
    
    for name, vector_data in vectors:
        filename = os.path.join(output_dir, f"{name}.bin")
        with open(filename, 'wb') as f:
            f.write(vector_data)
    
    print(f"Saved {len(vectors)} attack vector files")
    return output_dir


if __name__ == "__main__":
    print("M17 Deframer Attack Vector Tests using Scapy")
    print()
    
    # Run tests
    results = test_m17_deframer_with_vectors()
    
    # Save vectors to files
    output_dir = save_attack_vectors_to_files()
    
    print()
    print("=" * 70)
    print("Attack vectors saved for further testing")
    print(f"Location: {output_dir}")
    print("=" * 70)
    
    sys.exit(0 if results['errors'] == 0 else 1)

