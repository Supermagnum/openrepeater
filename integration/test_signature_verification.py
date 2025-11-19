#!/usr/bin/env python3
"""
Signature Verification Test Script

Generates valid and invalid Brainpool signatures and tests the verification logic.
Uses Scapy to format and deliver test packets to the system under test.
"""

import json
import os
import sys
import time
from typing import List, Tuple

try:
    from cryptography.hazmat.backends import default_backend
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec

    CRYPTO_AVAILABLE = True
except ImportError:
    CRYPTO_AVAILABLE = False
    print("ERROR: cryptography library not available.")
    sys.exit(1)

try:
    from scapy.all import IP, UDP, Raw, send
    from scapy.packet import Packet

    SCAPY_AVAILABLE = True
except ImportError:
    SCAPY_AVAILABLE = False
    print("WARNING: Scapy not available. Install with: pip3 install scapy")
    print("Will use ZMQ directly instead.")

try:
    import zmq

    ZMQ_AVAILABLE = True
except ImportError:
    ZMQ_AVAILABLE = False
    print("ERROR: pyzmq not available. Install with: pip3 install pyzmq")
    sys.exit(1)


def generate_valid_signatures(message: bytes) -> Tuple[bytes, bytes, bytes, bytes]:
    """
    Generate 2 valid Brainpool signatures for testing.

    Returns:
        (signature1, signature2, public_key1_pem, public_key2_pem)
    """
    # Generate first key pair (BrainpoolP256R1)
    private_key1 = ec.generate_private_key(ec.BrainpoolP256R1(), default_backend())
    public_key1 = private_key1.public_key()
    signature1 = private_key1.sign(message, ec.ECDSA(hashes.SHA256()))

    # Generate second key pair (BrainpoolP384R1 for variety)
    private_key2 = ec.generate_private_key(ec.BrainpoolP384R1(), default_backend())
    public_key2 = private_key2.public_key()
    signature2 = private_key2.sign(message, ec.ECDSA(hashes.SHA256()))

    # Serialize public keys to PEM
    public_key1_pem = public_key1.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )
    public_key2_pem = public_key2.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )

    return signature1, signature2, public_key1_pem, public_key2_pem


def generate_invalid_signatures(message: bytes, valid_sig1: bytes, valid_sig2: bytes) -> List[Tuple[bytes, str]]:
    """
    Generate 20 invalid signatures for testing.

    Returns:
        List of (signature_bytes, description) tuples
    """
    invalid_sigs = []

    # 1. All zeros
    invalid_sigs.append((b"\x00" * 64, "All zeros"))

    # 2. All ones
    invalid_sigs.append((b"\xFF" * 64, "All ones"))

    # 3. Truncated signature (first half)
    invalid_sigs.append((valid_sig1[:32], "Truncated (first 32 bytes)"))

    # 4. Truncated signature (second half)
    invalid_sigs.append((valid_sig1[32:], "Truncated (last 32 bytes)"))

    # 5. Wrong length (too short)
    invalid_sigs.append((valid_sig1[:40], "Wrong length (too short)"))

    # 6. Wrong length (too long)
    invalid_sigs.append((valid_sig1 + b"\x00" * 20, "Wrong length (too long)"))

    # 7. Random bytes
    invalid_sigs.append((os.urandom(64), "Random bytes"))

    # 8. Random bytes (wrong size)
    invalid_sigs.append((os.urandom(32), "Random bytes (wrong size)"))

    # 9. Signature for different message
    different_message = b"DIFFERENT_MESSAGE_" + message
    private_key_temp = ec.generate_private_key(ec.BrainpoolP256R1(), default_backend())
    sig_different_msg = private_key_temp.sign(different_message, ec.ECDSA(hashes.SHA256()))
    invalid_sigs.append((sig_different_msg, "Signature for different message"))

    # 10. Signature from different key (but same message)
    private_key_other = ec.generate_private_key(ec.BrainpoolP256R1(), default_backend())
    sig_other_key = private_key_other.sign(message, ec.ECDSA(hashes.SHA256()))
    invalid_sigs.append((sig_other_key, "Signature from different key"))

    # 11. Corrupted signature (flip bits)
    corrupted = bytearray(valid_sig1)
    for i in range(0, len(corrupted), 8):
        corrupted[i] ^= 0xFF
    invalid_sigs.append((bytes(corrupted), "Corrupted (bit-flipped)"))

    # 12. Empty signature
    invalid_sigs.append((b"", "Empty signature"))

    # 13. Single byte
    invalid_sigs.append((b"\x42", "Single byte"))

    # 14. Reversed signature
    invalid_sigs.append((valid_sig1[::-1], "Reversed bytes"))

    # 15. Signature with null bytes in middle
    sig_with_null = valid_sig1[:32] + b"\x00" * 10 + valid_sig1[32:]
    invalid_sigs.append((sig_with_null, "Null bytes in middle"))

    # 16. Signature from wrong curve (secp256r1 instead of Brainpool)
    try:
        private_key_wrong_curve = ec.generate_private_key(ec.SECP256R1(), default_backend())
        sig_wrong_curve = private_key_wrong_curve.sign(message, ec.ECDSA(hashes.SHA256()))
        invalid_sigs.append((sig_wrong_curve, "Signature from wrong curve (SECP256R1)"))
    except Exception:
        # Fallback if curve not available
        invalid_sigs.append((b"\xAA" * 64, "Wrong curve (fallback)"))

    # 17. Signature with wrong hash algorithm (if we had signed with SHA1)
    # Note: We can't easily create this without the private key, so we'll use a pattern
    invalid_sigs.append((b"SHA1_SIG" + b"\x00" * 56, "Wrong hash algorithm (simulated)"))

    # 18. Duplicate bytes pattern
    invalid_sigs.append((b"\xAA\xBB" * 32, "Duplicate pattern"))

    # 19. Incremental bytes
    invalid_sigs.append((bytes(range(64)), "Incremental bytes"))

    # 20. Valid signature but for second message (mismatch)
    invalid_sigs.append((valid_sig2, "Valid signature but for different key/message"))

    return invalid_sigs


def create_test_packet(message: bytes, signature: bytes) -> Packet:
    """
    Create a Scapy packet containing the message and signature.

    Args:
        message: JSON command message bytes
        signature: Signature bytes

    Returns:
        Scapy packet
    """
    # Create packet with message and signature
    # Using IP/UDP/Raw to simulate network delivery
    packet = IP(dst="127.0.0.1") / UDP(dport=5555) / Raw(load=message + signature)
    return packet


def send_via_zmq(message: bytes, signature: bytes, host: str = "localhost", port: int = 5555):
    """
    Send message and signature via ZMQ (actual delivery method).

    Args:
        message: JSON command message bytes
        signature: Signature bytes
        host: ZMQ host
        port: ZMQ port
    """
    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        endpoint = f"tcp://{host}:{port}"
        socket.connect(endpoint)
        time.sleep(0.1)  # Give connection time to establish

        # Send two-part message: [json_bytes, signature]
        socket.send_multipart([message, signature])

    except Exception as e:
        print(f"ERROR: Failed to send via ZMQ: {e}")
    finally:
        socket.close()
        context.term()


def test_signature_verification(
    test_message: bytes,
    public_key_pem: bytes,
    signature: bytes,
    description: str,
    expected_valid: bool,
    use_scapy: bool = False,
):
    """
    Test signature verification.

    Args:
        test_message: Message bytes
        public_key_pem: Public key in PEM format
        signature: Signature bytes
        description: Test description
        expected_valid: Whether signature should be valid
        use_scapy: Whether to use Scapy for packet creation (for testing)
    """
    from cryptography.hazmat.backends import default_backend
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec

    print(f"\n{'='*60}")
    print(f"Test: {description}")
    print(f"Expected: {'VALID' if expected_valid else 'INVALID'}")
    print(f"Signature length: {len(signature)} bytes")

    # Create Scapy packet (for testing packet creation)
    if use_scapy and SCAPY_AVAILABLE:
        packet = create_test_packet(test_message, signature)
        print(f"Scapy packet created: {len(packet)} bytes total")

    # Test verification
    try:
        public_key = serialization.load_pem_public_key(public_key_pem, backend=default_backend())

        if isinstance(public_key, ec.EllipticCurvePublicKey):
            public_key.verify(signature, test_message, ec.ECDSA(hashes.SHA256()))
            is_valid = True
            result = "VALID"
        else:
            is_valid = False
            result = "INVALID (wrong key type)"
    except Exception as e:
        is_valid = False
        result = f"INVALID ({type(e).__name__}: {str(e)[:50]})"

    # Check if result matches expectation
    status = "PASS" if (is_valid == expected_valid) else "FAIL"
    print(f"Result: {result}")
    print(f"Status: {status}")

    return status == "PASS"


def main():
    """Main test execution."""
    print("=" * 60)
    print("Signature Verification Test Suite")
    print("=" * 60)

    # Create test message (JSON command)
    test_command = {
        "callsign": "LA1ABC",
        "command": "SET_SQUELCH -24",
        "timestamp": time.time(),
    }
    test_message = json.dumps(test_command).encode("utf-8")

    print(f"\nTest message: {test_message.decode('utf-8')}")
    print(f"Message length: {len(test_message)} bytes")

    # Generate valid signatures
    print("\n" + "=" * 60)
    print("Generating valid signatures...")
    sig1_valid, sig2_valid, pubkey1_pem, pubkey2_pem = generate_valid_signatures(test_message)
    print(f"Valid signature 1: {len(sig1_valid)} bytes")
    print(f"Valid signature 2: {len(sig2_valid)} bytes")

    # Generate invalid signatures
    print("\n" + "=" * 60)
    print("Generating invalid signatures...")
    invalid_sigs = generate_invalid_signatures(test_message, sig1_valid, sig2_valid)
    print(f"Generated {len(invalid_sigs)} invalid signatures")

    # Test valid signatures
    print("\n" + "=" * 60)
    print("TESTING VALID SIGNATURES")
    print("=" * 60)

    results = []

    # Test valid signature 1
    result1 = test_signature_verification(
        test_message, pubkey1_pem, sig1_valid, "Valid signature 1 (BrainpoolP256R1)", True, use_scapy=True
    )
    results.append(("Valid signature 1", result1))

    # Test valid signature 2
    result2 = test_signature_verification(
        test_message, pubkey2_pem, sig2_valid, "Valid signature 2 (BrainpoolP384R1)", True, use_scapy=True
    )
    results.append(("Valid signature 2", result2))

    # Test invalid signatures
    print("\n" + "=" * 60)
    print("TESTING INVALID SIGNATURES")
    print("=" * 60)

    for i, (invalid_sig, description) in enumerate(invalid_sigs, 1):
        result = test_signature_verification(
            test_message,
            pubkey1_pem,  # Use first public key for all invalid tests
            invalid_sig,
            f"Invalid signature {i}: {description}",
            False,
            use_scapy=True,
        )
        results.append((f"Invalid signature {i}: {description}", result))

    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)

    passed = sum(1 for _, result in results if result)
    total = len(results)

    for test_name, result in results:
        status = "PASS" if result else "FAIL"
        print(f"{status}: {test_name}")

    print(f"\nTotal: {passed}/{total} tests passed")

    # Demonstrate Scapy packet delivery (without actually sending)
    if SCAPY_AVAILABLE:
        print("\n" + "=" * 60)
        print("SCAPY PACKET CREATION DEMONSTRATION")
        print("=" * 60)

        # Create example packets
        packet_valid = create_test_packet(test_message, sig1_valid)
        packet_invalid = create_test_packet(test_message, invalid_sigs[0][0])

        print(f"Valid signature packet: {len(packet_valid)} bytes")
        print(f"Invalid signature packet: {len(packet_invalid)} bytes")
        print("\nNote: Use send_via_zmq() to actually deliver to system under test")
        print("      Scapy packets shown here for format demonstration")

    # Option to actually send via ZMQ
    if len(sys.argv) > 1 and sys.argv[1] == "--send":
        print("\n" + "=" * 60)
        print("SENDING TEST PACKETS VIA ZMQ")
        print("=" * 60)

        # Send valid signature
        print("\nSending valid signature 1...")
        send_via_zmq(test_message, sig1_valid)
        time.sleep(0.5)

        # Send a few invalid signatures
        print("Sending invalid signatures (should be rejected)...")
        for i, (invalid_sig, description) in enumerate(invalid_sigs[:5], 1):
            print(f"  Sending invalid signature {i}: {description}")
            send_via_zmq(test_message, invalid_sig)
            time.sleep(0.2)

        print("\nTest packets sent. Check system logs for verification results.")

    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())

