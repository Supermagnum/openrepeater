# SVXLink Command Reference

This document lists the commands that SVXLink accepts via its TCP control port and other control interfaces.

## TCP Control Port

SVXLink can be configured to accept commands via a TCP control port. This is configured in the `[ReflectorLogic]` section of `/etc/svxlink/svxlink.conf`:

```ini
[ReflectorLogic]
TcpPort=5210
```

Commands are sent as plain text, terminated with a newline character (`\n`).

## Standard SVXLink Commands

### Module Control Commands

#### Activate Module
```
ACTIVATE <module_name>
```
Activates a specific SVXLink module.

**Example:**
```
ACTIVATE EchoLink
```

#### Deactivate Module
```
DEACTIVATE <module_name>
```
Deactivates a specific SVXLink module.

**Example:**
```
DEACTIVATE EchoLink
```

#### Enable Module
```
ENABLE_MODULE <module_name>
```
Enables a module (if it supports enable/disable).

**Example:**
```
ENABLE_MODULE EchoLink
```

#### Disable Module
```
DISABLE_MODULE <module_name>
```
Disables a module (if it supports enable/disable).

**Example:**
```
DISABLE_MODULE EchoLink
```

### Repeater Control Commands

#### Enable Repeater
```
ENABLE_REPEATER
```
Enables the repeater (brings it online).

#### Disable Repeater
```
DISABLE_REPEATER
```
Disables the repeater (takes it offline).

#### Repeater Status
```
STATUS
```
Queries the current status of the repeater.

**Response format:**
- Status information as text

### Squelch Control

#### Set Squelch Threshold
```
SET_SQUELCH <threshold>
```
Sets the squelch threshold in dB.

**Parameters:**
- `<threshold>`: Squelch threshold in dB (typically -120 to -30)

**Example:**
```
SET_SQUELCH -120
```

**Note:** The exact command format may vary depending on SVXLink version and configuration. Some versions may use:
```
SQL_THRESHOLD <threshold>
```

### Transmitter Control

#### Set TX Power
```
SET_TX_POWER <power>
```
Sets transmitter power level (if supported by hardware).

**Parameters:**
- `<power>`: Power level (format depends on hardware)

**Example:**
```
SET_TX_POWER 25
```

#### Set TX Timeout
```
SET_TX_TIMEOUT <seconds>
```
Sets the transmit timeout in seconds.

**Parameters:**
- `<seconds>`: Timeout in seconds (typically 60-600)

**Example:**
```
SET_TX_TIMEOUT 300
```

### Identification Control

#### Manual Identification
```
IDENTIFY
```
Triggers a manual identification (same as DTMF `*` command).

#### Set Short ID Interval
```
SET_SHORT_ID_INTERVAL <minutes>
```
Sets the short identification interval.

**Parameters:**
- `<minutes>`: Interval in minutes (0 to disable)

**Example:**
```
SET_SHORT_ID_INTERVAL 10
```

#### Set Long ID Interval
```
SET_LONG_ID_INTERVAL <minutes>
```
Sets the long identification interval.

**Parameters:**
- `<minutes>`: Interval in minutes (0 to disable)

**Example:**
```
SET_LONG_ID_INTERVAL 60
```

## Module-Specific Commands

### EchoLink Module

#### Connect to Node
```
EL_CONNECT <node_id>
```
Connects to an EchoLink node by ID.

**Example:**
```
EL_CONNECT 12345
```

#### Disconnect
```
EL_DISCONNECT
```
Disconnects from the current EchoLink connection.

#### List Connected Stations
```
EL_LIST
```
Lists all connected EchoLink stations.

### Remote Relay Module

#### Relay Control
```
RELAY <relay_number> <state>
```
Controls a relay.

**Parameters:**
- `<relay_number>`: Relay number (1-8)
- `<state>`: State (0=OFF, 1=ON, 2=MOMENTARY)

**Examples:**
```
RELAY 1 1    # Turn relay 1 ON
RELAY 1 0    # Turn relay 1 OFF
RELAY 1 2    # Momentary activation of relay 1
```

#### All Relays OFF
```
RELAY_ALL_OFF
```
Turns off all relays.

#### All Relays ON
```
RELAY_ALL_ON
```
Turns on all relays.

### Rig Control Module

#### Get Frequency
```
RIG_GET_FREQ
```
Gets the current radio frequency.

#### Set Frequency
```
RIG_SET_FREQ <frequency>
```
Sets the radio frequency.

**Parameters:**
- `<frequency>`: Frequency in Hz

**Example:**
```
RIG_SET_FREQ 145000000
```

#### Get Mode
```
RIG_GET_MODE
```
Gets the current radio mode.

#### Set Mode
```
RIG_SET_MODE <mode> [passband]
```
Sets the radio mode.

**Parameters:**
- `<mode>`: Mode (LSB, USB, FM, AM, CW)
- `<passband>`: Optional passband in Hz

**Examples:**
```
RIG_SET_MODE FM 12000
RIG_SET_MODE LSB 3000
```

## Command Response Format

SVXLink typically responds to commands with:
- Success: Status message or confirmation
- Failure: Error message

Responses are plain text, terminated with newline.

**Example responses:**
```
OK
ERROR: Module not found
Squelch set to -120
```

## Implementation Notes

### Command Handler Integration

The authenticated command handler sends commands to SVXLink via TCP:

```python
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("localhost", 5210))
sock.sendall(f"{command}\n".encode("utf-8"))
response = sock.recv(4096).decode("utf-8")
sock.close()
```

### Command Format

Commands sent via TCP should:
1. Be plain ASCII text
2. End with newline (`\n`)
3. Not exceed reasonable length limits (typically 256-512 bytes)

### Error Handling

If a command fails:
- SVXLink may return an error message
- The connection may close
- The command handler should handle timeouts gracefully

## Testing Commands

### Manual Testing with netcat

```bash
# Connect to SVXLink TCP port
nc localhost 5210

# Send command
STATUS

# Receive response
# (SVXLink will respond with status)

# Exit
^C
```

### Testing with telnet

```bash
telnet localhost 5210
STATUS
```

### Testing with Python

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("localhost", 5210))
sock.sendall(b"STATUS\n")
response = sock.recv(4096).decode("utf-8")
print(response)
sock.close()
```

## Version Compatibility

**Note:** Command availability and syntax may vary between SVXLink versions:
- SVXLink 1.x: Basic command set
- SVXLink 19.x: Enhanced command set
- SVXLink 20.x+: Additional module-specific commands

Always verify command syntax with your specific SVXLink version and configuration.

## Alternative Control Methods

### DTMF Commands

SVXLink also accepts commands via DTMF tones. Common DTMF commands:
- `*`: Manual identification
- `0#`: Help
- `1#`: Module-specific commands
- `##`: Deactivate module

See SVXLink documentation for complete DTMF command reference.

### Configuration File Modification

Some settings can be changed by modifying `/etc/svxlink/svxlink.conf` and sending SIGHUP to the svxlink process:

```bash
# Edit config file
sudo nano /etc/svxlink/svxlink.conf

# Reload configuration
sudo killall -HUP svxlink
```

## References

- [SVXLink Documentation](https://github.com/sm0svx/svxlink/wiki)
- [SVXLink Configuration Guide](https://github.com/sm0svx/svxlink/wiki/ConfigFile)
- [OpenRepeater Documentation](https://openrepeater.com/)

## Command Summary Table

| Command | Description | Parameters | Example |
|---------|-------------|------------|---------|
| `ACTIVATE` | Activate module | `<module_name>` | `ACTIVATE EchoLink` |
| `DEACTIVATE` | Deactivate module | `<module_name>` | `DEACTIVATE EchoLink` |
| `ENABLE_REPEATER` | Enable repeater | None | `ENABLE_REPEATER` |
| `DISABLE_REPEATER` | Disable repeater | None | `DISABLE_REPEATER` |
| `STATUS` | Query status | None | `STATUS` |
| `SET_SQUELCH` | Set squelch threshold | `<threshold>` | `SET_SQUELCH -120` |
| `SET_TX_POWER` | Set TX power | `<power>` | `SET_TX_POWER 25` |
| `SET_TX_TIMEOUT` | Set TX timeout | `<seconds>` | `SET_TX_TIMEOUT 300` |
| `IDENTIFY` | Manual ID | None | `IDENTIFY` |
| `EL_CONNECT` | Connect EchoLink | `<node_id>` | `EL_CONNECT 12345` |
| `EL_DISCONNECT` | Disconnect EchoLink | None | `EL_DISCONNECT` |
| `RELAY` | Control relay | `<num> <state>` | `RELAY 1 1` |
| `RIG_SET_FREQ` | Set frequency | `<frequency>` | `RIG_SET_FREQ 145000000` |
| `RIG_SET_MODE` | Set mode | `<mode> [passband]` | `RIG_SET_MODE FM 12000` |

