# GNU Radio Flowgraphs

This directory is where you should place your GNU Radio Companion (GRC) flowgraph files.

## Required Flowgraphs

### Transmitter Flowgraph (`tx_authenticated.grc`)

Your transmitter flowgraph should:

1. **Accept audio input** from microphone or audio source
2. **Accept command input** (text message to be signed)
3. **Sign the command** using either:
   - PKCS#11 hardware token (Nitrokey, YubiKey)
   - Kernel Keyring software keys
4. **Encode as AX.25 frames**:
   - Frame 1: Command text (ASCII, unencrypted)
   - Frame 2: Digital signature (proof of authenticity)
5. **Modulate using NFM** (Narrow Band FM)
6. **Output to SDR hardware** or audio sink

**Key Blocks:**
- `audio_source` - Microphone/audio input
- `linux_crypto_kernel_keyring_source` or PKCS#11 signer - Cryptographic signing
- `packet_protocols_ax25_encoder` - AX.25 frame encoding
- `qradiolink_mod_nbfm` - NFM modulation
- SDR sink or audio output

**Output Format:**
- Two separate AX.25 frames transmitted in sequence
- Frame 1: `timestamp:operator_callsign:command_text`
- Frame 2: Signature bytes (DER encoded)

### Receiver Flowgraph (`rx_authenticated.grc`)

Your receiver flowgraph should:

1. **Receive RF signal** from SDR hardware
2. **Demodulate NFM** signal
3. **Decode AX.25 frames** from the signal
4. **Extract command and signature frames**
5. **Send to command handler** via IPC (ZMQ, FIFO, or socket)

**Key Blocks:**
- SDR source (osmosdr, SoapySDR, etc.)
- `qradiolink_demod_nbfm` - NFM demodulation
- `packet_protocols_ax25_decoder` - AX.25 frame decoding
- Embedded Python block to extract frames and send via IPC

**IPC Output:**
- Send two-part message via ZMQ: `[command_frame, signature_frame]`
- Or write to FIFO: `[4-byte length][command_frame][4-byte length][signature_frame]`

## Example Flowgraphs

Example flowgraphs can be found in the main OpenRepeater repository:
- `examples/grc/signed-message-tx.grc` - Signed message transmitter
- `examples/grc/signed-message-rx.grc` - Signed message receiver
- `examples/grc/confirmed-command.grc` - Bidirectional command exchange

## Integration with Command Handler

The receiver flowgraph must output frames in a format compatible with `authenticated_command_handler.py`:

**ZMQ Format (Recommended):**
```python
import zmq
context = zmq.Context()
sender = context.socket(zmq.PUSH)
sender.connect("ipc:///tmp/authenticated_rx.sock")
sender.send_multipart([command_frame_bytes, signature_frame_bytes])
```

**FIFO Format:**
```python
with open("/tmp/authenticated_commands.fifo", "wb") as fifo:
    fifo.write(len(command_frame).to_bytes(4, 'big'))
    fifo.write(command_frame)
    fifo.write(len(signature_frame).to_bytes(4, 'big'))
    fifo.write(signature_frame)
```

## Testing

After placing your flowgraphs here, you can test them:

1. **Compile flowgraph:**
   ```bash
   grcc tx_authenticated.grc
   ```

2. **Run flowgraph:**
   ```bash
   python3 tx_authenticated.py
   ```

3. **Verify IPC connection:**
   - Check ZMQ socket: `ls -l /tmp/authenticated_*.sock`
   - Check FIFO: `ls -l /tmp/authenticated_commands.fifo`

## Documentation

For detailed flowgraph documentation, see:
- `docs/flowgraphs/FLOWGRAPHS.md` in the main repository
- GNU Radio documentation: https://wiki.gnuradio.org/

