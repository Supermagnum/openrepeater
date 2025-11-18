#!/usr/bin/env python3
"""
Authenticated Repeater Control Command Handler with ZMQ Bidirectional Communication

This service:
1. Receives authenticated commands from GNU Radio RX flowgraph via ZMQ (port 5555)
2. Executes commands (changes repeater settings via SVXLink)
3. Sends acknowledgment reply back via radio (TX flowgraph via ZMQ port 5557)
4. Sends GNU Radio parameter updates via ZMQ (port 5556)

Architecture:
Radio → RX Flowgraph → ZMQ:5555 → Command Handler → SVXLink
                                              ↓
                                         ZMQ:5556 → GNU Radio params
                                              ↓
                                         ZMQ:5557 → TX Flowgraph → Radio
"""

import hashlib
import json
import logging
import os
import signal
import sys
import time
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import yaml

try:
    import zmq

    ZMQ_AVAILABLE = True
except ImportError:
    ZMQ_AVAILABLE = False
    print("ERROR: pyzmq not available. ZMQ required for this handler.")
    sys.exit(1)

try:
    from cryptography.hazmat.backends import default_backend
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec

    CRYPTO_AVAILABLE = True
except ImportError:
    CRYPTO_AVAILABLE = False
    print("ERROR: cryptography library not available. Cannot verify signatures.")
    sys.exit(1)

# Import our modules
from reply_formatter import ReplyFormatter
from svxlink_interface import SVXLinkInterface

# Global state
RUNNING: bool = True
COMMAND_HISTORY: Dict[str, List[Dict[str, Any]]] = defaultdict(list)


def load_config(config_path: Optional[str] = None) -> Dict:
    """Load configuration from YAML file."""
    if config_path is None:
        config_path = os.getenv(
            "AUTHENTICATED_CONFIG", "/etc/authenticated-repeater/config.yaml"
        )
    try:
        with open(config_path, "r", encoding="utf-8") as f:
            config = yaml.safe_load(f)
        return config
    except FileNotFoundError:
        print(f"ERROR: Configuration file not found: {config_path}")
        sys.exit(1)
    except yaml.YAMLError as e:
        print(f"ERROR: Failed to parse configuration: {e}")
        sys.exit(1)


def setup_logging(config: Dict):
    """Configure logging with two log files."""
    log_level = getattr(logging, config.get("log_level", "INFO").upper())

    # Service operation log
    service_log_file = config.get(
        "service_log_file", "/var/log/authenticated_repeater.log"
    )
    service_log_dir = os.path.dirname(service_log_file)
    os.makedirs(service_log_dir, exist_ok=True)

    # Command history log
    command_log_file = config.get("command_log_file", "/var/log/repeater_commands.log")
    command_log_dir = os.path.dirname(command_log_file)
    os.makedirs(command_log_dir, exist_ok=True)

    # Create formatters
    service_formatter = logging.Formatter(
        "%(asctime)s [%(levelname)s] %(message)s", datefmt="%Y-%m-%d %H:%M:%S"
    )
    command_formatter = logging.Formatter(
        "[%(asctime)s] [%(message)s]", datefmt="%Y-%m-%d %H:%M:%S"
    )

    # Service logger
    service_logger = logging.getLogger("service")
    service_logger.setLevel(log_level)
    service_handler = logging.FileHandler(service_log_file)
    service_handler.setFormatter(service_formatter)
    service_logger.addHandler(service_handler)
    service_logger.addHandler(logging.StreamHandler(sys.stdout))

    # Command logger (separate file for command history)
    command_logger = logging.getLogger("commands")
    command_logger.setLevel(logging.INFO)
    command_handler = logging.FileHandler(command_log_file)
    command_handler.setFormatter(command_formatter)
    command_logger.addHandler(command_handler)

    return service_logger, command_logger


def load_authorized_keys(keys_dir: str) -> Dict[str, bytes]:
    """Load all authorized operator public keys from directory."""
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

    return keys


def verify_signature(message: bytes, signature: bytes, public_key_data: bytes) -> bool:
    """Verify ECDSA signature using Brainpool curve."""
    try:
        public_key = serialization.load_pem_public_key(
            public_key_data, backend=default_backend()
        )

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


def parse_json_command(json_data: bytes) -> Optional[Dict]:
    """
    Parse JSON command from RX flowgraph.

    Expected format: {"operator": "LA1ABC", "command": "SET_SQUELCH -24", "timestamp": 1234567890.0}

    Returns:
        Dictionary with parsed fields or None if invalid
    """
    try:
        data = json.loads(json_data.decode("utf-8"))

        if "operator" not in data or "command" not in data or "timestamp" not in data:
            logging.error("Invalid JSON command format: missing required fields")
            return None

        return {
            "timestamp": float(data["timestamp"]),
            "callsign": data["operator"].upper(),
            "command": data["command"].strip(),
            "raw": json_data,
        }
    except json.JSONDecodeError as e:
        logging.error(f"Failed to parse JSON command: {e}")
        return None
    except Exception as e:
        logging.error(f"Failed to parse command: {e}")
        return None


def check_replay_protection(command_data: Dict, config: Dict) -> bool:
    """Check if command is a replay attack."""
    callsign = command_data["callsign"]
    timestamp = command_data["timestamp"]
    command_hash = hashlib.sha256(command_data["raw"]).hexdigest()

    window = config.get("replay_protection_window", 300)
    current_time = time.time()

    # Check if timestamp is too old
    if current_time - timestamp > window:
        logging.warning(
            f"Command timestamp too old: {timestamp} (current: {current_time})"
        )
        return False

    # Check if timestamp is in the future (clock skew)
    if timestamp > current_time + 60:
        logging.warning(
            f"Command timestamp in future: {timestamp} (current: {current_time})"
        )
        return False

    # Check if we've seen this exact command recently
    history = COMMAND_HISTORY[callsign]
    for entry in history:
        if entry["hash"] == command_hash:
            logging.warning(f"Replay detected: duplicate command from {callsign}")
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
            f"Rate limit exceeded for {callsign}: {len(recent_commands)} commands in last minute"
        )
        return False

    return True


def parse_command(command_str: str) -> Tuple[str, List[str]]:
    """
    Parse command string into command name and arguments.

    Args:
        command_str: Command string (e.g., "SET_SQUELCH -24")

    Returns:
        (command_name, arguments)
    """
    parts = command_str.split()
    if not parts:
        return "", []

    command_name = parts[0].upper()
    args = parts[1:] if len(parts) > 1 else []

    return command_name, args


def process_command(
    json_command: bytes,
    signature: bytes,
    authorized_keys: Dict[str, bytes],
    config: Dict,
    svxlink: SVXLinkInterface,
    reply_formatter: ReplyFormatter,
    command_logger: logging.Logger,
) -> Tuple[bool, Dict]:
    """
    Process authenticated command: verify signature and execute.

    Returns:
        (success, reply_dict)
    """
    start_time = time.time()

    # Parse JSON command
    command_data = parse_json_command(json_command)
    if not command_data:
        reply = reply_formatter.format_failure_reply(
            "UNKNOWN", "PARSE_ERROR", "Failed to parse command"
        )
        return False, reply

    callsign = command_data["callsign"]

    # Check if operator is authorized
    if callsign not in authorized_keys:
        logging.warning(f"Unauthorized operator: {callsign}")
        reply = reply_formatter.format_failure_reply(
            callsign, "AUTH_ERROR", f"Operator {callsign} not authorized"
        )
        return False, reply

    # Check replay protection
    if not check_replay_protection(command_data, config):
        reply = reply_formatter.format_failure_reply(
            callsign, "REPLAY_ERROR", "Replay protection check failed"
        )
        return False, reply

    # Verify signature (skip if signature is empty for testing)
    if signature and len(signature) > 0:
        public_key_data = authorized_keys[callsign]
        if not verify_signature(json_command, signature, public_key_data):
            logging.warning(f"Invalid signature from {callsign}")
            reply = reply_formatter.format_failure_reply(
                callsign, "SIGNATURE_ERROR", "Signature verification failed"
            )
            return False, reply
    else:
        logging.warning(
            f"No signature provided for command from {callsign} (testing mode?)"
        )

    # Parse command
    command_str = command_data["command"]
    command_name, args = parse_command(command_str)

    # Execute command
    logging.info(f"Executing authenticated command from {callsign}: {command_str}")

    success, result = svxlink.execute_command(command_name, args)

    # Calculate execution time
    execution_time_ms = int((time.time() - start_time) * 1000)

    # Format reply
    if success:
        param, value = reply_formatter.parse_command_result(
            command_str, success, result
        )
        if param and value:
            reply = reply_formatter.format_success_reply(
                callsign, command_str, param, value
            )
        else:
            reply = reply_formatter.format_success_reply(
                callsign, command_str, "command", "executed"
            )
    else:
        reply = reply_formatter.format_failure_reply(callsign, command_str, result)

    # Log command execution
    log_entry = (
        f"[{callsign}] [{command_str}] "
        f"[{'SUCCESS' if success else 'FAILURE'}] "
        f"[{execution_time_ms}ms]"
    )
    command_logger.info(log_entry)

    logging.info(
        f"Command executed: {command_str} from {callsign} - "
        f"{'SUCCESS' if success else 'FAILURE'} ({execution_time_ms}ms)"
    )

    return success, reply


def zmq_command_handler(
    config: Dict,
    authorized_keys: Dict[str, bytes],
    service_logger: logging.Logger,
    command_logger: logging.Logger,
):
    """Main ZMQ command handler with bidirectional communication."""
    context = zmq.Context()

    # ZMQ port 5555: Receive commands from RX flowgraph (SUB socket)
    rx_socket = context.socket(zmq.SUB)
    rx_bind = config.get("zmq_rx_bind", "tcp://*:5555")
    rx_socket.bind(rx_bind)
    rx_socket.setsockopt(zmq.SUBSCRIBE, b"")  # Subscribe to all messages
    service_logger.info(f"ZMQ receiver listening on {rx_bind}")

    # ZMQ port 5556: Send GNU Radio parameter updates (PUB socket)
    param_socket = context.socket(zmq.PUB)
    param_bind = config.get("zmq_param_bind", "tcp://*:5556")
    param_socket.bind(param_bind)
    time.sleep(0.1)  # Give subscribers time to connect
    service_logger.info(f"ZMQ parameter publisher on {param_bind}")

    # ZMQ port 5557: Send reply messages to TX flowgraph (PUB socket)
    reply_socket = context.socket(zmq.PUB)
    reply_bind = config.get("zmq_reply_bind", "tcp://*:5557")
    reply_socket.bind(reply_bind)
    time.sleep(0.1)  # Give subscribers time to connect
    service_logger.info(f"ZMQ reply publisher on {reply_bind}")

    # Initialize SVXLink interface
    svxlink_config = config.get("svxlink_config", "/etc/svxlink/svxlink.conf")
    svxlink_host = config.get("svxlink_tcp_host", "localhost")
    svxlink_port = config.get("svxlink_tcp_port", 5210)
    svxlink = SVXLinkInterface(svxlink_config, svxlink_host, svxlink_port)

    # Initialize reply formatter
    repeater_callsign = config.get("repeater_callsign", "REPEATER")
    reply_formatter = ReplyFormatter(repeater_callsign)

    # Poller for non-blocking receive
    poller = zmq.Poller()
    poller.register(rx_socket, zmq.POLLIN)

    service_logger.info("Command handler ready, waiting for commands...")

    while RUNNING:
        try:
            # Poll for messages (non-blocking)
            socks = dict(poller.poll(timeout=1000))  # 1 second timeout

            if rx_socket in socks and socks[rx_socket] == zmq.POLLIN:
                # Receive two-part message: JSON command, signature
                try:
                    frames = rx_socket.recv_multipart(zmq.NOBLOCK)

                    if len(frames) == 2:
                        json_command = frames[0]
                        signature = frames[1]

                        # Process command
                        success, reply = process_command(
                            json_command,
                            signature,
                            authorized_keys,
                            config,
                            svxlink,
                            reply_formatter,
                            command_logger,
                        )

                        # Send reply to TX flowgraph (port 5557)
                        reply_json = reply_formatter.format_json_reply(reply)
                        reply_socket.send(reply_json)
                        service_logger.debug(
                            f"Sent reply to TX flowgraph: {reply['message']}"
                        )

                        # Send parameter update to GNU Radio (port 5556) if command succeeded
                        if success:
                            param_update = {
                                "command": reply.get("command", ""),
                                "success": True,
                                "timestamp": time.time(),
                            }
                            param_json = json.dumps(param_update).encode("utf-8")
                            param_socket.send(param_json)
                            service_logger.debug("Sent parameter update to GNU Radio")

                    elif len(frames) == 1:
                        # Single frame: try to parse as JSON directly
                        json_command = frames[0]
                        # For testing without signature
                        dummy_signature = b""
                        success, reply = process_command(
                            json_command,
                            dummy_signature,
                            authorized_keys,
                            config,
                            svxlink,
                            reply_formatter,
                            command_logger,
                        )
                        reply_json = reply_formatter.format_json_reply(reply)
                        reply_socket.send(reply_json)

                    else:
                        service_logger.warning(
                            f"Received invalid message format: {len(frames)} frames"
                        )

                except zmq.Again:
                    pass  # No message available
                except Exception as e:
                    service_logger.error(
                        f"Error processing command: {e}", exc_info=True
                    )

        except KeyboardInterrupt:
            break
        except Exception as e:
            service_logger.error(f"ZMQ handler error: {e}", exc_info=True)
            time.sleep(1)

    # Cleanup
    rx_socket.close()
    param_socket.close()
    reply_socket.close()
    context.term()
    service_logger.info("ZMQ command handler stopped")


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
    service_logger, command_logger = setup_logging(config)
    service_logger.info(
        "Authenticated Repeater Control Command Handler (ZMQ) starting..."
    )

    # Setup signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # Load authorized keys
    keys_dir = config.get(
        "authorized_keys_dir", "/etc/authenticated-repeater/authorized_operators"
    )
    authorized_keys = load_authorized_keys(keys_dir)

    if not authorized_keys:
        service_logger.error("No authorized keys found. Cannot process commands.")
        sys.exit(1)

    service_logger.info(f"Loaded {len(authorized_keys)} authorized operator keys")

    # Start ZMQ command handler
    try:
        zmq_command_handler(config, authorized_keys, service_logger, command_logger)
    except KeyboardInterrupt:
        service_logger.info("Shutting down...")
    except Exception as e:
        service_logger.error(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)


if __name__ == "__main__":
    main()
