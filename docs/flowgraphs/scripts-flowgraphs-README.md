# GNU Radio Flowgraphs for Authenticated Control

This directory contains GNU Radio Companion (GRC) flowgraph files for the OpenRepeater authenticated control system.

> **Note**: The main flowgraph examples are documented in `docs/flowgraphs/`. See [FLOWGRAPHS.md](../flowgraphs/FLOWGRAPHS.md) for detailed documentation with Mermaid diagrams.

## Flowgraphs

### authenticated_control_rx.grc

Receiver flowgraph for authenticated commands.

**Function:**
- Receives RF signal via SDR (osmosdr source)
- Demodulates 2FSK signal using gr-qradiolink
- Decodes FX.25 frames with error correction using gr-packet-protocols
- Extracts command data and passes to command handler

**Configuration:**
- RX Frequency: Configurable (default: 145.000 MHz)
- Modulation: 2FSK
- Baud Rate: Configurable (default: 1200 baud)
- SDR Device: Configurable via device_args

**Usage:**
1. Open in GNU Radio Companion
2. Configure SDR device and frequency
3. Set baud rate and modulation parameters
4. Generate and run the flowgraph

## Modern Flowgraphs (examples/grc/)

For the latest flowgraphs using NFM modulation and PKCS#11/Kernel Keyring support, see:

- **signed-message-tx.grc**: Signed message transmitter with PKCS#11/Kernel Keyring dual-mode
- **signed-message-rx.grc**: Signed message receiver with verification
- **confirmed-command.grc**: Bidirectional command exchange

> **TX hardware note**: When running the transmitter-oriented flowgraphs (`signed-message-tx.grc` and the TX portion of `confirmed-command.grc`) keep the `audio_source_0 → epy_block_1 (Audio/Data Scheduler)` path intact and replace the terminal `blocks_null_sink` with the SDR/audio hardware that feeds your repeater. Leave `blocks_throttle_0` enabled so AX.25 traffic remains rate-limited before entering the scheduler.
These flowgraphs support:
- **NFM (Narrow Band FM)** modulation
- **AX.25** framing
- **PKCS#11** hardware tokens (Nitrokey, YubiKey)
- **Kernel Keyring** software keys
- **Two-frame protocol** for legal compliance

See [FLOWGRAPHS.md](../flowgraphs/FLOWGRAPHS.md) for complete documentation with Mermaid flowgraph diagrams.

## Integration with Command Handler

The flowgraph outputs decoded command strings in the format:
```
timestamp:command:signature:key_id
```

This is passed to the command handler script (`command_handler.py`) which:
1. Verifies the timestamp (replay protection)
2. Verifies the ECDSA signature
3. Executes the command via SVXLink if valid
4. Logs the result

## Notes

- The flowgraph is designed to run continuously
- Commands are processed in real-time as they are received
- Invalid commands are logged but not executed
- The system is designed for headless operation
