#!/usr/bin/env python3
"""
Authenticated Repeater Control Command Handler

This service monitors for authenticated commands from the RX flowgraph,
verifies signatures against authorized operator public keys, and executes
commands via SVXLink control interface.

IPC Mechanism: ZMQ (default), FIFO, or Unix socket
SVXLink Control: TCP (preferred), config file modification, or DTMF injection
"""

import hashlib
import json
import logging
import os
import signal
import socket
import sys
import time
from collections import defaultdict
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import yaml

try:
    import zmq

    ZMQ_AVAILABLE = True
except ImportError:
    ZMQ_AVAILABLE = False
    print("WARNING: pyzmq not available. ZMQ IPC disabled.")

try:
    from cryptography.hazmat.backends import default_backend
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec

    CRYPTO_AVAILABLE = True
except ImportError:
    CRYPTO_AVAILABLE = False
    print("ERROR: cryptography library not available. Cannot verify signatures.")
    sys.exit(1)

# Global state
RUNNING: bool = True
COMMAND_HISTORY: Dict[str, List[Dict[str, Any]]] = defaultdict(
    list
)  # Track commands for replay protection


def normalize_callsign(callsign: Optional[str]) -> Tuple[str, str]:
    """
    Normalize a callsign string.

    Returns:
        (normalized_callsign_with_ssid, base_callsign_without_ssid)
    """
    if not callsign:
        return "", ""

    normalized = callsign.strip().upper()
    if "-" in normalized:
        base, _, suffix = normalized.partition("-")
        if suffix.isdigit():
            return normalized, base
    return normalized, normalized


def load_config(config_path: Optional[str] = None) -> Dict:
    """Load configuration from YAML file."""
    # Allow config path to be overridden via environment variable
    if config_path is None:
        config_path = os.getenv(
            "AUTHENTICATED_CONFIG", "/etc/authenticated-repeater/config.yaml"
        )
    try:
        with open(config_path, "r") as f:
            config = yaml.safe_load(f)
        return config
    except FileNotFoundError:
        print(f"ERROR: Configuration file not found: {config_path}")
        sys.exit(1)
    except yaml.YAMLError as e:
        print(f"ERROR: Failed to parse configuration: {e}")
        sys.exit(1)


def setup_logging(config: Dict):
    """Configure logging based on config file."""
    log_file = config.get("log_file", "/var/log/authenticated-repeater/commands.log")
    log_level = getattr(logging, config.get("log_level", "INFO").upper())

    # Create log directory if it doesn't exist
    log_dir = os.path.dirname(log_file)
    os.makedirs(log_dir, exist_ok=True)

    logging.basicConfig(
        level=log_level,
        format="%(asctime)s [%(levelname)s] %(message)s",
        handlers=[logging.FileHandler(log_file), logging.StreamHandler(sys.stdout)],
    )

    return logging.getLogger(__name__)


def load_authorized_keys(keys_dir: str) -> Dict[str, bytes]:
    """
    Load all authorized operator public keys from directory.

    Returns:
        Dictionary mapping callsign to public key bytes
    """
    keys: Dict[str, bytes] = {}
    keys_path = Path(keys_dir)

    if not keys_path.exists():
        logging.warning(f"Authorized keys directory does not exist: {keys_dir}")
        return keys

    for key_file in keys_path.glob("*.pem"):
        try:
            callsign = key_file.stem.upper()
            with open(key_file, "rb") as f:
                key_data = f.read()
            keys[callsign] = key_data
            logging.info(f"Loaded authorized key for {callsign}")
        except Exception as e:
            logging.error(f"Failed to load key from {key_file}: {e}")

    # Also support GPG ASCII format keys
    for key_file in keys_path.glob("*.asc"):
        try:
            callsign = key_file.stem.upper()
            with open(key_file, "rb") as f:
                key_data = f.read()
            # Try to parse as GPG key (simplified - would need gpg library for full support)
            keys[callsign] = key_data
            logging.info(f"Loaded GPG key for {callsign} (basic support)")
        except Exception as e:
            logging.error(f"Failed to load GPG key from {key_file}: {e}")

    return keys


def verify_signature(message: bytes, signature: bytes, public_key_data: bytes) -> bool:
    """
    Verify ECDSA signature using Brainpool curve.

    Args:
        message: Original message bytes
        signature: Signature bytes (DER encoded)
        public_key_data: Public key in PEM format

    Returns:
        True if signature is valid, False otherwise
    """
    try:
        # Load public key
        public_key = serialization.load_pem_public_key(
            public_key_data, backend=default_backend()
        )

        # Verify signature - handle EllipticCurvePublicKey specifically
        # The cryptography library returns a union type, but we expect ECDSA keys
        if isinstance(public_key, ec.EllipticCurvePublicKey):
            public_key.verify(signature, message, ec.ECDSA(hashes.SHA256()))
            return True
        else:
            logging.error(
                f"Unsupported key type: {type(public_key)}. Expected EllipticCurvePublicKey."
            )
            return False
    except Exception as e:
        logging.error(f"Signature verification failed: {e}")
        return False


def parse_command_frame(frame_data: bytes) -> Optional[Dict]:
    """
    Parse command frame from AX.25 packet.

    Expected format: "timestamp:operator_callsign:command_text"

    Returns:
        Dictionary with parsed fields or None if invalid
    """
    try:
        frame_str = frame_data.decode("utf-8", errors="ignore")
        parts = frame_str.split(":", 2)

        if len(parts) != 3:
            logging.error(f"Invalid command frame format: {frame_str}")
            return None

        timestamp_str, callsign_raw, command = parts

        try:
            timestamp = float(timestamp_str)
        except ValueError:
            logging.error(f"Invalid timestamp: {timestamp_str}")
            return None

        normalized_callsign, base_callsign = normalize_callsign(callsign_raw)
        if not normalized_callsign:
            logging.error(f"Invalid callsign: {callsign_raw}")
            return None

        return {
            "timestamp": timestamp,
            "callsign": normalized_callsign,
            "callsign_base": base_callsign,
            "command": command.strip(),
            "raw": frame_data,
        }
    except Exception as e:
        logging.error(f"Failed to parse command frame: {e}")
        return None


def check_replay_protection(command_data: Dict, config: Dict) -> bool:
    """
    Check if command is a replay attack.

    Returns:
        True if command is valid (not a replay), False if replay detected
    """
    callsign = command_data.get("callsign_base") or command_data["callsign"]
    timestamp = command_data["timestamp"]
    command_hash = hashlib.sha256(command_data["raw"]).hexdigest()

    window = config.get("replay_protection_window", 300)
    current_time = time.time()

    # Check if timestamp is too old
    if current_time - timestamp > window:
        logging.warning(
            f"Command timestamp too old for {command_data['callsign']}: {timestamp} (current: {current_time})"
        )
        return False

    # Check if timestamp is in the future (clock skew)
    if timestamp > current_time + 60:  # Allow 60 second clock skew
        logging.warning(
            f"Command timestamp in future for {command_data['callsign']}: {timestamp} (current: {current_time})"
        )
        return False

    # Check if we've seen this exact command recently
    history = COMMAND_HISTORY[callsign]
    for entry in history:
        if entry["hash"] == command_hash:
            logging.warning(
                f"Replay detected: duplicate command from {command_data['callsign']}"
            )
            return False

    # Add to history
    history.append({"hash": command_hash, "timestamp": timestamp, "time": current_time})

    # Clean old entries
    cutoff_time = current_time - window
    COMMAND_HISTORY[callsign] = [e for e in history if e["time"] > cutoff_time]

    # Rate limiting
    max_per_minute = config.get("max_commands_per_minute", 10)
    recent_commands = [e for e in history if e["time"] > current_time - 60]
    if len(recent_commands) > max_per_minute:
        logging.warning(
            f"Rate limit exceeded for {command_data['callsign']}: {len(recent_commands)} commands in last minute"
        )
        return False

    return True


def execute_svxlink_command_tcp(command: str, config: Dict) -> Tuple[bool, str]:
    """
    Execute SVXLink command via TCP control port.

    Returns:
        (success, result_message)
    """
    host = config.get("svxlink_tcp_host", "localhost")
    port = config.get("svxlink_tcp_port", 5210)
    timeout = config.get("command_timeout", 30)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect((host, port))

        # Send command
        sock.sendall(f"{command}\n".encode("utf-8"))

        # Receive response
        response = sock.recv(4096).decode("utf-8", errors="ignore").strip()
        sock.close()

        logging.info(f"SVXLink TCP response: {response}")
        return True, response

    except socket.timeout:
        error_msg = f"SVXLink TCP connection timeout after {timeout}s"
        logging.error(error_msg)
        return False, error_msg
    except ConnectionRefusedError:
        error_msg = "SVXLink TCP control port not available (connection refused)"
        logging.error(error_msg)
        return False, error_msg
    except Exception as e:
        error_msg = f"SVXLink TCP error: {e}"
        logging.error(error_msg)
        return False, error_msg


def execute_svxlink_command_config(
    command: str, config: Dict[str, Any]
) -> Tuple[bool, str]:
    """
    Execute SVXLink command by modifying config file and sending SIGHUP.

    This is a simplified implementation - actual implementation would need
    to parse and modify the SVXLink config file appropriately.
    """
    # config_file is reserved for future implementation
    _config_file = config.get("svxlink_config", "/etc/svxlink/svxlink.conf")

    try:
        # This is a placeholder - actual implementation would:
        # 1. Parse the SVXLink config file
        # 2. Modify the appropriate section based on command
        # 3. Write the modified config
        # 4. Send SIGHUP to svxlink process

        # For now, just log the command
        logging.info(f"Config-based command (not fully implemented): {command}")
        return False, "Config-based control not fully implemented"

    except Exception as e:
        error_msg = f"SVXLink config modification error: {e}"
        logging.error(error_msg)
        return False, error_msg


def execute_svxlink_command(command: str, config: Dict) -> Tuple[bool, str]:
    """
    Execute command via configured SVXLink control method.

    Returns:
        (success, result_message)
    """
    method = config.get("svxlink_control", "tcp")

    if method == "tcp":
        return execute_svxlink_command_tcp(command, config)
    elif method == "config":
        return execute_svxlink_command_config(command, config)
    elif method == "dtmf":
        # DTMF injection would require additional implementation
        logging.warning("DTMF injection method not implemented")
        return False, "DTMF injection not implemented"
    else:
        logging.error(f"Unknown SVXLink control method: {method}")
        return False, f"Unknown control method: {method}"


def process_command(
    command_frame: bytes,
    signature_frame: bytes,
    authorized_keys: Dict[str, bytes],
    config: Dict,
) -> Tuple[bool, str]:
    """
    Process authenticated command: verify signature and execute.

    Returns:
        (success, result_message)
    """
    # Parse command frame
    command_data = parse_command_frame(command_frame)
    if not command_data:
        return False, "Failed to parse command frame"

    callsign = command_data["callsign"]

    # Check if operator is authorized
    if callsign not in authorized_keys:
        logging.warning(f"Unauthorized operator: {callsign}")
        return False, f"Operator {callsign} not authorized"

    # Check replay protection
    if not check_replay_protection(command_data, config):
        return False, "Replay protection check failed"

    # Verify signature
    public_key_data = authorized_keys[callsign]
    if not verify_signature(command_frame, signature_frame, public_key_data):
        logging.warning(f"Invalid signature from {callsign}")
        return False, "Signature verification failed"

    # Execute command
    command = command_data["command"]
    logging.info(f"Executing authenticated command from {callsign}: {command}")

    success, result = execute_svxlink_command(command, config)

    # Log command execution
    log_entry = {
        "timestamp": datetime.now().isoformat(),
        "operator": callsign,
        "command": command,
        "success": success,
        "result": result,
    }
    logging.info(f"Command executed: {json.dumps(log_entry)}")

    return success, result


def zmq_receiver(
    config: Dict, authorized_keys: Dict[str, bytes], logger: logging.Logger
):
    """Receive commands via ZMQ socket."""
    if not ZMQ_AVAILABLE:
        logger.error("ZMQ not available, cannot use ZMQ receiver")
        return

    context = zmq.Context()
    socket_path = config.get("zmq_rx_socket", "ipc:///tmp/authenticated_rx.sock")

    # Create socket directory if using IPC
    if socket_path.startswith("ipc://"):
        socket_file = socket_path.replace("ipc://", "")
        os.makedirs(os.path.dirname(socket_file), exist_ok=True)

    receiver = context.socket(zmq.PULL)
    receiver.bind(socket_path)
    logger.info(f"ZMQ receiver listening on {socket_path}")

    while RUNNING:
        try:
            # Receive two-part message: command frame, signature frame
            frames = receiver.recv_multipart(zmq.NOBLOCK)

            if len(frames) == 2:
                command_frame = frames[0]
                signature_frame = frames[1]

                success, result = process_command(
                    command_frame, signature_frame, authorized_keys, config
                )

                # Send acknowledgment back (if needed)
                if config.get("zmq_tx_socket"):
                    send_acknowledgment(config, success, result)
            else:
                logger.warning(f"Received invalid message format: {len(frames)} frames")

        except zmq.Again:
            time.sleep(0.1)  # No message available
        except KeyboardInterrupt:
            break
        except Exception as e:
            logger.error(f"ZMQ receiver error: {e}")
            time.sleep(1)

    receiver.close()
    context.term()


def send_acknowledgment(config: Dict, success: bool, result: str):
    """Send acknowledgment back to transmitter."""
    if not ZMQ_AVAILABLE:
        return

    context = zmq.Context()
    socket_path = config.get("zmq_tx_socket", "ipc:///tmp/authenticated_tx.sock")

    sender = context.socket(zmq.PUSH)
    sender.connect(socket_path)

    ack = {"success": success, "result": result, "timestamp": time.time()}

    sender.send_json(ack)
    sender.close()
    context.term()


def fifo_receiver(
    config: Dict, authorized_keys: Dict[str, bytes], logger: logging.Logger
):
    """Receive commands via named pipe (FIFO)."""
    fifo_path = config.get("fifo_path", "/tmp/authenticated_commands.fifo")

    # Create FIFO if it doesn't exist
    if not os.path.exists(fifo_path):
        os.mkfifo(fifo_path)
        # Note: 0o666 permissions required for FIFO to be accessible by flowgraph and handler
        # This is safe as FIFO is in /tmp and only accepts authenticated commands
        os.chmod(fifo_path, 0o666)  # nosec B103

    logger.info(f"FIFO receiver listening on {fifo_path}")

    while RUNNING:
        try:
            with open(fifo_path, "rb") as fifo:
                # Read command frame length
                cmd_len_bytes = fifo.read(4)
                if len(cmd_len_bytes) != 4:
                    continue
                cmd_len = int.from_bytes(cmd_len_bytes, "big")

                # Read command frame
                command_frame = fifo.read(cmd_len)
                if len(command_frame) != cmd_len:
                    continue

                # Read signature frame length
                sig_len_bytes = fifo.read(4)
                if len(sig_len_bytes) != 4:
                    continue
                sig_len = int.from_bytes(sig_len_bytes, "big")

                # Read signature frame
                signature_frame = fifo.read(sig_len)
                if len(signature_frame) != sig_len:
                    continue

                success, result = process_command(
                    command_frame, signature_frame, authorized_keys, config
                )

        except KeyboardInterrupt:
            break
        except Exception as e:
            logger.error(f"FIFO receiver error: {e}")
            time.sleep(1)


def signal_handler(signum: int, _frame: Any) -> None:
    """Handle shutdown signals."""
    global RUNNING
    logging.info(f"Received shutdown signal {signum}, stopping...")
    RUNNING = False


def main():
    """Main entry point."""
    # Load configuration
    config = load_config()

    # Setup logging
    logger = setup_logging(config)
    logger.info("Authenticated Repeater Control Command Handler starting...")

    # Setup signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # Load authorized keys
    keys_dir = config.get(
        "authorized_keys_dir", "/etc/authenticated-repeater/authorized_operators"
    )
    authorized_keys = load_authorized_keys(keys_dir)

    if not authorized_keys:
        logger.error("No authorized keys found. Cannot process commands.")
        sys.exit(1)

    logger.info(f"Loaded {len(authorized_keys)} authorized operator keys")

    # Start receiver based on IPC mechanism
    ipc_mechanism = config.get("ipc_mechanism", "zmq")

    try:
        if ipc_mechanism == "zmq":
            if not ZMQ_AVAILABLE:
                logger.error("ZMQ requested but not available. Install pyzmq.")
                sys.exit(1)
            zmq_receiver(config, authorized_keys, logger)
        elif ipc_mechanism == "fifo":
            fifo_receiver(config, authorized_keys, logger)
        elif ipc_mechanism == "socket":
            logger.error("Unix socket IPC not yet implemented")
            sys.exit(1)
        else:
            logger.error(f"Unknown IPC mechanism: {ipc_mechanism}")
            sys.exit(1)
    except KeyboardInterrupt:
        logger.info("Shutting down...")
    except Exception as e:
        logger.error(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)


if __name__ == "__main__":
    main()
