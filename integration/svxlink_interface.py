#!/usr/bin/env python3
"""
SVXLink Interface Module

Handles interaction with SVXLink repeater controller:
- Parse and modify /etc/svxlink/svxlink.conf
- Send SIGHUP to reload configuration
- Execute commands via TCP control port
"""

import configparser
import logging
import os
import signal
import socket
import subprocess
import time
from typing import Optional, Tuple

logger = logging.getLogger(__name__)


class SVXLinkInterface:
    """Interface to SVXLink repeater controller."""

    def __init__(
        self,
        config_file: str = "/etc/svxlink/svxlink.conf",
        tcp_host: str = "localhost",
        tcp_port: int = 5210,
    ):
        """
        Initialize SVXLink interface.

        Args:
            config_file: Path to SVXLink configuration file
            tcp_host: TCP control port host
            tcp_port: TCP control port number
        """
        self.config_file = config_file
        self.tcp_host = tcp_host
        self.tcp_port = tcp_port
        self.config_parser = configparser.ConfigParser(allow_no_value=True)

    def read_config(self) -> bool:
        """Read SVXLink configuration file."""
        try:
            if not os.path.exists(self.config_file):
                logger.error(f"SVXLink config file not found: {self.config_file}")
                return False

            # Read config file preserving comments and structure
            with open(self.config_file, "r", encoding="utf-8") as f:
                self.config_parser.read_file(f)

            return True
        except Exception as e:
            logger.error(f"Failed to read SVXLink config: {e}")
            return False

    def write_config(self) -> bool:
        """Write SVXLink configuration file."""
        try:
            # Backup original config
            backup_file = f"{self.config_file}.backup"
            if os.path.exists(self.config_file):
                import shutil

                shutil.copy2(self.config_file, backup_file)

            with open(self.config_file, "w", encoding="utf-8") as f:
                self.config_parser.write(f)

            return True
        except Exception as e:
            logger.error(f"Failed to write SVXLink config: {e}")
            return False

    def set_squelch(self, threshold: float) -> Tuple[bool, str]:
        """
        Set squelch threshold.

        Args:
            threshold: Squelch threshold in dB

        Returns:
            (success, message)
        """
        # Try TCP first (preferred method)
        success, _ = self.execute_tcp_command(f"SET_SQUELCH {threshold}")
        if success:
            return True, f"Squelch set to {threshold} dB"

        # Fallback to config file modification
        if not self.read_config():
            return False, "Failed to read SVXLink config"

        # Find RX port sections and update squelch
        updated = False
        for section in self.config_parser.sections():
            if section.startswith("RX_"):
                if "SQL_OPEN_THRESH" in self.config_parser[section]:
                    self.config_parser[section]["SQL_OPEN_THRESH"] = str(int(threshold))
                    updated = True
                elif "SIGLEV_OPEN_THRESH" in self.config_parser[section]:
                    # Convert dB to signal level (approximate)
                    siglev = int((threshold + 120) * 2.55)  # Rough conversion
                    self.config_parser[section]["SIGLEV_OPEN_THRESH"] = str(siglev)
                    updated = True

        if updated:
            if self.write_config():
                self.reload_config()
                return True, f"Squelch set to {threshold} dB via config file"
            else:
                return False, "Failed to write config file"

        return False, "Could not find squelch setting in config"

    def set_tx_power(self, power: float) -> Tuple[bool, str]:
        """
        Set transmitter power level.

        Args:
            power: Power level (percentage or absolute value, depends on hardware)

        Returns:
            (success, message)
        """
        # Try TCP first
        success, _ = self.execute_tcp_command(f"SET_TX_POWER {power}")
        if success:
            return True, f"TX power set to {power}%"

        # Config file modification for TX power is hardware-specific
        # Most systems don't support software power control
        return False, "TX power control not available via config file"

    def set_timeout(self, seconds: int) -> Tuple[bool, str]:
        """
        Set transmit timeout.

        Args:
            seconds: Timeout in seconds

        Returns:
            (success, message)
        """
        # Try TCP first
        success, _ = self.execute_tcp_command(f"SET_TX_TIMEOUT {seconds}")
        if success:
            return True, f"TX timeout set to {seconds} seconds"

        # Fallback to config file
        if not self.read_config():
            return False, "Failed to read SVXLink config"

        updated = False
        for section in self.config_parser.sections():
            if section.startswith("TX_"):
                if "TIMEOUT" in self.config_parser[section]:
                    self.config_parser[section]["TIMEOUT"] = str(seconds)
                    updated = True

        if updated:
            if self.write_config():
                self.reload_config()
                return True, f"TX timeout set to {seconds} seconds"
            else:
                return False, "Failed to write config file"

        return False, "Could not find timeout setting in config"

    def restart_service(self) -> Tuple[bool, str]:
        """
        Restart SVXLink service.

        Returns:
            (success, message)
        """
        try:
            # Try systemd restart
            result = subprocess.run(  # nosec B607, B603
                ["systemctl", "restart", "svxlink"],
                capture_output=True,
                text=True,
                timeout=30,
                check=False,
            )

            if result.returncode == 0:
                return True, "SVXLink service restarted"
            else:
                # Fallback to killall and restart
                subprocess.run(
                    ["killall", "svxlink"], timeout=5, check=False
                )  # nosec B607, B603
                time.sleep(1)
                subprocess.Popen(
                    ["svxlink"], start_new_session=True
                )  # nosec B607, B603
                return True, "SVXLink process restarted"

        except subprocess.TimeoutExpired:
            return False, "Timeout waiting for service restart"
        except Exception as e:
            logger.error(f"Failed to restart SVXLink: {e}")
            return False, f"Failed to restart: {e}"

    def execute_tcp_command(self, command: str, timeout: int = 30) -> Tuple[bool, str]:
        """
        Execute command via SVXLink TCP control port.

        Args:
            command: Command string to send
            timeout: Connection timeout in seconds

        Returns:
            (success, response_message)
        """
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(timeout)
            sock.connect((self.tcp_host, self.tcp_port))

            # Send command
            sock.sendall(f"{command}\n".encode("utf-8"))

            # Receive response
            response = sock.recv(4096).decode("utf-8", errors="ignore").strip()
            sock.close()

            logger.info(f"SVXLink TCP response: {response}")
            return True, response

        except socket.timeout:
            error_msg = f"SVXLink TCP connection timeout after {timeout}s"
            logger.error(error_msg)
            return False, error_msg
        except ConnectionRefusedError:
            error_msg = "SVXLink TCP control port not available (connection refused)"
            logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"SVXLink TCP error: {e}"
            logger.error(error_msg)
            return False, error_msg

    def reload_config(self) -> bool:
        """
        Reload SVXLink configuration by sending SIGHUP.

        Returns:
            True if successful
        """
        try:
            # Find svxlink process
            result = subprocess.run(  # nosec B607, B603
                ["pidof", "svxlink"],
                capture_output=True,
                text=True,
                timeout=5,
                check=False,
            )

            if result.returncode == 0:
                pids = result.stdout.strip().split()
                for pid in pids:
                    try:
                        os.kill(int(pid), signal.SIGHUP)
                        logger.info(f"Sent SIGHUP to svxlink process {pid}")
                    except ProcessLookupError:
                        logger.warning(f"Process {pid} not found")
                    except Exception as e:
                        logger.error(f"Failed to send SIGHUP to {pid}: {e}")

                return True
            else:
                logger.warning("svxlink process not found")
                return False

        except Exception as e:
            logger.error(f"Failed to reload SVXLink config: {e}")
            return False

    def execute_command(
        self, command: str, args: Optional[list] = None
    ) -> Tuple[bool, str]:
        """
        Execute a command with arguments.

        Args:
            command: Command name (SET_SQUELCH, SET_POWER, SET_TIMEOUT, RESTART)
            args: Command arguments

        Returns:
            (success, message)
        """
        if args is None:
            args = []

        command_upper = command.upper()

        if command_upper == "SET_SQUELCH":
            if not args:
                return False, "SET_SQUELCH requires threshold argument"
            try:
                threshold = float(args[0])
                return self.set_squelch(threshold)
            except ValueError:
                return False, f"Invalid squelch threshold: {args[0]}"

        elif command_upper == "SET_POWER":
            if not args:
                return False, "SET_POWER requires power level argument"
            try:
                power = float(args[0])
                return self.set_tx_power(power)
            except ValueError:
                return False, f"Invalid power level: {args[0]}"

        elif command_upper == "SET_TIMEOUT":
            if not args:
                return False, "SET_TIMEOUT requires timeout argument"
            try:
                timeout = int(args[0])
                if timeout < 0:
                    return False, "Timeout must be positive"
                return self.set_timeout(timeout)
            except ValueError:
                return False, f"Invalid timeout: {args[0]}"

        elif command_upper == "RESTART":
            return self.restart_service()

        else:
            # Try as raw TCP command
            full_command = (
                f"{command} {' '.join(str(a) for a in args)}" if args else command
            )
            return self.execute_tcp_command(full_command)
