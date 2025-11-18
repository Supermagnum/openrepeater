# Operator Guide

## Key Generation

1. Generate Brainpool P256r1 key pair:
   ```bash
   openssl ecparam -name brainpoolP256r1 -genkey -noout -out operator_private.pem
   openssl ec -in operator_private.pem -pubout -out operator_public.pem
   ```
2. Send `operator_public.pem` to repeater administrator.
3. Protect `operator_private.pem` with file permissions (`chmod 600`).

## Sending Commands

1. Install dependencies:
   ```bash
   pip install -r ax25_protocol/requirements.txt
   ```
2. Use CLI to create two frames:
   ```bash
   python operator_cli.py \
     --callsign LA1ABC-5 \
     --key operator_private.pem \
     --repeater-key repeater_public.pem \
     --repeater LA1DEF-0 \
     --send "SET_SQUELCH -120"
   ```
3. The CLI outputs two separate frames:
   - **Frame 1**: Command data (unencrypted, readable) - transmit first
   - **Frame 2**: Signature (proof of authenticity) - transmit second
4. Transmit both frames via radio in sequence (with small delay between them).
5. **Note**: Frame 1 contains readable command text. Anyone monitoring can read it. Frame 2 contains only the signature proving you sent Frame 1.

## Receiving Responses

1. Capture **two response frames** from radio decoder:
   - **Frame 1**: Result data (unencrypted, readable)
   - **Frame 2**: Signature (proof of authenticity)
2. Pipe both frames into CLI:
   ```bash
   python operator_cli.py --listen --callsign ... < response_frame1.hex response_frame2.hex
   ```
   Or process separately:
   ```bash
   # Parse Frame 1 (result)
   python operator_cli.py --parse-result < response_frame1.hex
   
   # Parse Frame 2 (signature) and verify
   python operator_cli.py --parse-signature < response_frame2.hex
   ```
3. CLI matches frames by timestamp and verifies repeater signature.
4. If signature valid: Displays verified result.
5. If signature invalid: Warns of possible spoofing.

## Troubleshooting

- **Signature invalid**: Verify correct private key usage and that both frames were transmitted.
- **No response**: Request may be rate limited, signature failed, or Frame 2 (signature) was lost.
- **Replay detected**: Ensure system clock is accurate.
- **Frame mismatch**: Ensure Frame 1 and Frame 2 have matching timestamps.
- **Missing Frame 2**: If only Frame 1 is received, repeater will wait up to 10 seconds for Frame 2 before discarding.

## Security Best Practices

- Store private keys on encrypted media.
- Rotate keys periodically.
- Verify repeater public key fingerprint.
- Never share private key.
- **Remember**: All commands are readable by anyone monitoring the frequency. The signature only proves authenticity, it does not hide the command content.

