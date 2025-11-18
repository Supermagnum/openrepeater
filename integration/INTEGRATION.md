# Authenticated Repeater Control - Technical Integration Documentation

This document provides detailed technical information about how the authenticated repeater control system integrates with OpenRepeater and SVXLink.

## System Architecture

### Component Overview

```
┌─────────────────┐
│  Operator       │
│  (Nitrokey)     │
└────────┬────────┘
         │
         │ Radio Transmission
         │ (NFM + AX.25)
         ▼
┌─────────────────────────────────────┐
│  Repeater Site                       │
│                                      │
│  ┌──────────────┐                   │
│  │ SDR Hardware │                   │
│  └──────┬───────┘                   │
│         │                            │
│         ▼                            │
│  ┌─────────────────────┐            │
│  │ RX Flowgraph        │            │
│  │ (NFM Demod +        │            │
│  │  AX.25 Decode)      │            │
│  └──────┬──────────────┘            │
│         │                            │
│         │ IPC (ZMQ/FIFO/Socket)      │
│         ▼                            │
│  ┌─────────────────────┐            │
│  │ Command Handler     │            │
│  │ (Signature Verify)  │            │
│  └──────┬──────────────┘            │
│         │                            │
│         │ SVXLink Control            │
│         ▼                            │
│  ┌─────────────────────┐            │
│  │ SVXLink             │            │
│  │ (Repeater Control)  │            │
│  └─────────────────────┘            │
└─────────────────────────────────────┘
```

## Command Flow

### 1. Command Transmission

**Operator Side:**
1. Operator enters command text in transmitter flowgraph GUI
2. Flowgraph creates command frame: `timestamp:operator_callsign:command_text`
3. Command is signed using:
   - PKCS#11 hardware token (Nitrokey/YubiKey), OR
   - Kernel Keyring software keys
4. Two AX.25 frames are created:
   - Frame 1: Command text (ASCII, unencrypted)
   - Frame 2: Signature bytes (DER encoded ECDSA signature)
5. Frames are transmitted via NFM modulation

**Frame Format:**
```
Frame 1 (Command):
  ASCII: "1234567890.123:LA5MR:SET_SQUELCH -120"
  Length: Variable (typically 50-200 bytes)

Frame 2 (Signature):
  Binary: DER-encoded ECDSA signature
  Length: 64-72 bytes (depending on curve)
```

### 2. Command Reception

**Repeater Side:**
1. SDR hardware receives RF signal
2. RX flowgraph:
   - Demodulates NFM signal
   - Decodes AX.25 frames
   - Extracts command and signature frames
3. Frames are sent to command handler via IPC

### 3. Command Processing

**Command Handler:**
1. Receives two frames via IPC
2. Parses command frame to extract:
   - Timestamp
   - Operator callsign
   - Command text
3. Checks replay protection:
   - Timestamp within window (default 300 seconds)
   - Not a duplicate command
   - Rate limiting (max 10 commands/minute)
4. Verifies signature:
   - Loads operator's public key from `/etc/authenticated-repeater/authorized_operators/`
   - Verifies ECDSA signature using Brainpool curve
5. If valid, executes command via SVXLink
6. Logs operation with:
   - Timestamp
   - Operator callsign
   - Command
   - Success/failure
   - Result

### 4. Command Execution

**SVXLink Integration:**

The system supports three methods for controlling SVXLink:

#### Method A: TCP Control Port (Preferred)

SVXLink can be configured with a TCP control port that accepts commands:

**Configuration (`/etc/svxlink/svxlink.conf`):**
```ini
[ReflectorLogic]
TcpPort=5210
```

**Command Format:**
```
SET_SQUELCH -120\n
```

**Implementation:**
- Command handler connects to `localhost:5210`
- Sends command as plain text
- Receives response
- Closes connection

**Advantages:**
- Real-time command execution
- Can receive responses
- No file system modifications
- No service restart required

#### Method B: Configuration File Modification

Modify SVXLink config file and send SIGHUP to reload:

**Implementation:**
1. Parse `/etc/svxlink/svxlink.conf`
2. Modify appropriate section based on command
3. Write modified config
4. Send SIGHUP to svxlink process: `kill -HUP $(pidof svxlink)`

**Advantages:**
- Works with any SVXLink configuration
- Persistent changes

**Disadvantages:**
- Requires config file parsing
- More complex implementation
- Risk of corrupting config file

#### Method C: DTMF Command Injection

Inject DTMF commands that SVXLink already understands:

**Implementation:**
- Translate authenticated commands to DTMF sequences
- Inject into SVXLink's DTMF processing pipeline

**Advantages:**
- Uses existing SVXLink functionality
- No new interfaces needed

**Disadvantages:**
- Limited to DTMF-accessible functions
- More complex translation layer

## IPC Mechanisms

### ZMQ (ZeroMQ) - Recommended

**Configuration:**
```yaml
ipc_mechanism: zmq
zmq_rx_socket: ipc:///tmp/authenticated_rx.sock
zmq_tx_socket: ipc:///tmp/authenticated_tx.sock
```

**Flowgraph Implementation:**
```python
import zmq

context = zmq.Context()
sender = context.socket(zmq.PUSH)
sender.connect("ipc:///tmp/authenticated_rx.sock")

# Send two-part message
sender.send_multipart([
    command_frame_bytes,
    signature_frame_bytes
])
```

**Command Handler:**
- Creates PULL socket bound to `zmq_rx_socket`
- Receives two-part messages
- Processes commands
- Optionally sends acknowledgments via PUSH socket to `zmq_tx_socket`

**Advantages:**
- High performance
- Reliable message delivery
- Supports acknowledgments
- Cross-platform

**Disadvantages:**
- Requires pyzmq library
- More complex than FIFO

### FIFO (Named Pipe)

**Configuration:**
```yaml
ipc_mechanism: fifo
fifo_path: /tmp/authenticated_commands.fifo
```

**Flowgraph Implementation:**
```python
import struct

with open("/tmp/authenticated_commands.fifo", "wb") as fifo:
    # Write command frame
    fifo.write(struct.pack(">I", len(command_frame_bytes)))
    fifo.write(command_frame_bytes)
    
    # Write signature frame
    fifo.write(struct.pack(">I", len(signature_frame_bytes)))
    fifo.write(signature_frame_bytes)
    fifo.flush()
```

**Command Handler:**
- Opens FIFO for reading
- Reads 4-byte length, then frame data
- Repeats for signature frame
- Processes command

**Advantages:**
- Simple implementation
- No additional dependencies
- Works with any language

**Disadvantages:**
- Blocking I/O
- No built-in acknowledgments
- Less efficient than ZMQ

### Unix Domain Socket

**Configuration:**
```yaml
ipc_mechanism: socket
socket_path: /tmp/authenticated_commands.sock
```

**Status:** Not yet implemented

**Planned Implementation:**
- Similar to FIFO but with connection-oriented protocol
- Supports multiple concurrent connections
- More robust than FIFO

## Key Management

### Key Format

**PEM Format (Preferred):**
```
-----BEGIN PUBLIC KEY-----
...
-----END PUBLIC KEY-----
```

**GPG ASCII Format (Basic Support):**
```
-----BEGIN PGP PUBLIC KEY BLOCK-----
...
-----END PGP PUBLIC KEY BLOCK-----
```

### Key Storage

**Authorized Operators:**
- Location: `/etc/authenticated-repeater/authorized_operators/`
- Format: `CALLSIGN.pem` or `CALLSIGN.asc`
- Permissions: 644 (readable by service user)

**Repeater Keys:**
- Location: `/etc/authenticated-repeater/repeater_keys/`
- Private key: `private.pem` (0600 permissions)
- Public key: `public.pem` (644 permissions)

### Key Generation

**Brainpool ECC (Recommended):**
- `brainpoolP256r1` - 256-bit curve (battery-friendly)
- `brainpoolP384r1` - 384-bit curve (higher security)
- `brainpoolP512r1` - 512-bit curve (maximum security)

**RSA (Alternative):**
- 2048-bit minimum
- 3072-bit recommended
- 4096-bit for maximum security

## Logging

### Log Locations

**Service Logs:**
- Systemd journal: `journalctl -u authenticated-control`
- Format: Standard systemd logging

**Command Execution Log:**
- File: `/var/log/authenticated-repeater/commands.log`
- Format: JSON lines
- Example:
```json
{"timestamp": "2024-01-15T10:30:45.123456", "operator": "LA5MR", "command": "SET_SQUELCH -120", "success": true, "result": "Squelch set to -120"}
```

### Log Rotation

Configure log rotation in `/etc/logrotate.d/authenticated-repeater`:
```
/var/log/authenticated-repeater/*.log {
    daily
    rotate 30
    compress
    delaycompress
    missingok
    notifempty
    create 0644 svxlink svxlink
}
```

## Security Features

### Replay Protection

1. **Timestamp Validation:**
   - Commands must include current timestamp
   - Timestamp must be within configured window (default 300 seconds)
   - Future timestamps rejected (with 60-second clock skew allowance)

2. **Duplicate Detection:**
   - SHA-256 hash of command stored in memory
   - Duplicate commands within window rejected
   - History cleaned after window expires

3. **Rate Limiting:**
   - Maximum commands per minute per operator (default 10)
   - Prevents command flooding
   - Per-operator tracking

### Access Control

1. **Authorized Keys Only:**
   - Only operators with keys in `authorized_operators/` can execute commands
   - Unknown operators rejected immediately

2. **Signature Verification:**
   - Every command must have valid signature
   - Signature verified against operator's public key
   - Invalid signatures rejected

3. **File Permissions:**
   - Private keys: 0600 (owner read/write only)
   - Configuration: 644 (readable, writable by owner)
   - Logs: 644 (readable, writable by service user)

## Error Handling

### Invalid Signature

**Response:**
- Command rejected
- Logged as security event
- No action taken
- No acknowledgment sent (to avoid information leakage)

**Log Entry:**
```
WARNING: Invalid signature from LA5MR
```

### Unauthorized Operator

**Response:**
- Command rejected immediately (before signature verification)
- Logged as security event
- No action taken

**Log Entry:**
```
WARNING: Unauthorized operator: LA1XYZ
```

### Replay Detected

**Response:**
- Command rejected
- Logged as security event
- No action taken

**Log Entry:**
```
WARNING: Replay detected: duplicate command from LA5MR
```

### SVXLink Command Failure

**Response:**
- Command execution attempted
- Failure logged
- Error message returned (if using TCP method)
- Acknowledgment sent with failure status

**Log Entry:**
```
ERROR: SVXLink command failed: SET_SQUELCH -120 - Connection refused
```

### IPC Communication Failure

**Response:**
- Service continues running
- Error logged
- Attempts to reconnect (for ZMQ)
- Flowgraph may need restart

**Log Entry:**
```
ERROR: ZMQ receiver error: Connection refused
```

## Performance Considerations

### Command Processing Time

- **Signature Verification:** ~10-50ms (depending on curve and hardware)
- **SVXLink TCP Command:** ~50-200ms (depending on SVXLink response time)
- **Total:** Typically < 500ms end-to-end

### Resource Usage

- **Memory:** ~50-100 MB (Python service + flowgraphs)
- **CPU:** Minimal when idle, spikes during command processing
- **Disk:** Log files grow slowly (~1 MB per 1000 commands)

### Scalability

- **Concurrent Commands:** Handled sequentially (one at a time)
- **Multiple Operators:** Supported (rate limiting per operator)
- **Network-wide:** Can be deployed to multiple repeaters independently

## Troubleshooting

### Command Handler Not Receiving Commands

1. **Check IPC connection:**
   ```bash
   # ZMQ
   ls -l /tmp/authenticated_*.sock
   
   # FIFO
   ls -l /tmp/authenticated_commands.fifo
   ```

2. **Verify flowgraph is running:**
   ```bash
   ps aux | grep -i flowgraph
   ```

3. **Check flowgraph output:**
   - Verify flowgraph is sending to correct IPC endpoint
   - Check flowgraph logs/console output

4. **Test IPC manually:**
   ```bash
   # ZMQ test
   python3 -c "import zmq; ctx=zmq.Context(); s=ctx.socket(zmq.PUSH); s.connect('ipc:///tmp/authenticated_rx.sock'); s.send_multipart([b'test', b'sig'])"
   ```

### Signature Verification Always Fails

1. **Check key format:**
   ```bash
   file /etc/authenticated-repeater/authorized_operators/LA5MR.pem
   ```

2. **Verify key matches:**
   - Ensure operator is using the same key that's in authorized_operators/
   - Check for key corruption

3. **Test signature verification:**
   ```python
   from cryptography.hazmat.primitives import hashes, serialization
   from cryptography.hazmat.primitives.asymmetric import ec
   # ... test code ...
   ```

### SVXLink Not Responding

1. **Check SVXLink status:**
   ```bash
   sudo systemctl status svxlink
   ```

2. **Test TCP port:**
   ```bash
   telnet localhost 5210
   ```

3. **Check SVXLink logs:**
   ```bash
   sudo journalctl -u svxlink -f
   ```

4. **Verify configuration:**
   ```bash
   grep -i tcpport /etc/svxlink/svxlink.conf
   ```

## Future Enhancements

### Planned Features

1. **Unix Socket IPC** - Connection-oriented IPC mechanism
2. **DTMF Command Injection** - Alternative SVXLink control method
3. **Config File Modification** - Full implementation of config-based control
4. **GPG Key Import** - Full GPG key parsing and import
5. **Key Rotation** - Automated key rotation support
6. **Remote Monitoring** - Battery voltage, temperature, VSWR monitoring
7. **Network-wide Commands** - Broadcast commands to multiple repeaters

### Experimental Features

1. **Hardware Security Module** - Enhanced HSM support
2. **Quantum-resistant Cryptography** - Post-quantum algorithm support
3. **Distributed Key Management** - Centralized key server integration

## References

- [GNU Radio Documentation](https://wiki.gnuradio.org/)
- [SVXLink Documentation](https://github.com/sm0svx/svxlink/wiki)
- [OpenRepeater Documentation](https://openrepeater.com/)
- [Brainpool ECC](https://tools.ietf.org/html/rfc5639)
- [AX.25 Protocol](https://en.wikipedia.org/wiki/AX.25)

