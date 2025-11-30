#!/usr/bin/env python3
"""
Reply Formatter Module

Formats acknowledgment messages for transmission via AX.25 frames.
"""

import json
import logging
import re
import time
from typing import Dict, Optional, Tuple

logger = logging.getLogger(__name__)


class ReplyFormatter:
    """Formats reply messages for AX.25 transmission."""

    def __init__(self, repeater_callsign: str = "REPEATER"):
        """
        Initialize reply formatter.

        Args:
            repeater_callsign: Callsign of the repeater (for source address)
        """
        self.repeater_callsign = repeater_callsign.upper()

    def format_success_reply(
        self, destination: str, command: str, parameter: str, value: str
    ) -> Dict:
        """
        Format success reply message.

        Args:
            destination: Destination callsign
            command: Command that was executed
            parameter: Parameter that was changed
            value: New value

        Returns:
            Dictionary with formatted reply
        """
        message = f"Command successful, {parameter} set at {value}"

        return {
            "destination": destination.upper(),
            "source": self.repeater_callsign,
            "message": message,
            "timestamp": time.time(),
            "command": command,
            "status": "success",
        }

    def format_failure_reply(
        self, destination: str, command: str, error_message: str
    ) -> Dict:
        """
        Format failure reply message.

        Args:
            destination: Destination callsign
            command: Command that failed
            error_message: Error description

        Returns:
            Dictionary with formatted reply
        """
        message = f"Command failed: {error_message}"

        return {
            "destination": destination.upper(),
            "source": self.repeater_callsign,
            "message": message,
            "timestamp": time.time(),
            "command": command,
            "status": "failure",
            "error": error_message,
        }

    def format_unknown_command_reply(self, destination: str, command: str) -> Dict:
        """
        Format unknown command reply.

        Args:
            destination: Destination callsign
            command: Unknown command

        Returns:
            Dictionary with formatted reply
        """
        message = f"Unknown command: {command}"

        return {
            "destination": destination.upper(),
            "source": self.repeater_callsign,
            "message": message,
            "timestamp": time.time(),
            "command": command,
            "status": "error",
            "error": "Unknown command",
        }

    def format_ax25_frame(self, reply_dict: Dict) -> bytes:
        """
        Format reply as AX.25 frame data.

        Args:
            reply_dict: Reply dictionary from format_*_reply methods

        Returns:
            Bytes ready for AX.25 encoding
        """
        # Format as: "timestamp:source_callsign:message"
        timestamp = reply_dict.get("timestamp", time.time())
        source = reply_dict.get("source", self.repeater_callsign)
        message = reply_dict.get("message", "")

        frame_data = f"{timestamp:.3f}:{source}:{message}"
        return frame_data.encode("utf-8")

    def format_json_reply(self, reply_dict: Dict) -> bytes:
        """
        Format reply as JSON (for ZMQ transmission).

        Args:
            reply_dict: Reply dictionary from format_*_reply methods

        Returns:
            JSON-encoded bytes
        """
        return json.dumps(reply_dict).encode("utf-8")

    def parse_command_result(
        self, command: str, success: bool, result: str
    ) -> Tuple[Optional[str], Optional[str]]:
        """
        Parse command execution result to extract parameter and value.

        Args:
            command: Command that was executed
            success: Whether command succeeded
            result: Result message from command execution

        Returns:
            (parameter_name, value) tuple
        """
        if not success:
            return None, None

        # Extract parameter and value from result message
        if "squelch" in result.lower():
            # Extract dB value
            match = re.search(r"(-?\d+(?:\.\d+)?)\s*dB", result, re.IGNORECASE)
            if match:
                return "squelch", f"{match.group(1)} dB"

        elif "power" in result.lower():
            # Extract power value
            match = re.search(r"(\d+(?:\.\d+)?)\s*%", result, re.IGNORECASE)
            if match:
                return "power", f"{match.group(1)}%"

        elif "timeout" in result.lower():
            # Extract timeout value
            match = re.search(r"(\d+)\s*seconds?", result, re.IGNORECASE)
            if match:
                return "timeout", f"{match.group(1)} seconds"

        elif "restart" in result.lower():
            return "service", "restarted"

        # Fallback: try to extract any number
        match = re.search(r"(\d+(?:\.\d+)?)", result)
        if match:
            return "value", match.group(1)

        return "setting", "updated"

