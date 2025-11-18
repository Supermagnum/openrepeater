# Security Considerations

This document outlines security considerations for the SVXLink control interface.

## Security Model

The system implements multiple layers of security:

1. **Authentication**: Cryptographic signature verification
2. **Authorization**: Authorized keys management
3. **Input Validation**: Strict parameter validation
4. **Replay Protection**: Timestamp-based replay prevention
5. **Rate Limiting**: Command rate restrictions
6. **Audit Logging**: Comprehensive operation logging

## Input Validation

### Command Parsing

- All commands are parsed with strict validation
- Parameter ranges are enforced
- Module names are whitelisted
- Unknown commands are rejected

### Parameter Validation

- Numeric parameters are validated against safe ranges
- String inputs are sanitized
- No shell command injection possible
- No path traversal vulnerabilities

### Example Validation

```python
# Squelch level must be in range [-130, -50]
if not (-130 <= level <= -50):
    return False, "Squelch level out of range"

# Module names must be in whitelist
if module_name not in VALID_MODULES:
    return False, "Invalid module name"
```

## Subprocess Security

### Safe Subprocess Usage

- Never use `shell=True`
- Always use absolute paths for system commands
- Validate all inputs before passing to subprocess
- Use timeout to prevent hanging

### Example

```python
# Safe subprocess call
subprocess.run(
    ["/usr/bin/systemctl", "restart", "svxlink"],
    timeout=30,
    check=False
)

# NEVER do this:
# subprocess.run(f"systemctl restart {user_input}", shell=True)
```

## File Operations

### Path Validation

- All file paths are validated
- Directory traversal is prevented
- Only specific directories are accessible
- File permissions are checked

### Backup Security

- Backups are stored in restricted directory
- Backup files have appropriate permissions
- Backup directory is not web-accessible

## Replay Protection

### Timestamp Validation

- Commands must have recent timestamps
- Duplicate timestamps are rejected
- Old timestamps are rejected
- Timestamp cache is cleaned regularly

### Implementation

```python
def _check_replay_protection(self, operator_id, timestamp):
    current_time = time.time()
    time_diff = abs(current_time - timestamp)
    
    if time_diff > self.replay_window:
        return False, "Timestamp outside replay window"
    
    if timestamp in self.processed_timestamps[operator_id]:
        return False, "Duplicate timestamp"
    
    return True, ""
```

## Rate Limiting

### Protection Against Flooding

- Maximum commands per minute per operator
- Maximum commands per hour per operator
- Automatic lockout after failed attempts
- Cooldown periods for critical commands

### Implementation

```python
def _check_rate_limit(self, operator_id):
    recent_commands = sum(
        1 for t in self.command_times[operator_id]
        if t > time.time() - 60
    )
    
    if recent_commands >= self.max_commands_per_minute:
        return False, "Rate limit exceeded"
    
    return True, ""
```

## Audit Logging

### What is Logged

- All command attempts (successful and failed)
- Operator identifiers
- Timestamps
- Command parameters
- Execution results
- Error messages

### Log Security

- Logs are stored in restricted directory
- Log files have appropriate permissions
- Sensitive data is sanitized before logging
- Log rotation prevents disk exhaustion

## Access Control

### File Permissions

```bash
# Configuration files
chmod 600 /etc/svxlink/svxlink.conf

# Backup directory
chmod 700 /var/lib/openrepeater/config_backups

# Log directory
chmod 750 /var/log/openrepeater

# Authorized keys
chmod 600 /etc/openrepeater/keys/authorized_keys.json
```

### Process Permissions

- Run with minimal required privileges
- Use sudo only when necessary
- Separate user for service execution
- SELinux/AppArmor policies recommended

## Key Management

### Private Keys

- Never stored on repeater system
- Only public keys in authorized_keys.json
- Keys are base64-encoded in storage
- Key fingerprints are verified

### Key Rotation

- Regular key rotation recommended
- Immediate revocation on compromise
- Grace period for key updates
- Backup keys for recovery

## Error Handling

### Information Disclosure

- Error messages don't reveal system internals
- No stack traces in production logs
- Generic messages for authentication failures
- Detailed logs only for debugging

### Example

```python
# Good: Generic error
return False, "Authentication failed"

# Bad: Reveals internal details
return False, f"Signature verification failed: {crypto_error_details}"
```

## Dependency Security

### Regular Updates

- Keep dependencies updated
- Monitor security advisories
- Use pinned versions in production
- Test updates before deployment

### Vulnerability Scanning

```bash
# Scan for known vulnerabilities
bandit -r . -ll
pip-audit
```

## Deployment Security

### Production Checklist

- [ ] File permissions configured correctly
- [ ] Log directory secured
- [ ] Backup directory secured
- [ ] Authorized keys file secured
- [ ] Service runs with minimal privileges
- [ ] SELinux/AppArmor policies configured
- [ ] Firewall rules configured
- [ ] Regular security updates scheduled
- [ ] Audit logging enabled
- [ ] Rate limiting configured appropriately

### Monitoring

- Monitor failed authentication attempts
- Alert on repeated failures
- Review audit logs regularly
- Track command execution patterns

## Incident Response

### Suspected Compromise

1. Immediately revoke affected keys
2. Review audit logs for unauthorized access
3. Check for configuration changes
4. Restore from known-good backup if needed
5. Document incident for analysis

### Recovery

1. Generate new keys for all operators
2. Update authorized keys file
3. Verify configuration integrity
4. Test system functionality
5. Monitor for continued issues

## Best Practices

1. **Principle of Least Privilege**: Run with minimal permissions
2. **Defense in Depth**: Multiple security layers
3. **Fail Secure**: Reject by default
4. **Input Validation**: Validate everything
5. **Audit Everything**: Log all operations
6. **Regular Updates**: Keep software current
7. **Monitor Continuously**: Watch for anomalies

## Reporting Security Issues

If you discover a security vulnerability:

1. Do not open a public issue
2. Contact the development team privately
3. Provide detailed information
4. Allow time for fix before disclosure
5. Follow responsible disclosure practices

