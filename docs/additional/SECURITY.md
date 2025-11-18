# Security Considerations for Authenticated Control

This document outlines security considerations, best practices, and procedures for the authenticated control system.

## Security Model

### Authentication vs. Encryption

The system uses **digital signatures for authentication**, not encryption:
- Commands remain in the clear (legal compliance)
- Digital signatures provide authentication and integrity
- Only authorized operators can issue commands
- Commands cannot be forged or modified without detection

### Threat Model

The system is designed to protect against:
- **Command Forgery:** Unauthorized commands cannot be created
- **Command Modification:** Commands cannot be altered in transit
- **Replay Attacks:** Old commands cannot be replayed
- **Unauthorized Access:** Only operators with authorized keys can issue commands

The system does NOT protect against:
- **Eavesdropping:** Command content is visible (by design)
- **Jamming:** Radio jamming can prevent command reception
- **Physical Access:** Physical access to the repeater site

## Key Management

### Private Key Security

**Critical:** Private keys must be kept secure.

**Best Practices:**
1. **Never share private keys:** Only share public keys
2. **Secure storage:** Store private keys on encrypted storage
3. **Hardware security modules:** Use Nitrokey or TPM for key storage
4. **Access control:** Limit access to private keys
5. **Backup securely:** Backup private keys to encrypted storage

**Storage Options:**
- Encrypted filesystem
- Hardware security module (Nitrokey, TPM)
- Secure key management system
- Encrypted USB drive (for backups)

### Public Key Distribution

**Best Practices:**
1. **Verify fingerprints:** Always verify key fingerprints
2. **Secure channels:** Use secure channels for key distribution
3. **Key rotation:** Rotate keys periodically
4. **Revocation:** Maintain revocation list

### Key Rotation

**When to Rotate:**
- Annually (recommended)
- After suspected compromise
- When operator access changes
- After security incidents

**Rotation Procedure:**
1. Generate new key pair
2. Add new public key to authorized keys
3. Test new key
4. Revoke old key
5. Remove old key after grace period

## Access Control

### Authorized Keys Management

**Best Practices:**
1. **Minimal access:** Only authorize necessary operators
2. **Regular review:** Periodically review authorized keys
3. **Immediate revocation:** Revoke keys immediately when access is no longer needed
4. **Documentation:** Document all key additions and removals

### File Permissions

**Required Permissions:**
```bash
# Authorized keys file
chmod 600 /etc/openrepeater/keys/authorized_keys.json
chown root:root /etc/openrepeater/keys/authorized_keys.json

# Keys directory
chmod 700 /etc/openrepeater/keys
chown root:root /etc/openrepeater/keys

# Key management utilities
chmod 750 /usr/local/bin/orp_keymgmt
chown root:openrepeater /usr/local/bin/orp_keymgmt
```

## Replay Protection

### Timestamp Validation

The system uses timestamps to prevent replay attacks:
- Commands must have timestamps within the replay protection window
- Old timestamps are rejected
- Duplicate timestamps are rejected

**Configuration:**
- Default replay protection window: 3600 seconds (1 hour)
- Adjustable in module settings
- Balance between security and usability

### Timestamp Cache

The system maintains a cache of processed timestamps:
- Prevents duplicate command execution
- Automatically cleaned up
- Size limited to prevent memory issues

## Logging and Monitoring

### Command Logging

**What to Log:**
- All authenticated commands (if enabled)
- Failed authentication attempts
- Command execution results
- System errors

**Log Retention:**
- Retain logs for audit purposes
- Regular rotation to prevent disk fill
- Secure log storage

### Monitoring

**What to Monitor:**
- Failed authentication attempts
- Unusual command patterns
- System errors
- Performance issues

**Alerts:**
- Multiple failed authentication attempts
- Unauthorized access attempts
- System errors
- Performance degradation

## Physical Security

### Repeater Site Security

**Considerations:**
1. **Physical access control:** Limit physical access to repeater site
2. **Equipment security:** Secure equipment from tampering
3. **Network security:** Secure network connections (if applicable)
4. **Monitoring:** Monitor for physical tampering

### Key Storage at Site

**Best Practices:**
1. **Encrypted storage:** Store keys on encrypted storage
2. **Hardware security modules:** Use HSM for key storage
3. **Access control:** Limit access to key storage
4. **Backup:** Maintain secure backups off-site

## Network Security (If Applicable)

### Firewall Configuration

If using network-based command transmission:

```bash
# Allow only specific IP addresses
iptables -A INPUT -p tcp --dport 12345 -s 192.168.1.0/24 -j ACCEPT
iptables -A INPUT -p tcp --dport 12345 -j DROP
```

### VPN/Encrypted Tunnels

Consider using VPN or encrypted tunnels for network-based command transmission.

## Incident Response

### Suspected Compromise

**If private key is compromised:**
1. **Immediately revoke key:** Remove from authorized keys
2. **Generate new keys:** Create new key pair
3. **Notify operators:** Inform all operators
4. **Review logs:** Check for unauthorized commands
5. **Document incident:** Document what happened

### Unauthorized Access Attempts

**If unauthorized access is detected:**
1. **Review logs:** Identify source and pattern
2. **Block if necessary:** Block source if possible
3. **Notify authorities:** If required by regulations
4. **Document incident:** Document for future reference

### Key Loss

**If private key is lost:**
1. **Revoke key:** Remove from authorized keys
2. **Generate new keys:** Create new key pair
3. **Update authorized keys:** Add new public key
4. **Notify operators:** Inform of key change

## Compliance

### Amateur Radio Regulations

**Important:** The system is designed for legal compliance:
- Commands remain in the clear (not encrypted)
- Digital signatures are for authentication only
- System meets amateur radio regulatory requirements

**Operator Responsibilities:**
- Verify local regulations before deployment
- Ensure compliance with licensing authority
- Document command transmissions
- Maintain logs for regulatory compliance

### Data Protection

**Considerations:**
- Log data may contain sensitive information
- Protect logs from unauthorized access
- Follow data protection regulations
- Regular log cleanup

## Best Practices Summary

### For Operators

1. **Secure private keys:** Never share, store securely
2. **Verify commands:** Double-check before signing
3. **Monitor activity:** Review command execution logs
4. **Report issues:** Report security concerns immediately
5. **Regular updates:** Keep software updated

### For Repeater Operators

1. **Minimal access:** Only authorize necessary operators
2. **Regular review:** Periodically review authorized keys
3. **Secure storage:** Protect authorized keys file
4. **Monitor logs:** Regularly review authentication logs
5. **Incident response:** Have plan for security incidents

### For System Administrators

1. **Secure configuration:** Follow security best practices
2. **Regular updates:** Keep system and software updated
3. **Monitoring:** Set up monitoring and alerts
4. **Documentation:** Document configuration and changes
5. **Backup:** Regular secure backups

## Security Checklist

### Initial Setup

- [ ] Generate keys using secure method
- [ ] Store private keys securely
- [ ] Configure proper file permissions
- [ ] Set up logging and monitoring
- [ ] Test authentication and command execution
- [ ] Document configuration

### Ongoing Maintenance

- [ ] Regularly review authorized keys
- [ ] Monitor authentication logs
- [ ] Update software regularly
- [ ] Rotate keys periodically
- [ ] Review and update security procedures
- [ ] Test incident response procedures

### Incident Response

- [ ] Have incident response plan
- [ ] Know how to revoke keys
- [ ] Know how to generate new keys
- [ ] Have contact information for operators
- [ ] Know reporting procedures

## Support

For security concerns:

- Review this document
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- Contact OpenRepeater development team
- Report security issues responsibly

