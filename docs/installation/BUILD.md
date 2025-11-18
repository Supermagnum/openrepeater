# Build and Installation Guide

Complete guide for building and installing the OpenRepeater Authenticated Control System.

## Quick Start

```bash
# Clone repository
git clone https://github.com/OpenRepeater/scripts.git
cd scripts/authenticated_control

# Check dependencies
make check-deps

# Install
sudo make install
```

## Build System Overview

The project uses:
- **Makefile** - Main build system with targets for installation, testing, and development
- **setup.py** - Python package installation (setuptools)
- **pyproject.toml** - Modern Python packaging configuration

## Makefile Targets

### Installation Targets

- `make install` - Full installation (requires root)
- `make install-dev` - Installation with development dependencies
- `make install-dirs` - Create installation directories
- `make install-python` - Install Python packages
- `make install-scripts` - Install executable scripts
- `make install-config` - Install configuration files
- `make install-systemd` - Install systemd service files

### Testing Targets

- `make test` - Run all tests
- `make test-coverage` - Run tests with coverage report
- `make test-svxlink` - Run SVXLink control tests only
- `make test-ax25` - Run AX.25 protocol tests only

### Code Quality Targets

- `make lint` - Run all linters
- `make format` - Format all code
- `make type-check` - Run type checking
- `make security-check` - Run security checks
- `make quick-check` - Quick code quality check

### Build Targets

- `make sdist` - Create source distribution
- `make wheel` - Create wheel distribution
- `make package` - Create both distributions

### Development Targets

- `make dev-setup` - Set up development environment
- `make check-deps` - Check system dependencies
- `make clean` - Remove all build artifacts

## Installation Methods

### Method 1: Using Makefile (Recommended)

```bash
# Full installation
sudo make install

# With development tools
sudo make install-dev
```

### Method 2: Using pip

```bash
# From source directory
pip3 install .

# Development mode (editable)
pip3 install -e .
```

### Method 3: Using setup.py

```bash
# Build package
python3 setup.py sdist bdist_wheel

# Install from wheel
sudo pip3 install dist/*.whl
```

## System Dependencies

### Required Packages

Install via system package manager:

```bash
sudo apt-get install -y \
    python3-dev \
    python3-pip \
    build-essential \
    cmake \
    gnuradio-dev \
    gnuradio-runtime \
    libssl-dev \
    libsodium-dev \
    libkeyutils-dev
```

### GNU Radio OOT Modules

Required modules (install separately):
- gr-linux-crypto
- gr-packet-protocols
- gr-qradiolink

See `INSTALL.md` for installation instructions.

## Installation Paths

Default installation locations:

- **Binaries**: `/usr/local/bin/`
- **Python packages**: Python site-packages directory
- **Configuration**: `/etc/openrepeater/`
- **Data**: `/var/lib/openrepeater/`
- **Logs**: `/var/log/openrepeater/`

## Custom Installation Paths

Edit the Makefile to change installation paths:

```makefile
INSTALL_PREFIX := /opt/openrepeater
INSTALL_ETC := $(INSTALL_PREFIX)/etc
INSTALL_VAR := $(INSTALL_PREFIX)/var
```

## Development Build

For development:

```bash
# Set up environment
make dev-setup

# Install in editable mode
pip3 install -e .

# Run tests
make test

# Format code
make format
```

## Building Packages

### Source Distribution

```bash
make sdist
# Creates: dist/openrepeater-authenticated-control-1.0.0.tar.gz
```

### Wheel Distribution

```bash
make wheel
# Creates: dist/openrepeater_authenticated_control-1.0.0-py3-none-any.whl
```

### Both

```bash
make package
# Creates both source and wheel distributions
```

## Verification

After installation, verify:

```bash
# Check Python imports
python3 -c "from authenticated_control import svxlink_control, ax25_protocol; print('OK')"

# Check installed scripts
which orp-keygen orp-keymgmt operator-cli

# Check GNU Radio modules
python3 -c "from gnuradio import linux_crypto; print('OK')"
```

## Troubleshooting

### Build Errors

1. Check Python version: `python3 --version` (must be 3.8+)
2. Check dependencies: `make check-deps`
3. Install missing packages: See system dependencies above

### Installation Errors

1. Check permissions: May need `sudo`
2. Check disk space: `df -h`
3. Check Python path: `python3 -c "import sys; print(sys.path)"`

### Import Errors

1. Update library cache: `sudo ldconfig`
2. Reinstall package: `sudo pip3 install --force-reinstall .`
3. Check PYTHONPATH environment variable

## Uninstallation

```bash
# Remove package
sudo pip3 uninstall openrepeater-authenticated-control

# Remove scripts (if installed manually)
sudo rm -f /usr/local/bin/orp-* /usr/local/bin/operator-cli

# Remove configuration (optional)
sudo rm -rf /etc/openrepeater
```

## Continuous Integration

The project includes GitHub Actions workflow (`.github/workflows/ci.yml`) that:
- Tests on multiple Python versions
- Runs all code quality checks
- Generates coverage reports
- Builds packages

## Next Steps

After building and installing:
1. Read `INSTALL.md` for post-installation setup
2. Read `OPERATOR_GUIDE.md` for operator usage
3. Read `ADMIN_GUIDE.md` for administrator setup
4. Configure your system (see `ADMIN_GUIDE.md`)

