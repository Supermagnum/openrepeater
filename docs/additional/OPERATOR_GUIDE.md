# Authenticated Control Operator Guide

This guide explains how operators can generate keys, sign commands, and use the authenticated control system.

## Overview

The authenticated control system allows licensed operators to remotely control repeater settings using cryptographically-signed commands transmitted via radio. Commands are signed with Brainpool ECDSA signatures and transmitted using FX.25 framing for error correction.

## Key Concepts

### Digital Signatures

Commands are signed, not encrypted. This means:
- Command content remains in the clear (legal compliance)
- Digital signatures provide authentication
- Only operators with authorized keys can issue commands
- Commands cannot be forged or modified

### Command Format

Commands follow this format:
```
timestamp:command:signature:key_id
```

Where:
- `timestamp`: Unix timestamp (prevents replay attacks)
- `command`: The actual command (e.g., "SET_SQUELCH -120")
- `signature`: Base64-encoded ECDSA signature
- `key_id`: Identifier for the operator's key

## Generating Keys

### Step 1: Generate Key Pair

Use the `orp_keygen` utility to generate a Brainpool ECDSA key pair:

```bash
sudo /usr/local/bin/orp_keygen --curve brainpoolP256r1
```

This creates:
- `operator_private.pem`: Your private key (keep secure!)
- `operator_public.pem`: Your public key (share with repeater operator)
- `key_metadata.json`: Key metadata including fingerprint

### Step 2: Share Public Key

Send your `operator_public.pem` file to the repeater operator. They will add it to the authorized keys list.

**IMPORTANT**: Never share your private key! Only share the public key.

### Step 3: Verify Key Generation

Verify your key was generated correctly:

```bash
/usr/local/bin/orp_keymgmt verify /etc/openrepeater/keys/operator_public.pem
```

This will display the key fingerprint, which you can use to verify the key was generated correctly.

## Signing Commands

### Basic Command Signing

Sign a command using the `orp_sign_command` utility:

```bash
orp_sign_command "SET_SQUELCH -120" --private-key /path/to/private_key.pem
```

This outputs a JSON object with the signed command:

```json
{
  "timestamp": 1234567890,
  "command": "SET_SQUELCH -120",
  "signature": "base64_encoded_signature",
  "message": "1234567890:SET_SQUELCH -120"
}
```

### Using Custom Timestamp

You can specify a custom timestamp (useful for testing):

```bash
orp_sign_command "SET_SQUELCH -120" --timestamp 1234567890
```

### Raw Output Format

For direct transmission, use raw format:

```bash
orp_sign_command "SET_SQUELCH -120" --output raw
```

This outputs:
```
1234567890:SET_SQUELCH -120:base64_signature
```

## Supported Commands

### SET_SQUELCH

Set squelch threshold in dB.

**Usage:**
```
SET_SQUELCH <threshold>
```

**Example:**
```
SET_SQUELCH -120
```

### SET_TX_POWER

Set transmitter power level.

**Usage:**
```
SET_TX_POWER <power>
```

**Example:**
```
SET_TX_POWER 25
```

### ENABLE_REPEATER

Enable the repeater.

**Usage:**
```
ENABLE_REPEATER
```

### DISABLE_REPEATER

Disable the repeater.

**Usage:**
```
DISABLE_REPEATER
```

### STATUS

Query repeater status.

**Usage:**
```
STATUS
```

## Transmitting Commands

### Via Radio

1. Sign your command using `orp_sign_command`
2. Format the command for transmission (add key_id)
3. Transmit using your radio setup
4. Command will be received, verified, and executed automatically

### Using PTT Button

For manual PTT control:

1. **Prepare your command** (sign and format)
2. **Press and hold PTT button** before transmission
3. **Transmit the command** (two frames: command + signature)
4. **Release PTT button** after transmission completes

**PTT Timing:**
- Press PTT 50-100ms before starting transmission
- Hold PTT for entire transmission duration
- Keep PTT active 100-200ms after transmission ends (hang time)

For detailed PTT setup and usage, see [PTT Button Usage Guide](PTT_BUTTON_USAGE.md).

### Command Format for Transmission

The full command format for transmission is:
```
timestamp:command:signature:key_id
```

Where `key_id` is your operator identifier (e.g., your callsign).

### Example Transmission

```bash
# Sign the command
SIGNED=$(orp_sign_command "SET_SQUELCH -120" --output raw)

# Add your key_id (e.g., your callsign)
FULL_COMMAND="${SIGNED}:W1AW"

# Transmit via your radio setup
# (Implementation depends on your radio interface)
```

## Security Best Practices

### Private Key Security

1. **Never share your private key**: Only share public keys
2. **Store securely**: Keep private keys on encrypted storage
3. **Use hardware security modules**: Consider using Nitrokey or TPM for key storage
4. **Backup safely**: Backup private keys to secure, encrypted storage

### Command Security

1. **Verify before signing**: Double-check commands before signing
2. **Use timestamps**: Always use current timestamps (default behavior)
3. **Monitor logs**: Check repeater logs for command execution
4. **Report suspicious activity**: Report any unauthorized command attempts

### Key Rotation

1. **Regular rotation**: Rotate keys periodically (e.g., annually)
2. **Revoke old keys**: Remove old keys from authorized list
3. **Update promptly**: Update keys if compromised

## Troubleshooting

### Command Rejected

If your command is rejected:

1. **Check timestamp**: Ensure timestamp is within replay protection window
2. **Verify key**: Confirm your key is in the authorized keys list
3. **Check signature**: Verify signature was generated correctly
4. **Check logs**: Review repeater logs for error messages

### Key Not Found

If you get "Unknown sender" error:

1. **Verify key added**: Confirm your public key was added to authorized keys
2. **Check key_id**: Ensure you're using the correct key_id
3. **Contact operator**: Ask repeater operator to verify key configuration

### Signature Verification Failed

If signature verification fails:

1. **Check private key**: Verify you're using the correct private key
2. **Verify command**: Ensure command matches what was signed
3. **Check timestamp**: Verify timestamp is correct
4. **Regenerate if needed**: Generate new keys if key is compromised

## Examples

### Complete Workflow Example

```bash
# 1. Generate keys
sudo orp_keygen --curve brainpoolP256r1

# 2. Share public key with repeater operator
# (Send operator_public.pem file)

# 3. Sign a command
orp_sign_command "SET_SQUELCH -120" \
    --private-key /etc/openrepeater/keys/operator_private.pem \
    --output raw

# 4. Add key_id and transmit
# (Implementation depends on your setup)
```

### Batch Command Signing

```bash
#!/bin/bash
# Sign multiple commands

COMMANDS=(
    "SET_SQUELCH -120"
    "SET_TX_POWER 25"
    "ENABLE_REPEATER"
)

for cmd in "${COMMANDS[@]}"; do
    echo "Signing: $cmd"
    orp_sign_command "$cmd" --output json
    echo ""
done
```

## Legal Compliance

### Amateur Radio Regulations

- Commands remain in the clear (not encrypted)
- Digital signatures are for authentication only
- System designed to meet amateur radio regulatory requirements
- Operators must verify local regulations before deployment

### Important Notes

- Check local regulations regarding digital signatures on amateur bands
- Ensure compliance with your licensing authority
- Document command transmissions for regulatory compliance
- Maintain logs of all authenticated commands

## Support

For assistance:

- Review [CONFIGURATION.md](CONFIGURATION.md) for configuration details
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for common issues
- Contact the repeater operator for key management
- Contact OpenRepeater development team for technical support

