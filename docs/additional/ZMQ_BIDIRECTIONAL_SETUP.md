# ZMQ Bidirectional Command Handler Setup

This guide explains how to set up and use the bidirectional ZMQ command handler system.

## Architecture

```
Radio → RX Flowgraph → ZMQ:5555 → Command Handler → SVXLink
                                              ↓
                                         ZMQ:5556 → GNU Radio params
                                              ↓
                                         ZMQ:5557 → TX Flowgraph → Radio
```

## Components

### 1. Command Handler (`authenticated_command_handler_zmq.py`)

The enhanced command handler provides:
- **ZMQ Port 5555**: Receives commands from RX flowgraph (SUB socket)
- **ZMQ Port 5556**: Sends GNU Radio parameter updates (PUB socket)
- **ZMQ Port 5557**: Sends reply messages to TX flowgraph (PUB socket)

### 2. SVXLink Interface (`svxlink_interface.py`)

Handles all SVXLink interactions:
- TCP control port commands
- Config file modification
- Service restart

### 3. Reply Formatter (`reply_formatter.py`)

Formats acknowledgment messages for transmission:
- Success replies with parameter and value
- Failure replies with error messages
- AX.25 frame formatting

## Installation

### 1. Install Dependencies

```bash
sudo apt-get install python3-zmq python3-yaml python3-cryptography
```

### 2. Copy Files

```bash
sudo cp integration/authenticated_command_handler_zmq.py /usr/local/share/authenticated-repeater/
sudo cp integration/svxlink_interface.py /usr/local/share/authenticated-repeater/
sudo cp integration/reply_formatter.py /usr/local/share/authenticated-repeater/
sudo chmod +x /usr/local/share/authenticated-repeater/*.py
```

### 3. Install Service

```bash
sudo cp integration/authenticated-command-handler-zmq.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable authenticated-command-handler-zmq.service
sudo systemctl start authenticated-command-handler-zmq.service
```

## Configuration

### Config File (`/etc/authenticated-repeater/config.yaml`)

```yaml
# ZMQ Configuration
zmq_rx_bind: tcp://*:5555      # Receive commands from RX flowgraph
zmq_param_bind: tcp://*:5556    # Send parameter updates to GNU Radio
zmq_reply_bind: tcp://*:5557    # Send replies to TX flowgraph

# SVXLink Configuration
svxlink_config: /etc/svxlink/svxlink.conf
svxlink_tcp_host: localhost
svxlink_tcp_port: 5210

# Repeater Settings
repeater_callsign: REPEATER

# Logging
service_log_file: /var/log/authenticated_repeater.log
command_log_file: /var/log/repeater_commands.log
log_level: INFO

# Security Settings
replay_protection_window: 300
max_commands_per_minute: 10
command_timeout: 30

# Key Management
authorized_keys_dir: /etc/authenticated-repeater/authorized_operators
repeater_keys_dir: /etc/authenticated-repeater/repeater_keys
```

## RX Flowgraph Modifications

### Add ZMQ PUB Block

After signature verification succeeds in the RX flowgraph:

```python
import zmq
import json
import time

# In your flowgraph Python block
context = zmq.Context()
sender = context.socket(zmq.PUB)
sender.connect("tcp://localhost:5555")

# After verifying signature and extracting command
json_command = {
    "operator": operator_callsign,
    "command": command_text,
    "timestamp": time.time()
}

json_bytes = json.dumps(json_command).encode("utf-8")
signature_bytes = signature_frame  # From signature verification

# Send two-part message: JSON command, signature
sender.send_multipart([json_bytes, signature_bytes])
```

### GNU Radio Companion Block

1. Add a **Python Block** after signature verification
2. Set the code to the above Python snippet
3. Connect to the message output of signature verification

## TX Flowgraph Modifications

### Add ZMQ SUB Block

To receive and transmit reply messages:

```python
import zmq
import json

# In your flowgraph Python block
context = zmq.Context()
receiver = context.socket(zmq.SUB)
receiver.connect("tcp://localhost:5557")
receiver.setsockopt(zmq.SUBSCRIBE, b"")  # Subscribe to all messages

# In the flowgraph loop
try:
    reply_json = receiver.recv(zmq.NOBLOCK)
    reply = json.loads(reply_json.decode("utf-8"))
    
    # Format as AX.25 frame
    destination = reply["destination"]
    message = reply["message"]
    
    # Create AX.25 frame for transmission
    ax25_frame = format_ax25_frame(destination, message)
    
    # Transmit via radio
    transmit_frame(ax25_frame)
    
except zmq.Again:
    pass  # No message available
```

### GNU Radio Companion Block

1. Add a **ZMQ SUB Source** block
   - Address: `tcp://localhost:5557`
   - Type: `JSON`

2. Add a **Python Block** to format AX.25 frames
3. Connect to your AX.25 encoder
4. Connect to your modulator and transmitter

## Supported Commands

### SET_SQUELCH

Set squelch threshold in dB.

**Format:**
```
SET_SQUELCH <threshold>
```

**Example:**
```
SET_SQUELCH -24
```

**Reply:**
```
Command successful, squelch set at -24 dB
```

### SET_POWER

Set transmitter power level.

**Format:**
```
SET_POWER <percentage>
```

**Example:**
```
SET_POWER 50
```

**Reply:**
```
Command successful, power set at 50%
```

### SET_TIMEOUT

Set transmit timeout in seconds.

**Format:**
```
SET_TIMEOUT <seconds>
```

**Example:**
```
SET_TIMEOUT 300
```

**Reply:**
```
Command successful, timeout set at 300 seconds
```

### RESTART

Restart SVXLink service.

**Format:**
```
RESTART
```

**Reply:**
```
Command successful, service set at restarted
```

## Testing

### Test Script

Use the provided test script:

```bash
python3 integration/zmq_send_test.py "SET_SQUELCH -24" LA1ABC
```

### Manual Testing with Python

```python
import zmq
import json
import time

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.connect("tcp://localhost:5555")
time.sleep(0.1)

command = {
    "operator": "LA1ABC",
    "command": "SET_SQUELCH -24",
    "timestamp": time.time()
}

socket.send_multipart([
    json.dumps(command).encode("utf-8"),
    b"dummy_signature"
])
```

### Check Logs

```bash
# Service log
tail -f /var/log/authenticated_repeater.log

# Command history
tail -f /var/log/repeater_commands.log
```

### Monitor Replies

```python
import zmq
import json

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5557")
socket.setsockopt(zmq.SUBSCRIBE, b"")

while True:
    reply = socket.recv()
    data = json.loads(reply.decode("utf-8"))
    print(f"Reply: {data['message']}")
```

## Logging

### Service Log (`/var/log/authenticated_repeater.log`)

Contains service operation messages:
- Startup/shutdown
- Connection status
- Errors and warnings

### Command Log (`/var/log/repeater_commands.log`)

Contains command execution history:
```
[2024-01-15 10:30:45] [LA1ABC] [SET_SQUELCH -24] [SUCCESS] [45ms]
[2024-01-15 10:31:12] [LA1ABC] [SET_POWER 50] [SUCCESS] [52ms]
[2024-01-15 10:32:00] [LA1ABC] [INVALID_COMMAND] [FAILURE] [12ms]
```

Format: `[timestamp] [operator] [command] [result] [execution_time_ms]`

## Error Handling

### Invalid Command Format

- Logs warning to service log
- Sends error reply: `"Command failed: Failed to parse command"`

### Unknown Command

- Logs warning to service log
- Sends error reply: `"Command failed: Unknown command: <command>"`

### Execution Failure

- Logs error to service log
- Sends failure reply with reason: `"Command failed: <error message>"`

### ZMQ Connection Loss

- Logs error to service log
- Attempts reconnection automatically
- Service continues running

## Performance

### Latency Targets

- **Command Processing**: < 50ms from receipt to execution
- **Reply Transmission**: < 100ms from execution to ZMQ send
- **Total Round Trip**: < 200ms from command to reply transmission

### Throughput

- Supports up to 10 commands per minute per operator (configurable)
- Rate limiting prevents command flooding

## Troubleshooting

### Command Handler Not Receiving Commands

1. Check service status:
   ```bash
   sudo systemctl status authenticated-command-handler-zmq
   ```

2. Check ZMQ ports:
   ```bash
   netstat -tuln | grep 5555
   netstat -tuln | grep 5556
   netstat -tuln | grep 5557
   ```

3. Check logs:
   ```bash
   tail -f /var/log/authenticated_repeater.log
   ```

### Replies Not Being Transmitted

1. Verify TX flowgraph is connected to port 5557
2. Check ZMQ connection in TX flowgraph
3. Verify reply messages are being sent (monitor port 5557)

### SVXLink Commands Failing

1. Check SVXLink TCP control port:
   ```bash
   telnet localhost 5210
   ```

2. Verify SVXLink config file permissions
3. Check SVXLink service status:
   ```bash
   sudo systemctl status svxlink
   ```

## Security Considerations

- All commands require valid cryptographic signatures
- Replay protection prevents command duplication
- Rate limiting prevents command flooding
- All commands are logged with full audit trail
- Only authorized operators can execute commands

## References

- [ZMQ Documentation](https://zeromq.org/)
- [SVXLink Command Reference](SVXLINK_COMMANDS.md)
- [GNU Radio ZMQ Blocks](https://wiki.gnuradio.org/index.php/ZMQ_Blocks)

