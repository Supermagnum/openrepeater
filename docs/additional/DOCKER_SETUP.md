# Docker Setup Guide

This guide explains how to set up and run the Authenticated Repeater Control system using Docker.

## Overview

Docker provides an isolated environment for running the authenticated control system, making it easier to deploy and manage without affecting the host system.

## Prerequisites

- Docker Engine 20.10 or later
- Docker Compose 2.0 or later (optional, for multi-container setups)
- At least 4GB RAM available for Docker
- Network access for pulling images and dependencies

## Quick Start

### Option 1: Using Docker Compose (Recommended)

1. **Create docker-compose.yml**:

```yaml
version: '3.8'

services:
  authenticated-control:
    build:
      context: .
      dockerfile: Dockerfile
    container_name: authenticated-repeater-control
    restart: unless-stopped
    privileged: true  # Required for GPIO access if using PTT
    network_mode: host  # Required for radio hardware access
    volumes:
      - ./config:/etc/authenticated-repeater:ro
      - ./flowgraphs:/usr/local/share/authenticated-repeater/flowgraphs:ro
      - ./logs:/var/log/authenticated-repeater
      - /dev:/dev  # For SDR hardware access
    environment:
      - AUTHENTICATED_CONFIG=/etc/authenticated-repeater/config.yaml
      - PYTHONPATH=/usr/local/lib/python3/dist-packages
    devices:
      - /dev/ttyUSB0:/dev/ttyUSB0  # Serial devices (if needed)
      - /dev/ttyACM0:/dev/ttyACM0  # USB serial devices (if needed)
```

2. **Build and run**:

```bash
docker-compose up -d
```

3. **View logs**:

```bash
docker-compose logs -f authenticated-control
```

### Option 2: Using Docker Directly

1. **Build the image**:

```bash
docker build -t authenticated-repeater-control .
```

2. **Run the container**:

```bash
docker run -d \
  --name authenticated-repeater-control \
  --privileged \
  --network host \
  -v $(pwd)/config:/etc/authenticated-repeater:ro \
  -v $(pwd)/flowgraphs:/usr/local/share/authenticated-repeater/flowgraphs:ro \
  -v $(pwd)/logs:/var/log/authenticated-repeater \
  -v /dev:/dev \
  -e AUTHENTICATED_CONFIG=/etc/authenticated-repeater/config.yaml \
  -e PYTHONPATH=/usr/local/lib/python3/dist-packages \
  authenticated-repeater-control
```

## Dockerfile

Create a `Dockerfile` in the project root:

```dockerfile
FROM debian:bookworm-slim

# Install system dependencies
RUN apt-get update && apt-get install -y \
    gnuradio-dev \
    gnuradio-runtime \
    python3-gnuradio \
    python3-dev \
    python3-pip \
    python3-cryptography \
    python3-pkcs11 \
    python3-zmq \
    python3-yaml \
    libkeyutils-dev \
    libssl-dev \
    libsodium-dev \
    cmake \
    build-essential \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY integration/ /app/integration/
COPY modules/ /app/modules/
COPY openrepeater/scripts/functions/functions_authenticated_control.sh /app/scripts/

# Build GNU Radio modules
RUN cd /app/modules/gr-qradiolink && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install && ldconfig

RUN cd /app/modules/gr-packet-protocols && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install && ldconfig

RUN cd /app/modules/gr-linux-crypto && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install && ldconfig

# Create symlink for gr_linux_crypto
RUN ln -s /app/modules/gr-linux-crypto/python /app/modules/gr-linux-crypto/gr_linux_crypto

# Install Python dependencies
RUN pip3 install --no-cache-dir cryptography pyyaml pyzmq

# Create directories
RUN mkdir -p /etc/authenticated-repeater/authorized_operators \
    /etc/authenticated-repeater/repeater_keys \
    /var/log/authenticated-repeater \
    /usr/local/share/authenticated-repeater/flowgraphs

# Set environment variables
ENV PYTHONPATH=/app/modules/gr-linux-crypto:/usr/local/lib/python3/dist-packages:$PYTHONPATH
ENV AUTHENTICATED_CONFIG=/etc/authenticated-repeater/config.yaml

# Set entrypoint
WORKDIR /app/integration
ENTRYPOINT ["python3", "authenticated_command_handler.py"]
```

## Configuration

### Directory Structure

Create the following directory structure on your host:

```
.
├── config/
│   ├── config.yaml
│   └── authorized_operators/
│       └── (operator public keys)
├── flowgraphs/
│   ├── signed-message-tx.grc
│   └── signed-message-rx.grc
└── logs/
    └── (log files will be created here)
```

### Config File

Place your `config.yaml` in the `config/` directory:

```yaml
# IPC mechanism
ipc_mechanism: zmq

# ZMQ socket paths
zmq_rx_socket: ipc:///tmp/authenticated_rx.sock
zmq_tx_socket: ipc:///tmp/authenticated_tx.sock

# SVXLink control
svxlink_control: tcp
svxlink_tcp_host: localhost
svxlink_tcp_port: 5210

# Logging
log_file: /var/log/authenticated-repeater/commands.log
log_level: INFO

# Security settings
replay_protection_window: 300
max_commands_per_minute: 10
command_timeout: 30

# Key management
authorized_keys_dir: /etc/authenticated-repeater/authorized_operators
repeater_keys_dir: /etc/authenticated-repeater/repeater_keys
```

## Hardware Access

### SDR Hardware

For SDR hardware access (RTL-SDR, HackRF, etc.):

```bash
docker run --device=/dev/bus/usb ...
```

Or use `--privileged` flag (less secure but simpler):

```bash
docker run --privileged ...
```

### GPIO Access (Raspberry Pi)

For GPIO access on Raspberry Pi:

```bash
docker run --privileged --network host ...
```

**Note**: `--privileged` is required for GPIO access but reduces security isolation.

### Serial Devices

For serial device access:

```bash
docker run --device=/dev/ttyUSB0 --device=/dev/ttyACM0 ...
```

## Network Configuration

### Host Network Mode

For radio hardware access, use host network mode:

```bash
docker run --network host ...
```

This allows the container to access:
- Local network interfaces
- Radio hardware directly
- IPC sockets on the host

### Bridge Network Mode

If you don't need direct hardware access:

```bash
docker run -p 5210:5210 ...  # Expose SVXLink TCP port
```

## Volume Mounts

### Configuration

Mount configuration directory as read-only:

```bash
-v $(pwd)/config:/etc/authenticated-repeater:ro
```

### Flowgraphs

Mount flowgraphs directory:

```bash
-v $(pwd)/flowgraphs:/usr/local/share/authenticated-repeater/flowgraphs:ro
```

### Logs

Mount logs directory for persistence:

```bash
-v $(pwd)/logs:/var/log/authenticated-repeater
```

## Environment Variables

### AUTHENTICATED_CONFIG

Override config file path:

```bash
-e AUTHENTICATED_CONFIG=/path/to/config.yaml
```

### PYTHONPATH

Set Python path for module imports:

```bash
-e PYTHONPATH=/app/modules/gr-linux-crypto:/usr/local/lib/python3/dist-packages
```

## Running the Container

### Start Container

```bash
docker-compose up -d
# or
docker start authenticated-repeater-control
```

### Stop Container

```bash
docker-compose down
# or
docker stop authenticated-repeater-control
```

### View Logs

```bash
docker-compose logs -f authenticated-control
# or
docker logs -f authenticated-repeater-control
```

### Execute Commands in Container

```bash
docker exec -it authenticated-repeater-control bash
```

### Check Container Status

```bash
docker ps | grep authenticated-repeater-control
docker-compose ps
```

## Troubleshooting

### Container Won't Start

**Check logs:**
```bash
docker logs authenticated-repeater-control
```

**Common issues:**
- Missing configuration file
- Permission issues with mounted volumes
- Missing hardware devices

### Module Import Errors

**Verify modules are installed:**
```bash
docker exec authenticated-repeater-control python3 -c "from gnuradio import packet_protocols; print('OK')"
```

**Check PYTHONPATH:**
```bash
docker exec authenticated-repeater-control env | grep PYTHONPATH
```

### Hardware Access Issues

**Check device permissions:**
```bash
ls -l /dev/ttyUSB* /dev/ttyACM*
```

**Add user to dialout group** (if running as non-root):
```bash
usermod -aG dialout $USER
```

### Network Issues

**Test network connectivity:**
```bash
docker exec authenticated-repeater-control ping -c 3 localhost
```

**Check port access:**
```bash
docker exec authenticated-repeater-control netstat -tuln | grep 5210
```

## Security Considerations

### Running as Non-Root

Create a Dockerfile that runs as non-root user:

```dockerfile
RUN useradd -m -u 1000 repeater && \
    chown -R repeater:repeater /app /etc/authenticated-repeater /var/log/authenticated-repeater

USER repeater
```

### Volume Permissions

Ensure proper permissions on mounted volumes:

```bash
chmod 755 config/
chmod 700 config/authorized_operators/
chmod 755 flowgraphs/
chmod 755 logs/
```

### Network Isolation

Use bridge network mode when possible (if hardware access not needed):

```bash
docker run --network bridge -p 5210:5210 ...
```

## Development Setup

### Development Dockerfile

For development with live code reloading:

```dockerfile
FROM debian:bookworm-slim

# ... (same as production Dockerfile)

# Install development tools
RUN apt-get update && apt-get install -y \
    vim \
    git \
    && rm -rf /var/lib/apt/lists/*

# Mount code as volume for live editing
VOLUME ["/app"]
```

### Development docker-compose.yml

```yaml
version: '3.8'

services:
  authenticated-control:
    build:
      context: .
      dockerfile: Dockerfile.dev
    volumes:
      - .:/app  # Mount entire project for live editing
      - ./config:/etc/authenticated-repeater:ro
      - ./logs:/var/log/authenticated-repeater
    environment:
      - PYTHONPATH=/app/modules/gr-linux-crypto:/usr/local/lib/python3/dist-packages
```

## Production Deployment

### Multi-Stage Build

Optimize image size with multi-stage build:

```dockerfile
# Build stage
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y \
    cmake build-essential g++ \
    gnuradio-dev python3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY modules/ /build/modules/

# Build modules
RUN cd modules/gr-qradiolink && \
    mkdir build && cd build && \
    cmake .. && make -j$(nproc) && make install

# ... (build other modules)

# Runtime stage
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    gnuradio-runtime python3-gnuradio \
    python3-cryptography python3-zmq python3-yaml \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/lib /usr/local/lib
COPY --from=builder /usr/local/include /usr/local/include

# ... (rest of runtime setup)
```

### Health Checks

Add health check to docker-compose.yml:

```yaml
services:
  authenticated-control:
    healthcheck:
      test: ["CMD", "python3", "-c", "import zmq; print('OK')"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s
```

## Backup and Restore

### Backup Configuration

```bash
docker cp authenticated-repeater-control:/etc/authenticated-repeater ./backup/config
```

### Backup Logs

```bash
docker cp authenticated-repeater-control:/var/log/authenticated-repeater ./backup/logs
```

### Restore Configuration

```bash
docker cp ./backup/config authenticated-repeater-control:/etc/authenticated-repeater
docker restart authenticated-repeater-control
```

## Additional Resources

- [Docker Documentation](https://docs.docker.com/)
- [Docker Compose Documentation](https://docs.docker.com/compose/)
- [GNU Radio in Docker](https://wiki.gnuradio.org/index.php/Docker)
- [Installation Guide](INSTALLATION.md)
- [Configuration Guide](CONFIGURATION.md)

