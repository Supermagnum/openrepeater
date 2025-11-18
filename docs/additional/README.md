# OpenRepeater Authenticated Control System

This directory contains documentation for the OpenRepeater authenticated control system, which enables cryptographically-authenticated remote control of repeater settings via radio.

## Overview

The authenticated control system allows licensed operators to remotely control repeater settings using cryptographically-signed commands transmitted via radio. The system uses:

- **Brainpool ECDSA** for digital signatures
- **FX.25 framing** for error correction
- **GNU Radio OOT modules** for signal processing

Commands remain in the clear for legal compliance while providing strong authentication.

## Documentation

### [INSTALLATION.md](INSTALLATION.md)
Complete installation guide for the three GNU Radio OOT modules:
- Prerequisites and system requirements
- Automated and manual installation procedures
- Verification and troubleshooting

### [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
Guide for operators using the authenticated control system:
- Key generation and management
- Command signing procedures
- Supported commands
- Security best practices

### [CONFIGURATION.md](CONFIGURATION.md)
Configuration guide for repeater operators:
- Authorized keys management
- OpenRepeater module configuration
- GNU Radio flowgraph setup
- SVXLink integration

### [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
Troubleshooting guide for common issues:
- Installation problems
- Configuration issues
- Runtime problems
- Diagnostic commands

### [SECURITY.md](SECURITY.md)
Security considerations and best practices:
- Security model and threat analysis
- Key management procedures
- Access control
- Incident response

## Quick Start

### For Repeater Operators

1. **Install the system:**
   ```bash
   cd /usr/src/scripts
   sudo ./install_authenticated_control.sh
   ```

2. **Generate keys for operators:**
   ```bash
   sudo orp_keygen
   ```

3. **Add operator public keys:**
   ```bash
   sudo orp_keymgmt add OPERATOR_CALLSIGN /path/to/public_key.pem
   ```

4. **Enable module in OpenRepeater web UI**

### For Operators

1. **Generate your key pair:**
   ```bash
   sudo orp_keygen
   ```

2. **Share your public key with repeater operator**

3. **Sign commands:**
   ```bash
   orp_sign_command "SET_SQUELCH -120"
   ```

4. **Transmit signed commands via radio**

## System Components

### GNU Radio Modules

- **gr-qradiolink**: Digital and analog modulation blocks
- **gr-packet-protocols**: Packet radio protocols (AX.25, FX.25)
- **gr-linux-crypto**: Cryptographic infrastructure

### Key Management Utilities

- **orp_keygen**: Generate Brainpool ECDSA key pairs
- **orp_keymgmt**: Manage authorized public keys
- **orp_sign_command**: Sign commands for transmission

### Command Handler

- **command_handler.py**: Verifies and executes authenticated commands

### OpenRepeater Module

- **Authenticated_Control**: Web UI module for configuration

## Legal Compliance

The system is designed for legal compliance with amateur radio regulations:

- Commands remain in the clear (not encrypted)
- Digital signatures are for authentication only
- System meets amateur radio regulatory requirements
- Operators must verify local regulations before deployment

## Support

For assistance:

- Review the documentation in this directory
- Check GitHub issues for known problems
- Contact the OpenRepeater development team

## License

This system is part of the OpenRepeater project. See the main project repository for license information.

