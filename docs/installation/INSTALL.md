# Installation Guide

Complete installation guide for OpenRepeater Authenticated Control System.

## Prerequisites

### System Requirements

- **Operating System**: Debian 12 (Bookworm) or compatible Linux distribution
- **Python**: 3.8 or higher
- **Architecture**: ARM (Raspberry Pi), x86_64, or compatible
- **Disk Space**: Minimum 2GB free space
- **Memory**: Minimum 512MB RAM

### Required System Packages

```bash
# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-wheel

# Install GNU Radio (if not already installed)
sudo apt-get install -y \
    gnuradio-dev \
    gnuradio-runtime \
    python3-gnuradio

# Install cryptographic libraries
sudo apt-get install -y \
    libssl-dev \
    libsodium-dev \
    libkeyutils-dev

# Optional: Hardware support
sudo apt-get install -y \
    libnitrokey-dev \  # For hardware security modules
    libhamlib-dev \    # For Hamlib radio control
    python3-serial     # For serial interfaces
```

## GNU Radio OOT Modules

The system requires three GNU Radio out-of-tree (OOT) modules:

1. **gr-linux-crypto** - Cryptographic infrastructure
2. **gr-packet-protocols** - AX.25 and FX.25 support
3. **gr-qradiolink** - Digital modulation blocks

### Automated Installation

Use the provided installation script:

```bash
cd /usr/src/scripts
sudo ./install_authenticated_control.sh
```

### Manual Installation

See `functions/functions_authenticated_control.sh` for detailed installation steps.

## Python Package Installation

### From Source

```bash
# Clone repository
git clone https://github.com/OpenRepeater/scripts.git
cd scripts/authenticated_control

# Check dependencies
make check-deps

# Install
sudo make install

# Or install with development dependencies
sudo make install-dev
```

### Using pip

```bash
# Install from source directory
pip3 install .

# Or install in development mode
pip3 install -e .
```

### Using setup.py

```bash
# Build package
python3 setup.py sdist bdist_wheel

# Install
sudo pip3 install dist/openrepeater-authenticated-control-*.whl
```

## Post-Installation Setup

### 1. Create Directories

```bash
sudo mkdir -p /etc/openrepeater/keys
sudo mkdir -p /var/lib/openrepeater/config_backups
sudo mkdir -p /var/log/openrepeater
sudo chmod 700 /etc/openrepeater/keys
sudo chmod 700 /var/lib/openrepeater/config_backups
sudo chmod 750 /var/log/openrepeater
```

### 2. Generate Repeater Keys

```bash
sudo orp-keygen --curve brainpoolP256r1 --output-dir /etc/openrepeater/keys
```

### 3. Configure System

```bash
# Copy example configuration
sudo cp /etc/openrepeater/config.yml.example /etc/openrepeater/config.yml
sudo chmod 600 /etc/openrepeater/config.yml

# Edit configuration
sudo nano /etc/openrepeater/config.yml
```

### 4. Add Operator Keys

```bash
# Add operator public keys
sudo orp-keymgmt add OPERATOR_CALLSIGN /path/to/public_key.pem --description "Operator Name"
```

### 5. Install Systemd Service (Optional)

```bash
# Install service file
sudo make install-systemd

# Enable and start
sudo systemctl enable authenticated-control
sudo systemctl start authenticated-control

# Check status
sudo systemctl status authenticated-control
```

## Verification

### Check Installation

```bash
# Verify Python packages
python3 -c "from authenticated_control import svxlink_control, ax25_protocol; print('OK')"

# Check installed scripts
which orp-keygen orp-keymgmt operator-cli

# Verify GNU Radio modules
python3 -c "from gnuradio import linux_crypto; print('gr-linux-crypto OK')"
python3 -c "import gnuradio.packet_protocols as pp; print('gr-packet-protocols OK')"
python3 -c "from gnuradio import qradiolink; print('gr-qradiolink OK')"
```

### Test Key Generation

```bash
# Generate test keys
mkdir -p /tmp/test_keys
orp-keygen --output-dir /tmp/test_keys

# Verify keys
orp-keymgmt verify /tmp/test_keys/operator_public.pem
```

## Troubleshooting

### Import Errors

If you see import errors for GNU Radio modules:

```bash
# Update library cache
sudo ldconfig

# Verify module installation
find /usr/local -name "*linux_crypto*"
find /usr/local -name "*packet_protocols*"
find /usr/local -name "*qradiolink*"
```

### Permission Errors

```bash
# Fix directory permissions
sudo chown -R svxlink:svxlink /etc/openrepeater/keys
sudo chown -R svxlink:svxlink /var/lib/openrepeater
sudo chown -R svxlink:svxlink /var/log/openrepeater
```

### Service Won't Start

```bash
# Check service logs
sudo journalctl -u authenticated-control -n 50

# Check configuration
sudo /usr/local/bin/orp-command-handler --config /etc/openrepeater/config.yml --dry-run
```

## Uninstallation

```bash
# Remove Python package
sudo pip3 uninstall openrepeater-authenticated-control

# Remove scripts
sudo rm -f /usr/local/bin/orp-keygen
sudo rm -f /usr/local/bin/orp-keymgmt
sudo rm -f /usr/local/bin/orp-sign-command
sudo rm -f /usr/local/bin/operator-cli
sudo rm -f /usr/local/bin/orp-command-handler

# Remove configuration (optional)
sudo rm -rf /etc/openrepeater
sudo rm -rf /var/lib/openrepeater
sudo rm -rf /var/log/openrepeater

# Remove systemd service
sudo systemctl stop authenticated-control
sudo systemctl disable authenticated-control
sudo rm -f /etc/systemd/system/authenticated-control.service
sudo systemctl daemon-reload
```

## Development Installation

For development work:

```bash
# Clone repository
git clone https://github.com/OpenRepeater/scripts.git
cd scripts/authenticated_control

# Set up development environment
make dev-setup

# Install in editable mode
pip3 install -e .

# Run tests
make test

# Run code quality checks
make lint
make type-check
```

## Next Steps

After installation:

1. Read `OPERATOR_GUIDE.md` for operator instructions
2. Read `ADMIN_GUIDE.md` for administrator setup
3. Read `PROTOCOL.md` for protocol details
4. Configure your repeater (see `ADMIN_GUIDE.md`)
5. Test with operator client (see `OPERATOR_GUIDE.md`)

