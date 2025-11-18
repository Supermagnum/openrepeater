# AX.25 Authenticated Command Protocol

## Legal Compliance

**IMPORTANT**: This protocol uses two separate frames to comply with amateur radio regulations requiring all messages to be unencrypted and readable. The command/result text is transmitted in Frame 1 (readable by anyone), and the signature is transmitted in Frame 2 (proof of authenticity only).

## Frame Format

### Command Transmission (Operator → Repeater)

Commands are transmitted as **two separate AX.25 frames**:

#### Frame 1: Command Data (Unencrypted, Readable)

| Field | Size | Description |
| --- | --- | --- |
| Destination Address | 7 bytes | Repeater callsign/SSID encoded per AX.25 |
| Source Address | 7 bytes | Operator callsign/SSID |
| Control | 1 byte | `0x03` (UI frame) |
| PID | 1 byte | `0xF0` (no layer 3 - data frame) |
| Timestamp | 8 bytes | Unix timestamp (ms, big-endian) |
| Command Length | 2 bytes | Length of ASCII command |
| Command | variable | ASCII command text (unencrypted, readable) |
| Callsign Length | 1 byte | Length of operator callsign |
| Callsign | variable | ASCII callsign with SSID |
| FCS | 2 bytes | CRC-16-IBM |

**This frame contains only readable command text - no encryption, no signature.**

#### Frame 2: Signature (Proof of Authenticity)

| Field | Size | Description |
| --- | --- | --- |
| Destination Address | 7 bytes | Repeater callsign/SSID |
| Source Address | 7 bytes | Operator callsign/SSID |
| Control | 1 byte | `0x03` (UI frame) |
| PID | 1 byte | `0xF1` (signature frame identifier) |
| Timestamp | 8 bytes | Same timestamp as Frame 1 (for matching) |
| Signature Length | 2 bytes | Length of signature |
| Signature | variable | Raw ECDSA signature bytes (signs Frame 1 data) |
| FCS | 2 bytes | CRC-16-IBM |

**This frame contains only the cryptographic signature proving Frame 1 came from the operator.**

### Response Transmission (Repeater → Operator)

Responses are transmitted as **two separate AX.25 frames**:

#### Frame 1: Result Data (Unencrypted, Readable)

| Field | Size | Description |
| --- | --- | --- |
| Destination Address | 7 bytes | Operator callsign/SSID |
| Source Address | 7 bytes | Repeater callsign/SSID |
| Control | 1 byte | `0x03` (UI frame) |
| PID | 1 byte | `0xF0` (no layer 3 - data frame) |
| Timestamp | 8 bytes | Repeater timestamp in ms |
| Command Hash Length | 1 byte | Typically 32 |
| Original Command Hash | 32 bytes | SHA256 of original command |
| Success Flag | 1 byte | 0x01 success, 0x00 failure |
| Result Code | 1 byte | Enumeration (0-9) |
| Message Length | 2 bytes | Length of result message |
| Message | variable | ASCII result message (unencrypted, readable) |
| Callsign Length | 1 byte | Length of repeater callsign |
| Callsign | variable | Repeater callsign with SSID |
| FCS | 2 bytes | CRC-16-IBM |

**This frame contains only readable result text - no encryption, no signature.**

#### Frame 2: Signature (Proof of Authenticity)

| Field | Size | Description |
| --- | --- | --- |
| Destination Address | 7 bytes | Operator callsign/SSID |
| Source Address | 7 bytes | Repeater callsign/SSID |
| Control | 1 byte | `0x03` (UI frame) |
| PID | 1 byte | `0xF1` (signature frame identifier) |
| Timestamp | 8 bytes | Same timestamp as Frame 1 (for matching) |
| Signature Length | 2 bytes | Length of signature |
| Signature | variable | Raw ECDSA signature bytes (signs Frame 1 data) |
| FCS | 2 bytes | CRC-16-IBM |

**This frame contains only the cryptographic signature proving Frame 1 came from the repeater.**

## Protocol Flow

### Command Transmission Sequence

1. Operator creates command with timestamp
2. Operator signs command data (timestamp + command text)
3. Operator transmits **Frame 1** (command data, PID 0xF0)
4. Operator transmits **Frame 2** (signature, PID 0xF1) - typically with small delay

### Command Reception Sequence

1. Repeater receives **Frame 1** (command data)
   - Extracts and stores command text (readable by anyone)
   - Stores command PDU temporarily, waiting for signature
2. Repeater receives **Frame 2** (signature)
   - Matches signature frame to command frame by timestamp
   - Verifies ECDSA signature against Frame 1 data
   - If valid: Executes command, generates response
   - If invalid: Logs attempt, no radio response (silent reject)

### Response Transmission Sequence

1. Repeater generates result
2. Repeater signs result data (timestamp + result fields)
3. Repeater transmits **Frame 1** (result data, PID 0xF0)
4. Repeater transmits **Frame 2** (signature, PID 0xF1)

### Response Reception Sequence

1. Operator receives **Frame 1** (result data)
   - Extracts and displays result text (readable)
   - Stores result PDU temporarily
2. Operator receives **Frame 2** (signature)
   - Matches signature frame to result frame by timestamp
   - Verifies repeater's signature
   - If valid: Displays verified result
   - If invalid: Warns of possible spoofing

## Signature Algorithm

- Curve: Brainpool P256r1 (default), also supports P384r1/P512r1
- Hash: SHA-256
- Encoding: Raw DER signature bytes

## Timing Requirements

- Replay window: 60 seconds
- Rate limit: 10 commands per operator per minute
- Responses generated within 200 ms target

## Error Handling

- Invalid frames/logs: silently ignored
- Rate-limited or replay-detected commands return error response when possible
- Authentication failures are logged but not acknowledged

## Security

- **Nonce**: timestamp + command ensures uniqueness
- **Authentication**: ECDSA signatures verified per operator
- **Confidentiality**: Not provided (content remains in the clear for HAM compliance)
- **Integrity**: CRC + digital signatures
- **Legal Compliance**: All command and result text is unencrypted and readable
- **Signature Separation**: Signatures are in separate frames, clearly identifiable as authentication only (not encryption)

## Threat Model

- Protects against spoofed commands
- Prevents replay attacks within configured window
- Rate limiting mitigates flooding
- Does not protect against RF jamming or DoS at physical layer

