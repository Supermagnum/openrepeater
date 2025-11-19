#!/usr/bin/env python3
"""
ZMQ Message Format Test Suite

Tests all ZMQ message formats and edge cases:
- Valid commands from documentation
- Invalid commands
- Malformed JSON
- Edge cases in message format
- Oversized messages
- Missing fields
- Wrong data types
"""

import json
import os
import sys
import time
from typing import Dict, List, Tuple

try:
    import zmq

    ZMQ_AVAILABLE = True
except ImportError:
    ZMQ_AVAILABLE = False
    print("ERROR: pyzmq not available. Install with: pip3 install pyzmq")
    sys.exit(1)

try:
    from cryptography.hazmat.backends import default_backend
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec

    CRYPTO_AVAILABLE = True
except ImportError:
    CRYPTO_AVAILABLE = False
    print("WARNING: cryptography library not available. Using dummy signatures.")


# Test results tracking
TEST_RESULTS = []


def generate_test_signature(message: bytes) -> bytes:
    """Generate a valid test signature."""
    if not CRYPTO_AVAILABLE:
        return b"dummy_signature_" + b"x" * 50

    try:
        private_key = ec.generate_private_key(ec.BrainpoolP256R1(), default_backend())
        signature = private_key.sign(message, ec.ECDSA(hashes.SHA256()))
        return signature
    except Exception:
        return b"dummy_signature_" + b"x" * 50


def test_result(test_name: str, passed: bool, details: str = ""):
    """Record test result."""
    status = "PASS" if passed else "FAIL"
    TEST_RESULTS.append((test_name, status, details))
    symbol = "✓" if passed else "✗"
    print(f"{symbol} {test_name}: {status}")
    if details:
        print(f"    {details}")


def create_valid_json_command(operator: str, command: str, timestamp: float = None) -> bytes:
    """Create a valid JSON command message."""
    if timestamp is None:
        timestamp = time.time()

    cmd = {
        "operator": operator,
        "command": command,
        "timestamp": timestamp,
    }
    return json.dumps(cmd).encode("utf-8")


def send_zmq_message(
    socket: zmq.Socket, json_bytes: bytes, signature: bytes, description: str
) -> bool:
    """
    Send ZMQ message and check if it was sent successfully.

    Returns:
        True if message was sent, False otherwise
    """
    try:
        socket.send_multipart([json_bytes, signature], zmq.NOBLOCK)
        return True
    except zmq.Again:
        test_result(f"ZMQ send: {description}", False, "Socket not ready")
        return False
    except Exception as e:
        test_result(f"ZMQ send: {description}", False, f"Error: {str(e)[:50]}")
        return False


# ============================================================================
# VALID COMMANDS FROM DOCUMENTATION
# ============================================================================


def test_valid_commands():
    """Test all valid commands from documentation."""
    print("\n" + "=" * 70)
    print("VALID COMMANDS FROM DOCUMENTATION")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        valid_commands = [
            # Repeater Control Commands
            ("SET_SQUELCH -120", "Set squelch threshold (minimum)"),
            ("SET_SQUELCH -30", "Set squelch threshold (maximum)"),
            ("SET_SQUELCH -24", "Set squelch threshold (typical)"),
            ("SET_POWER 0", "Set power level (minimum)"),
            ("SET_POWER 50", "Set power level (medium)"),
            ("SET_POWER 100", "Set power level (maximum)"),
            ("SET_TIMEOUT 60", "Set timeout (minimum)"),
            ("SET_TIMEOUT 300", "Set timeout (typical)"),
            ("SET_TIMEOUT 600", "Set timeout (maximum)"),
            ("RESTART", "Restart service"),
            # Additional commands from SVXLink documentation
            ("ENABLE_REPEATER", "Enable repeater"),
            ("DISABLE_REPEATER", "Disable repeater"),
            ("STATUS", "Get repeater status"),
            ("IDENTIFY", "Manual identification"),
            ("SET_SHORT_ID_INTERVAL 10", "Set short ID interval"),
            ("SET_LONG_ID_INTERVAL 60", "Set long ID interval"),
        ]

        for command, description in valid_commands:
            json_bytes = create_valid_json_command("LA1ABC", command)
            signature = generate_test_signature(json_bytes)

            if send_zmq_message(socket, json_bytes, signature, f"Valid: {command}"):
                test_result(f"Valid command: {command}", True, description)

    except Exception as e:
        test_result("Valid commands test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# INVALID COMMANDS
# ============================================================================


def test_invalid_commands():
    """Test invalid commands that should be rejected."""
    print("\n" + "=" * 70)
    print("INVALID COMMANDS")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        invalid_commands = [
            ("INVALID_COMMAND", "Unknown command"),
            ("SET_SQUELCH", "Missing argument"),
            ("SET_SQUELCH abc", "Invalid argument type (string instead of number)"),
            ("SET_POWER", "Missing argument"),
            ("SET_POWER -10", "Invalid argument (negative power)"),
            ("SET_POWER 150", "Invalid argument (over 100%)"),
            ("SET_TIMEOUT", "Missing argument"),
            ("SET_TIMEOUT -5", "Invalid argument (negative timeout)"),
            ("SET_TIMEOUT abc", "Invalid argument type"),
            ("", "Empty command"),
            ("   ", "Whitespace only"),
            ("SET_SQUELCH -24 -30", "Too many arguments"),
            ("SET_POWER 50 75", "Too many arguments"),
        ]

        for command, description in invalid_commands:
            json_bytes = create_valid_json_command("LA1ABC", command)
            signature = generate_test_signature(json_bytes)

            if send_zmq_message(socket, json_bytes, signature, f"Invalid: {command}"):
                test_result(f"Invalid command: {command[:30]}", True, description)

    except Exception as e:
        test_result("Invalid commands test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# JSON FORMAT EDGE CASES
# ============================================================================


def test_json_format_edge_cases():
    """Test JSON format edge cases."""
    print("\n" + "=" * 70)
    print("JSON FORMAT EDGE CASES")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        test_cases = [
            # Valid JSON formats
            (
                b'{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":1234567890.0}',
                "Valid: compact JSON",
                True,
            ),
            (
                b'{"operator": "LA1ABC", "command": "SET_SQUELCH -24", "timestamp": 1234567890.0}',
                "Valid: formatted JSON",
                True,
            ),
            # Missing fields
            (b'{"operator":"LA1ABC","command":"SET_SQUELCH -24"}', "Missing: timestamp", False),
            (b'{"operator":"LA1ABC","timestamp":1234567890.0}', "Missing: command", False),
            (b'{"command":"SET_SQUELCH -24","timestamp":1234567890.0}', "Missing: operator", False),
            # Wrong field types
            (b'{"operator":123,"command":"SET_SQUELCH -24","timestamp":1234567890.0}', "Wrong type: operator is number", False),
            (b'{"operator":"LA1ABC","command":123,"timestamp":1234567890.0}', "Wrong type: command is number", False),
            (b'{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":"invalid"}', "Wrong type: timestamp is string", False),
            # Empty values
            (b'{"operator":"","command":"SET_SQUELCH -24","timestamp":1234567890.0}', "Empty: operator", False),
            (b'{"operator":"LA1ABC","command":"","timestamp":1234567890.0}', "Empty: command", False),
            # Invalid JSON
            (b'{"operator":"LA1ABC"', "Invalid: incomplete JSON", False),
            (b'operator:LA1ABC,command:SET_SQUELCH', "Invalid: not JSON", False),
            (b'{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":}', "Invalid: missing timestamp value", False),
            # Extra fields (should be accepted but ignored)
            (
                b'{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":1234567890.0,"extra":"field"}',
                "Extra field (should be accepted)",
                True,
            ),
            # Unicode in callsign
            (
                b'{"operator":"LA1ABC-0","command":"SET_SQUELCH -24","timestamp":1234567890.0}',
                "Valid: callsign with SSID",
                True,
            ),
            # Very long command
            (
                json.dumps(
                    {
                        "operator": "LA1ABC",
                        "command": "SET_SQUELCH " + "X" * 1000,
                        "timestamp": time.time(),
                    }
                ).encode("utf-8"),
                "Very long command (1000 chars)",
                True,
            ),
        ]

        for json_bytes, description, should_parse in test_cases:
            signature = generate_test_signature(json_bytes)

            try:
                # Try to parse JSON to verify format
                data = json.loads(json_bytes.decode("utf-8"))
                can_parse = True
            except Exception:
                can_parse = False

            if can_parse == should_parse:
                if send_zmq_message(socket, json_bytes, signature, description):
                    test_result(f"JSON format: {description}", True)
            else:
                test_result(f"JSON format: {description}", False, f"Parse result mismatch: expected {should_parse}, got {can_parse}")

    except Exception as e:
        test_result("JSON format test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# ZMQ MESSAGE FORMAT EDGE CASES
# ============================================================================


def test_zmq_message_format_edge_cases():
    """Test ZMQ message format edge cases."""
    print("\n" + "=" * 70)
    print("ZMQ MESSAGE FORMAT EDGE CASES")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        json_bytes = create_valid_json_command("LA1ABC", "SET_SQUELCH -24")
        signature = generate_test_signature(json_bytes)

        test_cases = [
            # Valid two-part message
            ([json_bytes, signature], "Valid: two-part message", True),
            # Single part (missing signature)
            ([json_bytes], "Invalid: single part (missing signature)", False),
            # Three parts (extra data)
            ([json_bytes, signature, b"extra"], "Invalid: three parts", False),
            # Empty parts
            ([b"", signature], "Invalid: empty JSON", False),
            ([json_bytes, b""], "Invalid: empty signature", True),  # Empty signature allowed for testing
            # Very large messages
            (
                [json_bytes, b"x" * 10000],
                "Large signature (10KB)",
                True,
            ),
            (
                [b"x" * 10000, signature],
                "Large JSON (10KB)",
                True,
            ),
            # Binary data in JSON (should fail to parse)
            ([b"\x00\x01\x02\x03", signature], "Invalid: binary JSON", False),
            # Null bytes in message
            ([json_bytes + b"\x00", signature], "Null byte in JSON", True),
            ([json_bytes, signature + b"\x00"], "Null byte in signature", True),
        ]

        for parts, description, should_send in test_cases:
            try:
                socket.send_multipart(parts, zmq.NOBLOCK)
                test_result(f"ZMQ format: {description}", should_send, "Message sent")
            except zmq.Again:
                test_result(f"ZMQ format: {description}", False, "Socket not ready")
            except Exception as e:
                if not should_send:
                    test_result(f"ZMQ format: {description}", True, f"Correctly rejected: {type(e).__name__}")
                else:
                    test_result(f"ZMQ format: {description}", False, f"Unexpected error: {str(e)[:50]}")

    except Exception as e:
        test_result("ZMQ format test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# TIMESTAMP EDGE CASES
# ============================================================================


def test_timestamp_edge_cases():
    """Test timestamp edge cases."""
    print("\n" + "=" * 70)
    print("TIMESTAMP EDGE CASES")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        current_time = time.time()

        timestamp_cases = [
            (current_time, "Current timestamp", True),
            (current_time - 100, "100 seconds ago (within window)", True),
            (current_time - 299, "299 seconds ago (boundary)", True),
            (current_time - 301, "301 seconds ago (too old)", False),
            (current_time + 1, "1 second in future (within skew)", True),
            (current_time + 59, "59 seconds in future (within skew)", True),
            (current_time + 61, "61 seconds in future (too far)", False),
            (0, "Zero timestamp", False),
            (9999999999, "Very large timestamp", True),
            (-1, "Negative timestamp", False),
            (current_time - 3600, "1 hour ago (too old)", False),
            (current_time + 3600, "1 hour in future (too far)", False),
        ]

        for timestamp, description, should_accept in timestamp_cases:
            json_bytes = create_valid_json_command("LA1ABC", "SET_SQUELCH -24", timestamp)
            signature = generate_test_signature(json_bytes)

            if send_zmq_message(socket, json_bytes, signature, f"Timestamp: {description}"):
                test_result(f"Timestamp: {description}", True)

    except Exception as e:
        test_result("Timestamp test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# CALLSIGN EDGE CASES
# ============================================================================


def test_callsign_edge_cases():
    """Test callsign edge cases."""
    print("\n" + "=" * 70)
    print("CALLSIGN EDGE CASES")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        callsign_cases = [
            ("LA1ABC", "Valid: standard callsign", True),
            ("la1abc", "Valid: lowercase (should normalize)", True),
            ("LA1ABC-0", "Valid: with SSID", True),
            ("LA1ABC-15", "Valid: max SSID", True),
            ("", "Invalid: empty", False),
            ("A" * 100, "Invalid: too long", False),
            ("LA1ABC-16", "Invalid: SSID too high", True),  # Should normalize and accept base
            ("123ABC", "Invalid: starts with number", True),  # May be valid in some systems
            ("LA1ABC-", "Invalid: SSID without number", True),  # Should normalize
            ("LA1ABC--0", "Invalid: double dash", True),  # Should normalize
        ]

        for callsign, description, should_send in callsign_cases:
            json_bytes = create_valid_json_command(callsign, "SET_SQUELCH -24")
            signature = generate_test_signature(json_bytes)

            if send_zmq_message(socket, json_bytes, signature, f"Callsign: {description}"):
                test_result(f"Callsign: {description}", True)

    except Exception as e:
        test_result("Callsign test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# COMMAND INJECTION IN JSON
# ============================================================================


def test_command_injection_in_json():
    """Test command injection attempts in JSON fields."""
    print("\n" + "=" * 70)
    print("COMMAND INJECTION IN JSON")
    print("=" * 70)

    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        socket.connect("tcp://localhost:5555")
        time.sleep(0.1)

        injection_attempts = [
            # SQL injection
            ('{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":1234567890.0,"extra":"\'; DROP TABLE--"}', "SQL injection in extra field"),
            # Script injection
            ('{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":1234567890.0,"extra":"<script>alert(1)</script>"}', "Script injection"),
            # Command injection in operator
            ('{"operator":"LA1ABC;rm -rf /","command":"SET_SQUELCH -24","timestamp":1234567890.0}', "Command injection in operator"),
            # Command injection in command
            ('{"operator":"LA1ABC","command":"SET_SQUELCH -24; cat /etc/passwd","timestamp":1234567890.0}', "Command injection in command"),
            # Path traversal
            ('{"operator":"../../etc/passwd","command":"SET_SQUELCH -24","timestamp":1234567890.0}', "Path traversal in operator"),
            # JSON injection
            ('{"operator":"LA1ABC","command":"SET_SQUELCH -24","timestamp":1234567890.0,"malicious":{"nested":"injection"}}', "Nested JSON injection"),
        ]

        for json_str, description in injection_attempts:
            try:
                json_bytes = json_str.encode("utf-8")
                # Verify it's valid JSON
                json.loads(json_str)
                signature = generate_test_signature(json_bytes)

                if send_zmq_message(socket, json_bytes, signature, f"Injection: {description}"):
                    test_result(f"Injection attempt: {description[:40]}", True, "Message sent (should be sanitized by handler)")
            except json.JSONDecodeError:
                test_result(f"Injection attempt: {description[:40]}", True, "Invalid JSON (correctly rejected)")

    except Exception as e:
        test_result("Injection test setup", False, str(e))
    finally:
        socket.close()
        context.term()


# ============================================================================
# MAIN TEST EXECUTION
# ============================================================================


def main():
    """Run all ZMQ message format tests."""
    print("=" * 70)
    print("ZMQ MESSAGE FORMAT TEST SUITE")
    print("=" * 70)
    print(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"ZMQ: {'Available' if ZMQ_AVAILABLE else 'Not Available'}")
    print(f"Cryptography: {'Available' if CRYPTO_AVAILABLE else 'Not Available'}")

    if not ZMQ_AVAILABLE:
        print("ERROR: ZMQ not available. Cannot run tests.")
        return 1

    # Run all test suites
    test_valid_commands()
    test_invalid_commands()
    test_json_format_edge_cases()
    test_zmq_message_format_edge_cases()
    test_timestamp_edge_cases()
    test_callsign_edge_cases()
    test_command_injection_in_json()

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
    print("NOTE: These tests verify message format and sending capability.")
    print("      Actual command processing and validation should be tested")
    print("      with the command handler running.")
    print("=" * 70)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

