# Authenticated Control System Installation Guide

This guide covers the installation of the GNU Radio out-of-tree (OOT) modules required for OpenRepeater authenticated control functionality.

## Overview

The authenticated control system enables remote, cryptographically-authenticated commands to control repeater settings without requiring physical site visits. The system uses:

- **gr-linux-crypto**: Linux-specific cryptographic infrastructure
- **gr-packet-protocols**: Packet radio protocol implementations (AX.25, FX.25)
- **gr-qradiolink**: Digital and analog modulation blocks

## Prerequisites

### System Requirements

- Debian 12 (Bookworm) or compatible
- GNU Radio >= 3.10.12.0
- Minimum 5GB free disk space
- Root access
- Internet connection

### Verify GNU Radio Installation

Before proceeding, verify that GNU Radio is installed and meets the version requirement:

```bash
gnuradio-config-info --version
```

Or:

```bash
python3 -c "import gnuradio; print(gnuradio.version())"
```

## Installation Methods

### Method 1: Automated Installation Script (Recommended)

The easiest way to install all three modules is using the provided installation script:

```bash
cd /usr/src/scripts
chmod +x install_authenticated_control.sh
sudo ./install_authenticated_control.sh
```

This script will:
1. Check prerequisites (GNU Radio version, disk space)
2. Install all required dependencies
3. Clone and build all three modules in the correct order
4. Configure GnuPG for headless operation
5. Verify the installation

### Method 2: Manual Installation

If you prefer to install manually or need to customize the build process:

#### Step 1: Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    libkeyutils-dev \
    gnuradio-dev \
    gnuradio-runtime \
    cmake \
    build-essential \
    pkg-config \
    python3-dev \
    python3-pip \
    libssl-dev \
    libsodium-dev \
    libnitrokey-dev \
    git \
    libvolk-dev \
    libboost-all-dev \
    libzmq3-dev \
    libfmt-dev \
    libcodec2-dev \
    libgsm1-dev \
    libopus-dev \
    libspeex-dev
```

#### Step 2: Install gr-qradiolink (First)

```bash
cd /usr/src
git clone https://github.com/Supermagnum/gr-qradiolink.git
cd gr-qradiolink
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
sudo ldconfig
```

#### Step 3: Install gr-packet-protocols (Second)

```bash
cd /usr/src
git clone https://github.com/Supermagnum/gr-packet-protocols.git
cd gr-packet-protocols
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
sudo ldconfig
```

#### Step 4: Install gr-linux-crypto (Last)

```bash
cd /usr/src
git clone https://github.com/Supermagnum/gr-linux-crypto.git
cd gr-linux-crypto
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
sudo ldconfig
```

## Verification

After installation, verify that all modules are properly installed:

```bash
python3 -c "from gnuradio import qradiolink; print('gr-qradiolink OK')"
python3 -c "import gnuradio.packet_protocols as pp; print('gr-packet-protocols OK')"
python3 -c "from gnuradio import linux_crypto; print('gr-linux-crypto OK')"
```

All three commands should print "OK" without errors.

## Post-Installation Configuration

### Configure GnuPG

The installation script automatically configures GnuPG for headless operation. If installing manually, create the following configuration:

```bash
mkdir -p /root/.gnupg
chmod 700 /root/.gnupg

cat > /root/.gnupg/gpg-agent.conf << 'EOF'
default-cache-ttl 3600
max-cache-ttl 7200
pinentry-program /usr/bin/pinentry-curses
allow-loopback-pinentry
EOF

cat > /root/.gnupg/gpg.conf << 'EOF'
no-tty
batch
yes
EOF
```

### Create Keys Directory

```bash
sudo mkdir -p /etc/openrepeater/keys
sudo chmod 700 /etc/openrepeater/keys
```

## Troubleshooting

### Build Errors

If you encounter build errors:

1. **Check GNU Radio version**: Ensure GNU Radio >= 3.10.12.0 is installed
2. **Check dependencies**: Verify all required packages are installed
3. **Clean build**: Remove build directories and rebuild from scratch
4. **Check logs**: Review error logs in `/var/log/authenticated_control_error.log`

### Import Errors

If Python imports fail:

1. **Check installation**: Verify modules were installed to the correct location
2. **Run ldconfig**: Execute `sudo ldconfig` to update library cache
3. **Check Python path**: Verify Python can find the GNU Radio modules
4. **Rebuild**: Try rebuilding the problematic module

### Hardware Security Module Issues

If using Nitrokey or other HSM:

1. **Check device**: Verify HSM is connected and recognized
2. **Check permissions**: Ensure user has access to HSM device
3. **Install drivers**: Verify HSM drivers are installed
4. **Test with gpg**: Test HSM access with `gpg --card-status`

## Next Steps

After successful installation:

1. Generate operator keys (see [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md))
2. Configure authorized keys (see [CONFIGURATION.md](CONFIGURATION.md))
3. Enable the Authenticated_Control module in OpenRepeater web UI
4. Configure GNU Radio flowgraph for your hardware

## Support

For issues or questions:

- Check the [TROUBLESHOOTING.md](TROUBLESHOOTING.md) guide
- Review GitHub issues for known problems
- Contact the OpenRepeater development team

