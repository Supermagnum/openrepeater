# Repeater Administrator Guide

## Protocol Overview

This system uses a **two-frame protocol** for legal compliance with amateur radio regulations:

- **Frame 1**: Command/result data (unencrypted, readable by anyone)
- **Frame 2**: Signature (proof of authenticity only)

All command and result text is transmitted in the clear. Signatures are in separate frames to clearly distinguish authentication from encryption.

## Initial Setup

1. Generate repeater private key:
   ```bash
   openssl ecparam -name brainpoolP256r1 -genkey -noout -out repeater_private.pem
   openssl ec -in repeater_private.pem -pubout -out repeater_public.pem
   ```
2. Store private key securely (`chmod 600`) or load into kernel keyring.
3. Configure `SignatureHandler` with key paths and authorized key directory.

## Managing Operators

1. Place operator public key in `authorized_keys/LA1ABC-0.pub.pem`.
2. Run `SignatureHandler.reload_keys()` or restart service.
3. Remove operators by deleting key file and reloading.

## Rate Limits & Replay Window

- Adjust `CommandProcessor` parameters:
  ```python
  CommandProcessor(..., replay_window_seconds=90, max_commands_per_minute=5)
  ```

## Monitoring

- Review audit logs for command history.
- Use `command_processor.get_command_history()` in diagnostics.
- Watch for repeated failures indicating potential attacks.
- Monitor pending commands: Commands wait up to 10 seconds for signature frame (Frame 2).
- Check for orphaned Frame 1 commands (command received but signature never arrived).

## Frame Matching

The repeater matches command frames by timestamp:
- Frame 1 (command data) is stored temporarily
- Frame 2 (signature) must arrive within 10 seconds
- Timestamps must match exactly
- If no match: Command is silently rejected (logged but no response)

## Key Rotation

1. Generate new keys.
2. Update configuration to point to new private key.
3. Distribute new public key to operators.
4. Reload signature handler to clear caches.

## Backup & Recovery

- Backup private key and authorized keys directory.
- Store backups offline.
- Test restoration workflow periodically.

