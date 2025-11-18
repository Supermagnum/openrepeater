# Authenticated Repeater Control System

Complete authenticated control system for amateur radio repeaters using cryptographic signatures and GNU Radio.

## Overview

This system enables licensed operators to remotely control repeater settings using cryptographically-signed commands transmitted via radio. It eliminates the need for dangerous trips to remote mountain repeater sites in harsh weather conditions.

### What Problem Does This Solve?

Repeater maintenance and configuration typically requires physical access to the repeater site:

- **Dangerous** - Harsh weather, avalanche risk, difficult terrain
- **Time-consuming** - Some sites require hours of travel
- **Expensive** - Fuel, equipment wear, potential rescue costs
- **Inefficient** - Simple adjustments shouldn't require expeditions

### How It Works

1. **Operator sends command** via radio (e.g., "LA5MR SET_SQUELCH -24")
2. **System signs command** with ECDSA (Brainpool curves) using Nitrokey or software keys
3. **Transmits two AX.25 frames**:
   - Frame 1: ASCII command (unencrypted, readable)
   - Frame 2: Digital signature (proof of authenticity)
4. **Repeater verifies signature** against authorized operator public keys
5. **If valid, executes command** via SVXLink control interface
6. **Logs operation** with operator callsign and timestamp

Voice transmissions remain completely open - normal amateur radio conversations pass through unchanged and unencrypted, fully compliant with radio regulations. Only the control commands are authenticated.

### Key Benefits

- **Safety** - No more dangerous winter trips to mountain sites
- **Efficiency** - Instant network-wide configuration from any location
- **Security** - Impossible to forge commands, even if intercepted
- **Future-proof** - Works with upcoming open-source SDR handhelds
- **Legally compliant** - Authentication only, no encrypted voice

## Project Structure

```
authenticated-repeater-control/
├── modules/                    # GNU Radio OOT modules
│   ├── gr-linux-crypto/       # Kernel keyring cryptographic support
│   ├── gr-packet-protocols/   # AX.25/FX.25 protocol support
│   └── gr-qradiolink/         # Radio link processing
├── openrepeater/              # OpenRepeater integration
│   ├── openrepeater/         # Main OpenRepeater repository
│   └── scripts/              # Installation scripts (modified)
├── flowgraphs/                # Your GRC flowgraph files go here
│   ├── tx_authenticated.grc   # Transmitter flowgraph
│   └── rx_authenticated.grc   # Receiver flowgraph
├── integration/               # Integration scripts and services
│   ├── authenticated_command_handler.py    # Command handler service
│   ├── install_authenticated_modules.sh    # Standalone installer
│   └── authenticated-control.service       # Systemd service file
└── README.md                 # This file
```

## Prerequisites

### Hardware

- **Single-Board Computer** (Raspberry Pi recommended)
- **SDR Hardware** (RTL-SDR, HackRF, PlutoSDR, etc.)
- **Radio Interface** (for actual RF transmission/reception)
- **Nitrokey or YubiKey** (optional, for hardware-backed keys)

### Software

- **Debian 12 (Bookworm)** or compatible Linux distribution
- **GNU Radio >= 3.10.12.0**
- **Python 3.8+**
- **SVXLink** (repeater controller software)
- **OpenRepeater** (web-based repeater management)

## Installation

### Option 1: Integrated with OpenRepeater

If installing as part of the full OpenRepeater system:

```bash
cd openrepeater/scripts
sudo ./install_orp.sh
```

The authenticated control system will be installed automatically during the OpenRepeater installation process.

### Option 2: Standalone Installation

To install only the authenticated control system:

```bash
cd integration
sudo ./install_authenticated_modules.sh
```

This will:
1. Install GNU Radio (if not present)
2. Build and install GNU Radio OOT modules
3. Set up Python environment and dependencies
4. Create key management directories
5. Install systemd service
6. Verify installation

## Configuration

### 1. Key Management

#### Generate Repeater Keypair

**Recommended Method: Using Seahorse GUI**

Seahorse is the recommended GUI for key handling. Install it with:

```bash
sudo apt install seahorse seahorse-nautilus
```

Using Seahorse:

1. Generate a new keypair on your Nitrokey device (or create a software key)
2. Export the public key in PEM format
3. Save the public key to `/etc/authenticated-repeater/repeater_keys/public.pem`

For Nitrokey devices, the private key remains securely stored on the Nitrokey device and is accessed via PKCS#11.

**Alternative Method: Software Key Generation**

If not using a Nitrokey, you can generate software keys using the kernel keyring or manually:

```bash
# Using Brainpool ECC (recommended for battery-powered devices)
python3 -c "
import os
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

# Generate keypair
private_key = ec.generate_private_key(ec.BrainpoolP256R1(), default_backend())
public_key = private_key.public_key()

# Save private key
with open('/etc/authenticated-repeater/repeater_keys/private.pem', 'wb') as f:
    f.write(private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption()
    ))
os.chmod('/etc/authenticated-repeater/repeater_keys/private.pem', 0o600)

# Save public key
with open('/etc/authenticated-repeater/repeater_keys/public.pem', 'wb') as f:
    f.write(public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    ))
"
```

**Note:** For production use, hardware-backed keys (Nitrokey/YubiKey) are strongly recommended for enhanced security.

#### Add Authorized Operator Keys

Place operator public keys in `/etc/authenticated-repeater/authorized_operators/`:

```bash
# Copy operator's public key
sudo cp LA1ABC.pem /etc/authenticated-repeater/authorized_operators/

# Or import GPG key (basic support)
sudo cp LA1ABC.asc /etc/authenticated-repeater/authorized_operators/
```

### 2. System Configuration

Edit `/etc/authenticated-repeater/config.yaml`:

```yaml
# IPC mechanism: 'zmq', 'fifo', or 'socket'
ipc_mechanism: zmq

# SVXLink control method: 'tcp', 'config', or 'dtmf'
svxlink_control: tcp
svxlink_tcp_host: localhost
svxlink_tcp_port: 5210

# Logging
log_file: /var/log/authenticated-repeater/commands.log
log_level: INFO

# Security settings
replay_protection_window: 300  # seconds
max_commands_per_minute: 10
```

### 3. Place Flowgraphs

Copy your GRC flowgraph files to:

```bash
sudo cp tx_authenticated.grc /usr/local/share/authenticated-repeater/flowgraphs/
sudo cp rx_authenticated.grc /usr/local/share/authenticated-repeater/flowgraphs/
```

See `flowgraphs/README.md` for flowgraph requirements.

### 4. Configure SVXLink

Enable SVXLink TCP control port in `/etc/svxlink/svxlink.conf`:

```ini
[ReflectorLogic]
TcpPort=5210
```

Or use config file modification method (see INTEGRATION.md for details).

## Usage

### Start the Service

```bash
sudo systemctl start authenticated-control
sudo systemctl status authenticated-control
```

### View Logs

```bash
# Service logs
sudo journalctl -u authenticated-control -f

# Command execution log
tail -f /var/log/authenticated-repeater/commands.log
```

### Send Authenticated Command

From operator station:

1. **Enter command** in transmitter flowgraph GUI
2. **Press send button** to sign and transmit
3. **Command is transmitted** as two AX.25 frames
4. **Repeater receives**, verifies, and executes
5. **Acknowledgment** sent back (if configured)

### Example Commands

- `SET_SQUELCH -120` - Set squelch threshold
- `SET_TX_TIMEOUT 300` - Set transmit timeout
- `ENABLE_MODULE EchoLink` - Enable EchoLink module
- `DISABLE_MODULE EchoLink` - Disable EchoLink module

(Actual commands depend on SVXLink configuration)

## Security Considerations

1. **Key Storage**: Private keys must be protected (0600 permissions)
2. **Authorized Keys**: Only trusted operators should have keys in authorized_operators/
3. **Replay Protection**: Commands include timestamps and are checked for duplicates
4. **Rate Limiting**: Maximum commands per minute per operator
5. **Logging**: All commands are logged with operator identity and timestamp
6. **Network Security**: If using TCP control, ensure firewall rules are appropriate

See `integration/INTEGRATION.md` for detailed security documentation.

## Troubleshooting

### Service Won't Start

```bash
# Check service status
sudo systemctl status authenticated-control

# Check logs
sudo journalctl -u authenticated-control -n 50

# Verify configuration
python3 -c "import yaml; yaml.safe_load(open('/etc/authenticated-repeater/config.yaml'))"
```

### No Commands Received

1. **Check IPC connection:**
   ```bash
   # ZMQ
   ls -l /tmp/authenticated_*.sock
   
   # FIFO
   ls -l /tmp/authenticated_commands.fifo
   ```

2. **Verify flowgraph is running** and sending to correct IPC endpoint

3. **Check authorized keys:**
   ```bash
   ls -la /etc/authenticated-repeater/authorized_operators/
   ```

### Signature Verification Fails

1. **Verify operator key is in authorized_operators/**
2. **Check key format** (must be PEM or GPG ASCII)
3. **Verify key matches** the one used to sign the command
4. **Check system clock** (timestamps must be within replay protection window)

### SVXLink Command Execution Fails

1. **Verify SVXLink is running:**
   ```bash
   sudo systemctl status svxlink
   ```

2. **Check TCP control port** (if using TCP method):
   ```bash
   telnet localhost 5210
   ```

3. **Verify command syntax** matches SVXLink expectations

## Related Repositories

- [gr-linux-crypto](https://github.com/Supermagnum/gr-linux-crypto) - Kernel keyring cryptographic support
- [gr-packet-protocols](https://github.com/Supermagnum/gr-packet-protocols) - AX.25/FX.25 protocol support
- [gr-qradiolink](https://github.com/Supermagnum/gr-qradiolink) - Radio link processing
- [OpenRepeater](https://github.com/OpenRepeater/openrepeater) - Main OpenRepeater repository

## Documentation

### Essential Documentation

- **[Technical Integration Guide](integration/INTEGRATION.md)** - Complete technical documentation covering system architecture, command flow, IPC mechanisms, SVXLink integration, security features, and troubleshooting
- **[Flowgraph Guide](flowgraphs/README.md)** - Requirements and examples for creating transmitter and receiver flowgraphs
- **[Setup Complete](SETUP_COMPLETE.md)** - Setup summary and next steps

### Detailed Documentation

- **[Protocol Specification](docs/guides/PROTOCOL.md)** - Complete AX.25 authenticated command protocol specification (two-frame format)
- **[Key Management Guide](docs/guides/KEY_MANAGEMENT.md)** - Comprehensive guide to key creation, handling, key servers, and battery-friendly cryptography
- **[Flowgraph Documentation](docs/flowgraphs/FLOWGRAPHS.md)** - Detailed flowgraph documentation with block references and hardware integration
- **[Admin Guide](docs/guides/ADMIN_GUIDE.md)** - Administrator guide for repeater setup and configuration
- **[Operator Guide](docs/guides/OPERATOR_GUIDE.md)** - Operator usage guide for sending authenticated commands

### Installation & Configuration

- **[Installation Guide](docs/additional/INSTALLATION.md)** - Complete installation guide for GNU Radio OOT modules
- **[Configuration Guide](docs/additional/CONFIGURATION.md)** - Configuration guide for repeater operators
- **[Troubleshooting Guide](docs/additional/TROUBLESHOOTING.md)** - Troubleshooting common issues

### Security & Testing

- **[Security Guide](docs/additional/SECURITY.md)** - Security considerations and best practices
- **[Test Results](docs/test-results/TEST_RESULTS.md)** - Test and analysis reports
- **[Code Quality Report](CODE_QUALITY_REPORT.md)** - Code quality analysis results

### Additional Resources

- **[Component Implementation](docs/components/IMPLEMENTATION_SUMMARY.md)** - SVXLink control interface implementation details
- **[Build System](docs/installation/BUILD_SYSTEM_SUMMARY.md)** - Build system documentation

## License

GPL-3.0 - See LICENSE files in individual repositories

## Support

- GitHub Issues: Create an issue in the appropriate repository
- Documentation: See `integration/INTEGRATION.md` for technical details
- Community: OpenRepeater forums

## Contributing

Contributions welcome! Please:

1. Follow code quality standards (see below)
2. Test on real hardware when possible
3. Document your changes
4. Submit pull requests with clear descriptions

## Code Quality

All code must pass:

- **bandit** - No security issues
- **flake8** - PEP 8 style compliance
- **shellcheck** - Shell script analysis
- **pylint** - Python code analysis
- **mypy** - Type checking (where applicable)

See code quality report in project documentation.

