# OpenRepeater Authenticated Control System

Complete authenticated control system for OpenRepeater repeaters using ECDSA digital signatures and AX.25 packet radio protocol.

## Table of Contents

- [Legal Compliance](#legal-compliance)
- [Components](#components)
- [Quick Start](#quick-start)
  - [Installation](#installation)
  - [Building from Source](#building-from-source)
- [Usage](#usage)
  - [Generate Keys](#generate-keys)
  - [Manage Authorized Keys](#manage-authorized-keys)
  - [Send Authenticated Commands](#send-authenticated-commands)
- [Development](#development)
  - [Setup Development Environment](#setup-development-environment)
  - [Run Tests](#run-tests)
  - [Code Quality](#code-quality)
  - [Clean Build Artifacts](#clean-build-artifacts)
- [Makefile Targets](#makefile-targets)
- [System Dependencies](#system-dependencies)
  - [Required](#required)
  - [Optional](#optional)
  - [Installation on Debian/Ubuntu](#installation-on-debianubuntu)
- [Configuration](#configuration)
- [Documentation](#documentation)
  - [Essential Guides](#essential-guides)
  - [Installation & Configuration](#installation--configuration)
  - [Flowgraphs](#flowgraphs)
  - [Components](#components-1)
  - [Installation & Build](#installation--build)
  - [Testing & Results](#testing--results)
  - [Additional Resources](#additional-resources)
- [Systemd Service](#systemd-service)
- [Troubleshooting](#troubleshooting)
  - [Check Installation](#check-installation)
  - [Common Issues](#common-issues)
- [License](#license)
- [Support](#support)

## Legal Compliance

**IMPORTANT**: This system uses a **two-frame protocol** to comply with amateur radio regulations:
- **Frame 1**: Command/result data (unencrypted, readable by anyone)
- **Frame 2**: Signature (proof of authenticity only)

All command and result text is transmitted in the clear. Signatures are in separate frames to clearly distinguish authentication from encryption. See [Protocol Specification](guides/PROTOCOL.md) for complete details.

## Components

- **SVXLink Control Interface**: Command parsing, configuration management, and execution
- **AX.25 Protocol**: Bidirectional authenticated command protocol over packet radio (two-frame format)
- **Key Management**: Tools for generating and managing cryptographic keys
- **GNU Radio Integration**: Flowgraphs for receiving and transmitting authenticated commands
  - **PKCS#11 Support**: Hardware token support via `python3-pkcs11` (Nitrokey, YubiKey, etc.)
  - **Kernel Keyring Support**: Software key support via `gr-linux-crypto`
  - **Dual-Mode**: Flowgraphs support both PKCS#11 and Kernel Keyring with runtime selection

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/OpenRepeater/scripts.git
cd scripts/authenticated_control

# Check dependencies
make check-deps

# Install (requires root)
sudo make install

# Or install with development dependencies
sudo make install-dev
```

### Building from Source

```bash
# Build source distribution
make sdist

# Build wheel
make wheel

# Build both
make package
```

## Usage

### Generate Keys

```bash
# Generate repeater keys
sudo orp-keygen --curve brainpoolP256r1

# Generate operator keys
orp-keygen --output-dir ~/.orp-keys
```

### Manage Authorized Keys

```bash
# Add operator public key
sudo orp-keymgmt add LA1ABC-5 /path/to/public_key.pem --description "Operator Name"

# List authorized keys
sudo orp-keymgmt list

# Revoke a key
sudo orp-keymgmt revoke LA1ABC-5
```

### Send Authenticated Commands

```bash
# Using operator CLI (creates two frames)
operator-cli --send "SET_SQUELCH -120" --wait-response

# Sign a command for manual transmission (outputs two frames)
orp-sign-command "SET_SQUELCH -120"
```

**Note**: Commands are transmitted as two separate frames:
1. Frame 1: Command text (unencrypted, readable)
2. Frame 2: Signature (proof of authenticity)

Both frames must be transmitted in sequence. See [Operator Guide](guides/OPERATOR_GUIDE.md) for details.

## Development

### Setup Development Environment

```bash
make dev-setup
```

### Run Tests

```bash
# Run all tests
make test

# Run with coverage
make test-coverage

# Run specific component tests
make test-svxlink
make test-ax25
```

### Code Quality

```bash
# Format code
make format

# Lint code
make lint

# Type checking
make type-check

# Security checks
make security-check

# Quick check (lint + type check)
make quick-check
```

### Clean Build Artifacts

```bash
make clean
```

## Makefile Targets

Run `make help` to see all available targets:

- **Installation**: `install`, `install-dev`, `install-dirs`, `install-scripts`, `install-config`
- **Testing**: `test`, `test-coverage`, `test-svxlink`, `test-ax25`
- **Code Quality**: `lint`, `format`, `type-check`, `security-check`
- **Build**: `sdist`, `wheel`, `package`
- **Development**: `dev-setup`, `check-deps`
- **Cleanup**: `clean`, `clean-pyc`, `clean-test`, `clean-build`

## System Dependencies

### Required

- Python 3.8 or higher
- pip3
- GNU Radio >= 3.10.12.0 (install via system package manager)
- gr-linux-crypto (install via system package manager)
- gr-packet-protocols (install via system package manager)
- gr-qradiolink (install via system package manager)

### Optional

- RPi.GPIO (for Raspberry Pi GPIO interface)
- pyserial (for serial hardware interface)
- hamlib (for Hamlib radio control)

### Installation on Debian/Ubuntu

```bash
# Install GNU Radio and dependencies
sudo apt-get update
sudo apt-get install -y \
    gnuradio-dev \
    gnuradio-runtime \
    python3-gnuradio \
    python3-pip \
    build-essential \
    cmake

# Install GNU Radio OOT modules (see installation scripts)
cd /usr/src
git clone https://github.com/Supermagnum/gr-linux-crypto.git
git clone https://github.com/Supermagnum/gr-packet-protocols.git
git clone https://github.com/Supermagnum/gr-qradiolink.git
# Build and install each module (see INSTALLATION.md)
```

## Configuration

Configuration files are installed to `/etc/openrepeater/`:

- `config.yml` - Main configuration file
- `keys/` - Directory for authorized public keys

See [Admin Guide](guides/ADMIN_GUIDE.md) for detailed configuration instructions.

## Documentation

### Essential Guides

- **[Protocol Specification](guides/PROTOCOL.md)** - Complete protocol specification (two-frame format)
- **[Operator Guide](guides/OPERATOR_GUIDE.md)** - Operator usage guide
- **[Admin Guide](guides/ADMIN_GUIDE.md)** - Repeater administrator guide
- **[Key Management Guide](guides/KEY_MANAGEMENT.md)** - Comprehensive key management guide
- **[Club Key Hierarchy](guides/CLUB_KEY_HIERARCHY.md)** - Three-tier key management structure for amateur radio clubs

### Installation & Configuration

- **[Installation Guide](additional/INSTALLATION.md)** - Complete installation guide for GNU Radio OOT modules
- **[Configuration Guide](additional/CONFIGURATION.md)** - Configuration guide for repeater operators
- **[Troubleshooting Guide](additional/TROUBLESHOOTING.md)** - Troubleshooting common issues
- **[Security Guide](additional/SECURITY.md)** - Security considerations and best practices

### Flowgraphs

- **[Flowgraph Documentation](flowgraphs/FLOWGRAPHS.md)** - Detailed flowgraph documentation with block references
- **[Flowgraph README](flowgraphs/scripts-flowgraphs-README.md)** - Flowgraph overview

### Components

- **[Component Implementation](components/IMPLEMENTATION_SUMMARY.md)** - SVXLink control interface implementation
- **[Component README](components/README.md)** - Component overview
- **[Component Security](components/SECURITY.md)** - Component security considerations
- **[Component Testing](components/TESTING.md)** - Testing guide

### Installation & Build

- **[Build Guide](installation/BUILD.md)** - Build instructions
- **[Install Guide](installation/INSTALL.md)** - Installation instructions
- **[Build System Summary](installation/BUILD_SYSTEM_SUMMARY.md)** - Build system documentation

### Testing & Results

- **[Test Results](test-results/TEST_RESULTS.md)** - Test and analysis reports

### Additional Resources

- **[Authenticated Control Summary](AUTHENTICATED_CONTROL_SUMMARY.md)** - System overview
- **[Uninstall Guide](UNINSTALL.md)** - Uninstallation instructions

## Systemd Service

The system can run as a systemd service:

```bash
# Install service
sudo make install-systemd

# Enable and start
sudo systemctl enable authenticated-control
sudo systemctl start authenticated-control

# Check status
sudo systemctl status authenticated-control

# View logs
sudo journalctl -u authenticated-control -f
```

## Troubleshooting

### Check Installation

```bash
# Verify Python packages
python3 -c "import yaml, zmq, cryptography; print('OK')"

# Check system dependencies
make check-deps

# Verify installed scripts
which orp-keygen orp-keymgmt operator-cli
```

### Common Issues

1. **Import errors**: Ensure all GNU Radio OOT modules are installed
2. **Permission errors**: Check file permissions in `/etc/openrepeater/`
3. **Key loading errors**: Verify key file format and permissions
4. **Service won't start**: Check logs with `journalctl -u authenticated-control`

## License

GPL-3.0 - See LICENSE file for details

## Support

- GitHub Issues: https://github.com/OpenRepeater/scripts/issues
- Documentation: See docs/ directory
- Community: OpenRepeater forums

