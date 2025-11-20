# Club Key Hierarchy for Repeater Authentication

This guide describes a three-tier key management structure for amateur radio clubs managing repeater authentication systems. This hierarchical approach provides centralized control, secure key distribution, and robust disaster recovery procedures.

## Table of Contents

1. [Three-Tier Structure](#three-tier-structure)
2. [Generate Master Certification Key](#generate-master-certification-key)
3. [Backup Master Key](#backup-master-key-critical)
4. [Encrypt and Store Backups](#encrypt-and-store-backups)
5. [Physical Storage Plan](#physical-storage-plan)
6. [Creating Operator Keys](#creating-operator-keys-two-identical-nitrokeys)
7. [Club Key Management Procedures](#club-key-management-procedures)
8. [Revocation Procedure](#revocation-procedure)
9. [Emergency Recovery Scenarios](#emergency-recovery-scenarios)
10. [Documentation for Club](#documentation-for-club)
11. [Testing the Backup](#testing-the-backup)

---

## Three-Tier Structure

### Master Certification Key (offline, air-gapped computer)

- Lives on secure offline system (best practice)
- Only certifies/revokes operator keys
- Backup stored in bank safe deposit box

### Operator Keys (on Nitrokeys, issued to members)

- Each operator receives two identical Nitrokeys
- Signed by master key
- Can be revoked if lost/stolen

### Emergency Board Keys (for club officers)

- 2-3 board members have special keys
- Can access repeaters if operator unavailable
- Also signed by master key

---

## Generate Master Certification Key

Generate master key (Brainpool P-256 for DARC compatibility):

```bash
gpg --expert --full-generate-key
```

**Select:**
- (9) ECC (sign and encrypt)
- (3) Brainpool P-256
- Key valid for: 0 (no expiration, or 5y for safety)
- Real name: "LA_CLUB Repeater Master Key" (use your club callsign)
- Email: repeaters@yourclub.no
- Comment: Master certification key - keep offline

---

## Backup Master Key (CRITICAL!)

### Get the key ID

```bash
gpg --list-secret-keys
```

Note the key ID (e.g., 0x1234ABCD)

### Export EVERYTHING securely

```bash
KEY_ID="YOUR_KEY_ID_HERE"

# 1. Export private master key (MOST IMPORTANT)
gpg --armor --export-secret-keys $KEY_ID > master-private-key.asc

# 2. Export public key
gpg --armor --export $KEY_ID > master-public-key.asc

# 3. Export revocation certificate (CRITICAL for emergency)
gpg --gen-revoke --armor --output master-revocation-cert.asc $KEY_ID

# 4. Create paper backup (for bank vault)
gpg --armor --export-secret-keys $KEY_ID | paperkey --output master-paperkey.txt

# 5. Print QR codes as additional backup
qrencode -r master-private-key.asc -o master-key-qr.png
```

---

## Encrypt and Store Backups

Encrypt backup with strong passphrase for storage:

```bash
# Encrypt backup with strong passphrase for storage
gpg --symmetric --cipher-algo AES256 master-private-key.asc
# Creates: master-private-key.asc.gpg

# Copy to multiple USB drives
for drive in /media/usb1 /media/usb2 /media/usb3; do
    cp master-private-key.asc.gpg $drive/
    cp master-revocation-cert.asc $drive/
done
```

---

## Physical Storage Plan

**THREE copies minimum:**

1. **Primary**: USB drive in club safe
2. **Backup**: USB drive in bank safe deposit box
3. **Emergency**: Paper backup (paperkey) with board chairman

---

## Creating Operator Keys (Two Identical Nitrokeys)

### Generate Once, Copy to Both Nitrokeys

On the SECURE OFFLINE computer (same as master key generation):

```bash
# 1. Generate operator subkey
gpg --expert --edit-key $MASTER_KEY_ID
```

In GPG prompt:
```
gpg> addkey
# Select (9) ECC
# Select Brainpool P-256
# Set capabilities: Sign and Authenticate (for your system)
# Expiry: 1 year (renewable)
gpg> save
```

```bash
# 2. Get the subkey ID
gpg --list-keys --with-subkey-fingerprints
OPERATOR_SUBKEY_ID="subkey_id_here"

# 3. Export operator's subkeys
gpg --armor --export-secret-subkeys $OPERATOR_SUBKEY_ID > operator-LA1ABC-subkeys.asc

# 4. Insert FIRST Nitrokey
gpg --card-status

# 5. Import and move to first Nitrokey
gpg --import operator-LA1ABC-subkeys.asc
gpg --edit-key $OPERATOR_SUBKEY_ID
```

In GPG prompt:
```
gpg> key 1  # Select the subkey
gpg> keytocard
# Choose slot (Authentication key = 3)
gpg> save
```

```bash
# 6. Insert SECOND Nitrokey (backup)
# Restore the subkey from file again
gpg --import operator-LA1ABC-subkeys.asc
gpg --edit-key $OPERATOR_SUBKEY_ID
```

In GPG prompt:
```
gpg> key 1
gpg> keytocard
gpg> save
```

```bash
# 7. SECURELY DELETE the subkey file
shred -vfz -n 10 operator-LA1ABC-subkeys.asc
```

---

## Club Key Management Procedures

### Operator Enrollment Process

1. New member joins repeater operators group
2. Club secretary generates operator subkey from master
3. Two Nitrokeys programmed with identical keys
4. Member signs key usage agreement
5. Public key added to authorized_keys on all repeaters
6. Member receives:
   - Primary Nitrokey (daily use)
   - Backup Nitrokey (home safe)
   - PINs (separately, never together with keys)

### Key Expiry and Renewal

Annually, extend operator key expiry (requires master key):

```bash
gpg --edit-key $OPERATOR_KEY_ID
```

In GPG prompt:
```
gpg> key 1  # Select subkey
gpg> expire
# Extend 1 year
gpg> save
```

Re-export updated key to operator's Nitrokeys.

---

## Revocation Procedure

If operator leaves club or key compromised:

On secure system with master key:

```bash
gpg --edit-key $OPERATOR_KEY_ID
```

In GPG prompt:
```
gpg> key 1  # Select compromised subkey
gpg> revkey
# Reason: 1 = Key has been compromised
gpg> save
```

```bash
# Publish revocation to repeater systems
gpg --armor --export $OPERATOR_KEY_ID > revoked-key.asc
# Update all repeaters' authorized_keys
```

---

## Emergency Recovery Scenarios

### Scenario 1: Master Key Owner Dies

Board members retrieve backup from bank:

```bash
# Or use Shamir shares to reconstruct
# From bank vault USB:
gpg --import /media/usb/master-private-key.asc.gpg
# Enter encryption passphrase (in club documents)
```

### Scenario 2: Fire Destroys Club Safe

Retrieve backup from bank safe deposit box:

```bash
# Or use paper backup + paperkey tool
paperkey --pubring master-public-key.asc \
         --secrets master-paperkey.txt | \
         gpg --import
```

### Scenario 3: Operator Loses Both Nitrokeys

1. Immediately revoke compromised keys (use master)
2. Generate new operator subkey
3. Program two new Nitrokeys
4. Update all repeaters with new public key
5. Old key automatically invalid after revocation

---

## Documentation for Club

Create a "Key Management Handbook" with:

- Master key location map (safe, bank, etc.)
- Passphrase location
- Step-by-step recovery procedures (printed, in safe)

---

## Testing the Backup

**CRITICAL: Test recovery annually!**

Disaster recovery drill:

1. Retrieve backup from bank
2. Import on test system
3. Verify can sign test message
4. Re-secure and return to bank
5. Document any issues

---

## Summary

This three-tier key hierarchy provides:

- **Centralized Control**: Master key manages all operator keys
- **Secure Distribution**: Operator keys on hardware tokens (Nitrokey)
- **Disaster Recovery**: Multiple backup locations and formats
- **Operational Continuity**: Emergency board keys for critical situations
- **Regular Maintenance**: Annual key renewal and backup testing

For detailed instructions on generating Brainpool P256r1 keys on Nitrokey devices, see [Key Management Guide](KEY_MANAGEMENT.md).

