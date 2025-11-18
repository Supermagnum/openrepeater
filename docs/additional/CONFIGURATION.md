# Authenticated Control Configuration Guide

This guide covers the configuration of the authenticated control system for repeater operators.

## Overview

The authenticated control system requires configuration in several areas:
1. Authorized keys management
2. OpenRepeater module configuration
3. GNU Radio flowgraph setup
4. SVXLink integration

## Authorized Keys Management

### Adding Operator Keys

Use the `orp_keymgmt` utility to manage authorized keys:

```bash
# Add an operator's public key
sudo orp_keymgmt add W1AW /path/to/operator_public.pem --description "W1AW Operator"

# List all authorized keys
sudo orp_keymgmt list

# Verify a key file
sudo orp_keymgmt verify /path/to/public_key.pem
```

### Key File Format

Authorized keys are stored in JSON format at `/etc/openrepeater/keys/authorized_keys.json`:

```json
{
  "W1AW": {
    "public_key": "base64_encoded_public_key",
    "fingerprint": "key_fingerprint",
    "description": "W1AW Operator",
    "added": "2024-01-01T00:00:00",
    "active": true
  }
}
```

### Revoking Keys

To revoke a key (mark as inactive):

```bash
sudo orp_keymgmt revoke W1AW
```

To permanently remove a key:

```bash
sudo orp_keymgmt remove W1AW
```

## OpenRepeater Module Configuration

### Enable Module

1. Log into OpenRepeater web UI
2. Navigate to Modules section
3. Enable "Authenticated Control" module
4. Configure settings (see below)

### Module Settings

#### Basic Settings

- **Enable Authenticated Control**: Enable/disable the module
- **RX Frequency**: Receive frequency in Hz (e.g., 145000000)
- **TX Frequency**: Transmit frequency in Hz (for acknowledgments)
- **Modulation**: Digital modulation type (2FSK, 4FSK, GMSK, BPSK, QPSK)
- **Baud Rate**: Data rate in baud (e.g., 1200)

#### Security Settings

- **Authorized Keys File**: Path to authorized keys JSON file
  - Default: `/etc/openrepeater/keys/authorized_keys.json`
- **Command Timeout**: Maximum time to wait for command execution (seconds)
  - Default: 300 seconds
- **Replay Protection Window**: Time window for replay protection (seconds)
  - Default: 3600 seconds (1 hour)
- **Log Commands**: Enable logging of all authenticated commands
- **Log Failed Attempts**: Enable logging of failed authentication attempts

## GNU Radio Flowgraph Configuration

### Basic Setup

1. Open `authenticated_control_rx.grc` in GNU Radio Companion
2. Configure SDR device:
   - Set device arguments (e.g., `rtl=0` for RTL-SDR)
   - Configure sample rate
3. Set frequency:
   - Match RX frequency from module settings
4. Configure demodulation:
   - Set baud rate to match module settings
   - Adjust frequency deviation if needed

### SDR Device Configuration

#### RTL-SDR

```python
device_args = "rtl=0"
```

#### HackRF

```python
device_args = "hackrf=0"
```

#### USRP

```python
device_args = "uhd=0"
```

### Frequency Configuration

Set the receive frequency to match your configured frequency:

```python
frequency = 145000000  # 145.000 MHz
```

### Demodulation Parameters

For 2FSK at 1200 baud:

```python
baud_rate = 1200
freq_dev = 2400  # Frequency deviation (typically 2x baud rate)
```

## SVXLink Integration

### Command Execution

The command handler interfaces with SVXLink via:

1. **Unix Sockets**: Direct communication with SVXLink processes
2. **Configuration Files**: Modify SVXLink configuration files
3. **Event System**: Trigger SVXLink events

### Supported Commands

The system supports the following commands:

- `SET_SQUELCH <threshold>`: Set squelch threshold in dB
- `SET_TX_POWER <power>`: Set transmitter power level
- `ENABLE_REPEATER`: Enable the repeater
- `DISABLE_REPEATER`: Disable the repeater
- `STATUS`: Query repeater status

### Custom Command Implementation

To add custom commands, modify `command_handler.py`:

```python
def execute_command(self, command: str) -> Tuple[bool, str]:
    command_parts = command.split()
    cmd = command_parts[0].upper()
    args = command_parts[1:] if len(command_parts) > 1 else []
    
    if cmd == "CUSTOM_COMMAND":
        # Implement custom command logic
        return True, "Custom command executed"
    
    # ... existing commands ...
```

## System Integration

### Service Configuration

Create a systemd service for the command handler:

```ini
[Unit]
Description=OpenRepeater Authenticated Control Command Handler
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/python3 /usr/local/bin/authenticated_control/command_handler.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### Logging Configuration

Configure logging in `/etc/rsyslog.d/openrepeater.conf`:

```
# OpenRepeater Authenticated Control logs
local0.*    /var/log/openrepeater/authenticated_control.log
```

### Monitoring

Monitor the system using:

```bash
# View command handler logs
tail -f /var/log/openrepeater/authenticated_control.log

# View failed authentication attempts
grep "FAILED" /var/log/openrepeater/authenticated_control.log

# View executed commands
grep "SUCCESS" /var/log/openrepeater/authenticated_control.log
```

## Security Configuration

### File Permissions

Ensure proper file permissions:

```bash
# Authorized keys file
chmod 600 /etc/openrepeater/keys/authorized_keys.json
chown root:root /etc/openrepeater/keys/authorized_keys.json

# Keys directory
chmod 700 /etc/openrepeater/keys
chown root:root /etc/openrepeater/keys
```

### Firewall Configuration

If using network-based command transmission:

```bash
# Allow only specific IP addresses
iptables -A INPUT -p tcp --dport 12345 -s 192.168.1.0/24 -j ACCEPT
iptables -A INPUT -p tcp --dport 12345 -j DROP
```

### Access Control

Limit access to key management utilities:

```bash
# Restrict key management to specific users
chmod 750 /usr/local/bin/orp_keymgmt
chown root:openrepeater /usr/local/bin/orp_keymgmt
```

## Testing Configuration

### Test Key Generation

```bash
# Generate test keys
sudo orp_keygen --output-dir /tmp/test_keys

# Verify keys
orp_keymgmt verify /tmp/test_keys/operator_public.pem
```

### Test Command Signing

```bash
# Sign a test command
orp_sign_command "STATUS" --private-key /tmp/test_keys/operator_private.pem
```

### Test Command Verification

```bash
# Add test key to authorized keys
sudo orp_keymgmt add TEST /tmp/test_keys/operator_public.pem --description "Test Key"

# Test command handler
python3 /usr/local/bin/authenticated_control/command_handler.py \
    --authorized-keys /etc/openrepeater/keys/authorized_keys.json \
    "$(orp_sign_command "STATUS" --output raw):TEST"
```

## Troubleshooting

### Commands Not Executing

1. Check command handler is running
2. Verify authorized keys are configured
3. Check logs for error messages
4. Verify SVXLink is running and accessible

### Signature Verification Failing

1. Verify public key matches private key
2. Check timestamp is within replay protection window
3. Verify key is active in authorized keys list
4. Check command format is correct

### Flowgraph Not Receiving

1. Verify SDR device is connected and recognized
2. Check frequency configuration matches
3. Verify signal strength and quality
4. Check demodulation parameters

## Advanced Configuration

### Hardware Security Module

To use Nitrokey or TPM for key storage:

1. Install HSM drivers and libraries
2. Configure GnuPG for HSM access
3. Generate keys on HSM device
4. Configure command handler to use HSM keys

### Custom Replay Protection

Modify replay protection in `command_handler.py`:

```python
def verify_timestamp(self, timestamp: int) -> bool:
    # Custom replay protection logic
    # ...
```

### Command Rate Limiting

Add rate limiting to prevent command flooding:

```python
# In command_handler.py
self.command_times = []
MAX_COMMANDS_PER_MINUTE = 10

def check_rate_limit(self) -> bool:
    current_time = time.time()
    self.command_times = [t for t in self.command_times if current_time - t < 60]
    return len(self.command_times) < MAX_COMMANDS_PER_MINUTE
```

## Support

For configuration assistance:

- Review [INSTALLATION.md](INSTALLATION.md) for installation details
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for common issues
- Contact OpenRepeater development team for technical support

