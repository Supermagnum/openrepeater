# Module Usage Guide

This guide explains how to use the GNU Radio out-of-tree (OOT) modules in your flowgraphs.

## Overview

The authenticated repeater control system uses three main GNU Radio modules:

1. **gr-linux-crypto** - Cryptographic operations and key management
2. **gr-packet-protocols** - AX.25 and FX.25 packet protocol support
3. **gr-qradiolink** - Digital and analog modulation blocks

## gr-linux-crypto Module

### Purpose

Provides Linux-specific cryptographic infrastructure including:
- Kernel keyring integration
- Hardware security module (HSM) support (Nitrokey, YubiKey)
- Cryptographic signing and verification
- Brainpool ECC curve support

### Key Blocks

#### kernel_keyring_source

**Purpose**: Retrieves keys from Linux kernel keyring

**Parameters**:
- `key_id`: Key identifier in kernel keyring
- `key_type`: Type of key (e.g., "user", "session")

**Usage Example**:
```
Block: kernel_keyring_source
ID: kernel_keyring_source_0
Key ID: "repeater_key_1"
Key Type: "user"
```

#### kernel_crypto_aes

**Purpose**: AES encryption/decryption using kernel crypto API

**Parameters**:
- `key_size`: Key size in bits (128, 192, or 256)
- `mode`: Encryption mode (CBC, CTR, GCM)
- `operation`: "encrypt" or "decrypt"

**Usage Example**:
```
Block: kernel_crypto_aes
ID: kernel_crypto_aes_0
Key Size: 256
Mode: "CBC"
Operation: "encrypt"
```

#### nitrokey_interface

**Purpose**: Interface to Nitrokey hardware security modules

**Parameters**:
- `slot`: PKCS#11 slot number
- `pin`: PIN for HSM access (use environment variable for security)

**Usage Example**:
```
Block: nitrokey_interface
ID: nitrokey_interface_0
Slot: 0
PIN: "${NITROKEY_PIN}"
```

### Python Helpers

The module includes Python helper classes:

```python
from gr_linux_crypto.crypto_helpers import CryptoHelpers

# Create helper instance
crypto = CryptoHelpers()

# Generate key
key = crypto.generate_random_key(32)

# Hash data
hash_value = crypto.hash_data("Hello, World!", "sha256")

# Sign data (requires private key)
signature = crypto.sign_data(data_bytes, private_key, "brainpoolP256r1")

# Verify signature (requires public key)
is_valid = crypto.verify_signature(data_bytes, signature, public_key, "brainpoolP256r1")
```

## gr-packet-protocols Module

### Purpose

Provides packet radio protocol implementations:
- AX.25 (Amateur X.25) protocol
- FX.25 (Forward Error Correction X.25)
- IL2P (Improved Layer 2 Protocol)
- KISS TNC interface

### Key Blocks

#### ax25_encoder

**Purpose**: Encodes data into AX.25 frames

**Parameters**:
- `source_callsign`: Source station callsign (e.g., "LA1ABC-5")
- `destination_callsign`: Destination station callsign (e.g., "LA1DEF-0")
- `control`: Control field value
- `pid`: Protocol identifier

**Usage Example**:
```
Block: ax25_encoder
ID: ax25_encoder_0
Source Callsign: "LA1ABC-5"
Destination Callsign: "LA1DEF-0"
Control: 0x03
PID: 0xF0
```

#### ax25_decoder

**Purpose**: Decodes AX.25 frames into data

**Parameters**:
- `check_crc`: Enable CRC checking (recommended: true)
- `verbose`: Enable verbose output for debugging

**Usage Example**:
```
Block: ax25_decoder
ID: ax25_decoder_0
Check CRC: True
Verbose: False
```

#### fx25_encoder

**Purpose**: Encodes data into FX.25 frames with forward error correction

**Parameters**:
- `source_callsign`: Source station callsign
- `destination_callsign`: Destination station callsign
- `fec_mode`: FEC mode (e.g., "RS_16_7")

**Usage Example**:
```
Block: fx25_encoder
ID: fx25_encoder_0
Source Callsign: "LA1ABC-5"
Destination Callsign: "LA1DEF-0"
FEC Mode: "RS_16_7"
```

#### fx25_decoder

**Purpose**: Decodes FX.25 frames with error correction

**Parameters**:
- `check_crc`: Enable CRC checking
- `fec_mode`: FEC mode (must match encoder)

**Usage Example**:
```
Block: fx25_decoder
ID: fx25_decoder_0
Check CRC: True
FEC Mode: "RS_16_7"
```

#### kiss_tnc

**Purpose**: KISS (Keep It Simple, Stupid) TNC interface

**Parameters**:
- `mode`: "encode" or "decode"
- `port`: Serial port path (e.g., "/dev/ttyUSB0")
- `baud_rate`: Serial port baud rate

**Usage Example**:
```
Block: kiss_tnc
ID: kiss_tnc_0
Mode: "encode"
Port: "/dev/ttyUSB0"
Baud Rate: 9600
```

### Python API

```python
from gnuradio.packet_protocols import ax25_encoder, ax25_decoder

# Create encoder
encoder = ax25_encoder(
    source_callsign="LA1ABC-5",
    destination_callsign="LA1DEF-0",
    control=0x03,
    pid=0xF0
)

# Create decoder
decoder = ax25_decoder(check_crc=True, verbose=False)
```

## gr-qradiolink Module

### Purpose

Provides digital and analog modulation blocks for radio links:
- NFM (Narrowband FM) modulation/demodulation
- Audio processing
- Signal conditioning

### Key Blocks

#### nfm_modulator

**Purpose**: Modulates baseband signal to NFM

**Parameters**:
- `samp_rate`: Sample rate in Hz
- `carrier_freq`: Carrier frequency offset in Hz
- `filter_width`: Filter width in Hz

**Usage Example**:
```
Block: nfm_modulator
ID: nfm_modulator_0
Sample Rate: 48000
Carrier Frequency: 1700
Filter Width: 8000
```

#### nfm_demodulator

**Purpose**: Demodulates NFM signal to baseband

**Parameters**:
- `samp_rate`: Sample rate in Hz
- `carrier_freq`: Carrier frequency offset in Hz
- `filter_width`: Filter width in Hz

**Usage Example**:
```
Block: nfm_demodulator
ID: nfm_demodulator_0
Sample Rate: 48000
Carrier Frequency: 1700
Filter Width: 8000
```

## Complete Flowgraph Example

### Transmitter Flowgraph

```
[Message Source] 
    ↓
[Sign Message Block] ← [Kernel Keyring Source]
    ↓
[AX.25 Encoder]
    ↓
[NFM Modulator]
    ↓
[Radio Sink]
```

### Receiver Flowgraph

```
[Radio Source]
    ↓
[NFM Demodulator]
    ↓
[AX.25 Decoder]
    ↓
[Verify Message Block] ← [Kernel Keyring Source]
    ↓
[Message Sink]
```

## Integration Tips

### 1. Sample Rate Consistency

Ensure all blocks use the same sample rate:
- Audio blocks: Typically 48000 Hz
- Radio blocks: Match your SDR hardware
- Modulation blocks: Must match audio sample rate

### 2. Buffer Sizes

Use throttle blocks to control data flow:
```
Block: blocks_throttle
ID: throttle_0
Sample Rate: 48000
```

### 3. Error Handling

Add error detection blocks:
- CRC checking in decoders
- Signature verification in crypto blocks
- Frame validation in protocol blocks

### 4. Debugging

Enable verbose output during development:
- Set `verbose=True` in decoders
- Use file sinks to capture intermediate data
- Monitor message ports for status updates

## Common Patterns

### Pattern 1: Signed Command Transmission

```
[GUI Text Input] → [Sign Block] → [AX.25 Encoder] → [NFM Modulator] → [Radio]
```

### Pattern 2: Signed Command Reception

```
[Radio] → [NFM Demodulator] → [AX.25 Decoder] → [Verify Block] → [Command Handler]
```

### Pattern 3: Bidirectional Communication

Use two separate flowgraphs:
- `tx_flowgraph.grc` for transmission
- `rx_flowgraph.grc` for reception

Or use a single flowgraph with mode switching controlled by PTT button.

## Troubleshooting

### Module Not Found

If you get "Module not found" errors:

1. Verify installation:
   ```bash
   python3 -c "from gnuradio import packet_protocols; print('OK')"
   ```

2. Check PYTHONPATH:
   ```bash
   export PYTHONPATH=/usr/local/lib/python3/dist-packages:$PYTHONPATH
   ```

3. Update library cache:
   ```bash
   sudo ldconfig
   ```

### Build Errors

If modules fail to build:

1. Check dependencies:
   ```bash
   pkg-config --modversion gnuradio-runtime
   ```

2. Clean and rebuild:
   ```bash
   cd build
   rm -rf *
   cmake ..
   make -j$(nproc)
   sudo make install
   sudo ldconfig
   ```

### Runtime Errors

If flowgraphs fail to run:

1. Check block parameters match your hardware
2. Verify sample rates are consistent
3. Check file permissions for key files
4. Review logs for specific error messages

## Additional Resources

- [GNU Radio Documentation](https://wiki.gnuradio.org/)
- [AX.25 Protocol Specification](https://www.tapr.org/pdf/AX25.2.2.pdf)
- [Flowgraph Examples](../flowgraphs/FLOWGRAPHS.md)
- [Troubleshooting Guide](TROUBLESHOOTING.md)

