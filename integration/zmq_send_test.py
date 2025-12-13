#!/usr/bin/env python3
"""
Test script for ZMQ command handler.

Sends test commands to the command handler via ZMQ port 5555.
"""

import json
import sys
import time

try:
    import zmq
except ImportError:
    print("ERROR: pyzmq not available. Install with: pip3 install pyzmq")
    sys.exit(1)


def send_test_command(
    command: str,
    operator: str = "LA1ABC",
    zmq_host: str = "localhost",
    zmq_port: int = 5555,
    include_signature: bool = True,
):
    """
    Send a test command to the command handler.

    Args:
        command: Command string (e.g., "SET_SQUELCH -24")
        operator: Operator callsign
        zmq_host: ZMQ host
        zmq_port: ZMQ port
        include_signature: Whether to include a dummy signature
    """
    context = zmq.Context()
    socket = context.socket(zmq.PUB)

    try:
        # Connect to command handler
        endpoint = f"tcp://{zmq_host}:{zmq_port}"
        socket.connect(endpoint)
        time.sleep(0.1)  # Give connection time to establish

        # Create JSON command
        json_command = {
            "operator": operator,
            "command": command,
            "timestamp": time.time(),
        }

        json_bytes = json.dumps(json_command).encode("utf-8")

        # Create dummy signature (for testing without real signature)
        if include_signature:
            signature = b"dummy_signature_for_testing_" + b"x" * 40
        else:
            signature = b""

        # Send two-part message
        socket.send_multipart([json_bytes, signature])

        print(f"Sent command: {command} from {operator}")
        print(f"JSON: {json.dumps(json_command, indent=2)}")
        print(f"Endpoint: {endpoint}")

    except Exception as e:
        print(f"Error sending command: {e}")
        sys.exit(1)

    finally:
        socket.close()
        context.term()


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        print("Usage: zmq_send_test.py <command> [operator] [host] [port]")
        print("")
        print("Examples:")
        print("  zmq_send_test.py 'SET_SQUELCH -24'")
        print("  zmq_send_test.py 'SET_POWER 50' LA1ABC")
        print("  zmq_send_test.py 'SET_TIMEOUT 300' LA1ABC localhost 5555")
        print("  zmq_send_test.py 'RESTART'")
        sys.exit(1)

    command = sys.argv[1]
    operator = sys.argv[2] if len(sys.argv) > 2 else "LA1ABC"
    host = sys.argv[3] if len(sys.argv) > 3 else "localhost"
    port = int(sys.argv[4]) if len(sys.argv) > 4 else 5555

    send_test_command(command, operator, host, port)


if __name__ == "__main__":
    main()
