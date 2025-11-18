# SVXLink Control Interface

This module provides the control interface for authenticated SVXLink repeater commands.

## Overview

The SVXLink control interface enables remote, authenticated control of SVXLink repeater settings through cryptographically-signed commands. The system provides:

- Command parsing and validation
- Configuration file management with backup/rollback
- Replay protection and rate limiting
- Comprehensive audit logging
- Hardware abstraction layer

## Components

### Command Parser (`svxlink_command_parser.py`)

Parses and validates command strings with strict input validation:
- Parameter range checking
- Module name whitelisting
- Command format validation

### Configuration Manager (`svxlink_config_manager.py`)

Manages SVXLink configuration files:
- Reading/writing configuration files
- Automatic backup creation
- SVXLink reload via SIGHUP
- Service restart capability

### Control Service (`svxlink_control_service.py`)

Orchestrates command execution with safety features:
- Replay protection
- Rate limiting
- Command cooldown periods
- Audit logging

### Hardware Interface (`hardware_interface.py`)

Abstracts hardware-specific operations:
- GPIO-based control
- Hamlib integration
- Serial port commands
- Simulation mode

### Authenticated Command Handler (`authenticated_command_handler.py`)

Bridges authentication with control service:
- Signature verification
- Command parsing
- Execution orchestration

## Installation

```bash
cd scripts/authenticated_control/svxlink_control
pip install -r requirements.txt
```

For development:

```bash
pip install -r requirements-dev.txt
```

## Usage

### Basic Command Execution

```python
from svxlink_control_service import SVXLinkControlService

service = SVXLinkControlService()
success, result = service.execute("OPERATOR_ID", "SET_SQUELCH -120")
```

### With Authentication

```python
from authenticated_command_handler import AuthenticatedCommandHandler

handler = AuthenticatedCommandHandler()
success, result = handler.handle_command("timestamp:command:signature:key_id")
```

## Testing

Run all tests:

```bash
pytest -v
```

With coverage:

```bash
pytest --cov=. --cov-report=html
```

## Code Quality

### Formatting

```bash
black .
isort .
```

### Linting

```bash
flake8 .
mypy .
bandit -r . -ll
```

## Configuration

The system uses the following default paths:
- SVXLink config: `/etc/svxlink/svxlink.conf`
- Backup directory: `/var/lib/openrepeater/config_backups`
- Log directory: `/var/log/openrepeater`
- Authorized keys: `/etc/openrepeater/keys/authorized_keys.json`

## Security

- All commands are validated before execution
- Replay protection prevents command replay attacks
- Rate limiting prevents command flooding
- Audit logging records all operations
- Input sanitization prevents injection attacks

## Documentation

See also:
- [TESTING.md](TESTING.md) - Testing instructions
- [SECURITY.md](SECURITY.md) - Security considerations

