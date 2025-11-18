# Key Management Guide

This guide covers key creation, handling, key servers, and battery-friendly cryptography for the OpenRepeater authenticated control system. The information is based on [gr-linux-crypto](https://github.com/Supermagnum/gr-linux-crypto) documentation.

## Table of Contents

1. [Key Creation](#key-creation)
2. [Key Handling](#key-handling)
3. [Key Servers (Internet-Connected Repeaters)](#key-servers-internet-connected-repeaters)
4. [Battery-Friendly Cryptography](#battery-friendly-cryptography)
5. [Kernel Keyring Operations](#kernel-keyring-operations)
6. [Hardware Security Modules](#hardware-security-modules)

---

## Key Creation

### Creating GnuPG Keys

**Step 1: Generate your key pair**
```bash
gpg --full-generate-key
```

**Step 2: Follow the prompts:**
1. **Key type:** Choose "RSA and RSA" (recommended) or "ECC" (modern, smaller keys)
2. **Key size:** Choose 3072 or 4096 bits (higher is more secure, slower)
3. **Expiration:** Choose how long the key is valid (or "Key does not expire")
4. **Your name:** Enter your real name or callsign
5. **Email:** Enter your email address
6. **Comment:** Optional (e.g., "Amateur Radio Callsign: KG7ABC")
7. **Passphrase:** Enter a strong passphrase (this protects your private key)

**Step 3: Verify your key was created**
```bash
# List your keys
gpg --list-keys        # Shows public keys
gpg --list-secret-keys # Shows private keys
```

**Step 4: Export your public key (to share with others)**
```bash
gpg --export --armor john@example.com > my_public_key.asc
# Share this file - it's safe to share publicly
```

**Your private key stays on your computer** - never share it!

### Creating Brainpool ECC Keys

For battery-friendly and algorithm-diverse cryptography, use Brainpool elliptic curves:

```python
from gr_linux_crypto.crypto_helpers import CryptoHelpers

crypto = CryptoHelpers()

# Generate Brainpool key pair (recommended: brainpoolP256r1 for battery devices)
private_key, public_key = crypto.generate_brainpool_keypair('brainpoolP256r1')

# Serialize keys for storage
public_pem = crypto.serialize_brainpool_public_key(public_key)
private_pem = crypto.serialize_brainpool_private_key(private_key)

# Save to files
with open('operator_public.pem', 'wb') as f:
    f.write(public_pem)

with open('operator_private.pem', 'wb') as f:
    f.write(private_pem)
    os.chmod('operator_private.pem', 0o600)  # Protect private key
```

**Supported Brainpool Curves:**
- `brainpoolP256r1` - 256-bit curve (recommended for battery-powered devices)
- `brainpoolP384r1` - 384-bit curve (higher security)
- `brainpoolP512r1` - 512-bit curve (maximum security)

### Centralized Key Generation and Distribution

For organizations, teams, or groups that need to manage multiple users:

**How It Works:**

1. **Primary Key Generation (Secure System)**
   - One trusted individual generates a primary PGP key on a secure, isolated system
   - The primary key pair includes the master key and multiple subkeys
   - The primary key is kept extremely secure (never leaves the secure system)
   - This centralizes key generation under controlled, secure conditions

2. **Subkey Export and Distribution**
   - After creation, individual subkeys are exported from the secure system
   - Each subkey is securely transferred to individual Nitrokey devices or GnuPG cards
   - Each user receives their own hardware device (Nitrokey or GnuPG card) with a pre-loaded subkey
   - Users never handle the primary key generation process

**Security Benefits:**
- **Centralized Control**: Key generation happens in a controlled, secure environment
- **Simplified Onboarding**: New users receive pre-loaded hardware devices - no complex key generation process required
- **Reduced Risk**: Users don't directly handle key generation, reducing the chance of mistakes or key compromise
- **Consistent Security**: All keys are generated with the same security parameters and best practices

**Use Cases:**
- **Amateur Radio Clubs**: One trusted administrator generates keys, distributes Nitrokey devices to club members
- **Research Teams**: Centralized key management for collaborative projects
- **Emergency Services**: Pre-configured hardware devices for rapid deployment

---

## Key Handling

### GnuPG Agent Configuration

**GnuPG agent** is a background program that manages your private keys securely.

**What it does:**
- Stores your private keys in memory (encrypted)
- Caches your PIN so you don't have to enter it repeatedly
- Handles PIN entry securely
- Protects keys from being accessed by unauthorized programs

**Configuring Agent Timeout for Battery-Powered Devices:**

For battery-powered devices that may be turned on/off frequently, you can configure longer timeouts so you only need to enter the PIN when the device is first powered on:

Edit `~/.gnupg/gpg-agent.conf`:
```
# Cache PINs for specified time (in seconds)
# For a week: 7 days * 24 hours * 3600 seconds = 604800
default-cache-ttl 604800

# Maximum cache time (in seconds)
# For a month: 30 days * 24 hours * 3600 seconds = 2592000
max-cache-ttl 2592000

# Pinentry program (GUI for PIN entry)
pinentry-program /usr/bin/pinentry-gtk-2
```

After editing, reload the agent:
```bash
gpg-connect-agent reloadagent /bye
```

**Timeout values:**
- `default-cache-ttl`: Time before PIN is required again (default: 3600 = 1 hour)
- `max-cache-ttl`: Maximum time PIN can be cached (default: 7200 = 2 hours)
- For a week: `604800` seconds
- For a month: `2592000` seconds (30 days)

**Note:** Longer timeouts reduce security but improve usability for battery-powered devices. The PIN will still be required when the device is first powered on or after the timeout expires.

### PIN Protection

- Private keys are protected with a PIN (Personal Identification Number)
- PIN supports both **numbers and letters** for stronger security
- **Recommended minimum PIN length: 5 characters** (but longer is better)
- The PIN prevents unauthorized use even if someone gains access to your key storage
- Choose a strong PIN that you can remember but others can't easily guess

### Pinentry Programs

**Pinentry programs** are graphical or text-based programs that securely prompt you for your PIN.

**Common pinentry programs:**
- `pinentry-gtk-2` - Graphical dialog (recommended for desktop)
- `pinentry-qt` - Qt-based graphical dialog
- `pinentry-curses` - Terminal-based (for SSH sessions)
- `pinentry-tty` - Plain text terminal (less secure, not recommended)

**How to configure:**
Edit `~/.gnupg/gpg-agent.conf`:
```
pinentry-program /usr/bin/pinentry-gtk-2
```

### Hardware Security Device Removal

**Important Security Behavior:**

When you remove a Nitrokey device or GnuPG smart card during operation, the module automatically detects the disconnection and **immediately clears all cached key data from memory**.

**Nitrokey (Password Safe Slots):**
- The module periodically checks if the Nitrokey device is still connected (every 1000 work() function calls)
- If disconnection is detected, all cached key data is immediately and securely cleared from memory
- The block will output zeros until the device is reconnected and keys are reloaded
- **Security**: Keys stored in password safe slots are removed from the computer's memory when the device is unplugged

**Kernel Keyring:**
- The module periodically checks if the key still exists in the kernel keyring (every 1000 work() function calls)
- If the key is removed from the keyring, all cached key data is immediately and securely cleared from memory
- The block will output zeros until the key is re-added to the keyring
- **Security**: Keys are removed from the computer's memory when removed from the keyring

**GnuPG Card (OpenPGP Smart Card):**
- Private keys stored on GnuPG cards (including Nitrokey in OpenPGP mode) **never leave the card**
- GnuPG operations require the physical card to be present
- If the card is removed, subsequent cryptographic operations will fail until the card is reinserted
- **Security**: Private keys cannot be extracted from the card - they remain secure even if the card is removed

---

## Key Servers (Internet-Connected Repeaters)

### What Are Key Servers?

Keyservers are distributed databases that store and synchronize public PGP/GPG keys across the internet. They enable the web of trust by making public keys discoverable and allowing people to find and verify keys from others around the world.

**How Keyservers Work:**

1. **Distributed Network**: Keyservers form a peer-to-peer network that automatically synchronizes public keys
   - When you upload a key to one keyserver, it propagates to other keyservers in the network
   - Keyserver networks (like the SKS keyserver pool) sync updates between peers
   - This ensures keys are available from multiple locations, providing redundancy and availability

2. **Synchronization**: Keyservers continuously synchronize with each other
   - Updates to keys (new keys, revocations, signatures) are propagated across the network
   - The sync process ensures that keys uploaded to one server become available on others
   - This creates a distributed, resilient system where no single point of failure exists

3. **Public Read Access**: Anyone can search and download public keys
   - Public keys are meant to be shared - they contain no secret information
   - Keyservers allow searching by email address, key ID, or fingerprint
   - This enables the web of trust by making signed keys discoverable

4. **Privacy Considerations**: 
   - Public keys are public by design - they cannot be deleted from keyservers
   - Email addresses and user IDs in public keys are permanently associated with the key
   - Upload revocation certificates if you need to invalidate a key
   - Be aware that keyserver data is permanent and publicly accessible

5. **Keyserver Mesh Network**: 
   - Keyservers form a mesh network where they peer with each other
   - You can view a graph of the keyserver network showing synchronization relationships at: [https://spider.pgpkeys.eu/graphs/](https://spider.pgpkeys.eu/graphs/)
   - For a detailed list of SKS peer servers with status, IP addresses, versions, and connectivity information, see: [https://spider.pgpkeys.eu/sks-peers](https://spider.pgpkeys.eu/sks-peers)
   - These visualizations show which keyservers are connected and synchronizing with each other
   - The graph and peer list help understand the distributed nature of the keyserver infrastructure

### Using Keyservers

**Upload your public key to a keyserver:**
```bash
# Upload your signed key to a keyserver
gpg --send-keys YOUR_KEY_ID
```

**Search for keys on a keyserver:**
```bash
# Search for keys on a keyserver
gpg --search-keys email@example.com
```

**Refresh keys from keyservers:**
```bash
# Refresh keys from keyservers
gpg --refresh-keys
```

### Common Keyservers

- `keys.openpgp.org` - Modern keyserver with privacy features
- `pgp.mit.edu` - MIT keyserver (legacy SKS network)
- `pool.sks-keyservers.net` - Pool of SKS keyservers

### Best Practices

- Upload your public key to a keyserver after creating it
- Upload signed keys after key signing parties to contribute to the web of trust
- Use `gpg --refresh-keys` periodically to update keys you have imported
- Revoke keys immediately if compromised, then upload the revocation certificate

### Web of Trust and Key Signing Parties

OpenPGP and GnuPG use a decentralized trust model called the **web of trust** to verify the identity of key owners and establish trust in public keys.

**What is the Web of Trust?**

All OpenPGP-compliant implementations include a certificate vetting scheme to assist with verifying key ownership; its operation has been termed a **web of trust**. OpenPGP certificates (which include one or more public keys along with owner information) can be digitally signed by other users who, by that act, endorse the association of that public key with the person or entity listed in the certificate.

**How It Works:**

1. **Key Distribution**: You distribute your public key (the one you generated with `gpg --full-generate-key`)
2. **Identity Verification**: Others verify that the public key actually belongs to you (they confirm your identity in person or through other trusted means)
3. **Key Signing**: Once confident, they digitally sign your public key certificate using their own private key
4. **Trust Building**: Each signature on your key adds to the "web of trust" - others who trust the signer may also trust your key

**Key Signing Parties:**

A **key signing party** is an event at which people present their public keys to others in person. At these events:

- Participants meet face-to-face to verify each other's identity
- Each person confirms the identity of others (checking ID cards, business cards, or other proof)
- Once identity is verified, participants digitally sign each other's public key certificates
- This creates a network of trust - if Alice trusts Bob and Bob signed Charlie's key, Alice may trust Charlie's key

**Common Workflow at a Key Signing Party:**

1. **Before the Event**: Generate your key fingerprint (not the full key)
   ```bash
   # Get your key fingerprint
   gpg --fingerprint YOUR_KEY_ID
   ```
   - The fingerprint is a string of letters and numbers created by a cryptographic hash function
   - It condenses your public key down to a shorter, more manageable string
   - Write down or print your fingerprint to bring to the event
   - Example fingerprint: `ABCD 1234 EFGH 5678 90AB CDEF 1234 5678 90AB CDEF`

2. **At the Event (No Computers)**: 
   - Participants meet in person and exchange **fingerprints only** (not full public keys)
   - Write down the fingerprints of participants whose identity you verify
   - Verify each person's identity (check ID cards, government-issued identification, business cards, or other proof)
   - Confirm that the fingerprint matches the person's claimed identity
   - The absence of computers prevents attackers from swapping keys or performing other attacks during the event

3. **After the Event**:
   - Obtain the full public keys corresponding to the fingerprints you received
   - You can search for keys on keyservers using the fingerprint or email/name
   - Verify that the fingerprint of the key you downloaded matches what was exchanged at the party
   - Sign the public keys of people whose identity you verified at the event
   - Upload your signed public key to keyservers so others can benefit from the web of trust

**Why Use Fingerprints Instead of Full Keys at the Event?**

- **Security**: Prevents attackers from swapping keys on computers during the event
- **Simplicity**: Fingerprints are short strings that can be written down or printed
- **Verification**: After obtaining the full key, you can verify the fingerprint matches
- **Trust**: In-person fingerprint exchange combined with identity verification builds stronger trust

**In Amateur Radio Context:**

- Operators can meet at hamfests, club meetings, or special events
- Verify each other's callsigns and identity
- Sign each other's GnuPG keys to build a trusted network
- This helps verify digital signatures on radio transmissions and messages
- Creates cryptographic proof of callsign ownership

---

## Battery-Friendly Cryptography

For battery-powered devices (portable radios, embedded systems, mobile SDR platforms), choosing the right cryptographic algorithms can significantly impact battery life.

### Recommended: BrainpoolP256r1 + ChaCha20Poly1305

**Why This Combination is Battery-Friendly**

**1. ChaCha20Poly1305 (from gr-nacl)**
- **Software-optimized**: Designed for efficient software implementation without requiring special hardware instructions
- **ARM-friendly**: Provides excellent performance on ARM processors (common in battery-powered devices)
- **No hardware dependency**: Unlike AES, which benefits from AES-NI instructions (Intel/AMD), ChaCha20 works efficiently in pure software
- **Lower power consumption**: Software implementations consume less power than hardware-accelerated instructions that require specialized CPU features
- **High throughput**: Even without hardware acceleration, ChaCha20 achieves high encryption speeds

**2. BrainpoolP256r1 (from gr-linux-crypto)**
- **Lightweight ECC**: Smaller key sizes compared to RSA (256 bits vs. 2048+ bits for equivalent security)
- **Efficient key exchange**: ECDH operations are computationally efficient
- **Battery-conscious**: Fewer CPU cycles = less power consumption during key exchange operations

### How to Use This Combination

```python
from gr_linux_crypto.crypto_helpers import CryptoHelpers
from gnuradio import nacl

# Step 1: ECDH Key Exchange using BrainpoolP256r1 (gr-linux-crypto)
crypto = CryptoHelpers()

# Alice generates Brainpool key pair
alice_private, alice_public = crypto.generate_brainpool_keypair('brainpoolP256r1')

# Bob generates Brainpool key pair  
bob_private, bob_public = crypto.generate_brainpool_keypair('brainpoolP256r1')

# Both compute shared secret via ECDH
alice_secret = crypto.brainpool_ecdh(alice_private, bob_public)
bob_secret = crypto.brainpool_ecdh(bob_private, alice_public)
# alice_secret == bob_secret

# Step 2: Derive encryption key from shared secret using HKDF
salt = crypto.generate_random_key(16)
info = b'battery-friendly-encryption-v1'
encryption_key = crypto.derive_key_hkdf(
    alice_secret, 
    salt=salt, 
    info=info, 
    length=32  # 256-bit key for ChaCha20
)

# Step 3: Encrypt with ChaCha20Poly1305 (gr-nacl)
# Note: Consult gr-nacl documentation for exact API calls
# encrypted = nacl.encrypt_chacha20poly1305(message, encryption_key, nonce)
```

### Comparison with Alternatives

**AES (Hardware-Accelerated)**
- **Pros**: Very fast when AES-NI instructions are available
- **Cons**: 
  - Requires hardware acceleration for best performance
  - Inefficient in software-only implementations
  - Higher power consumption on devices without AES-NI (ARM, older CPUs)
  - Not ideal for battery-powered devices without hardware acceleration

**AES (Software-Only)**
- **Pros**: Widely supported
- **Cons**: Significantly slower than ChaCha20 in software, higher power consumption

**RSA Key Exchange**
- **Pros**: Widely supported
- **Cons**: 
  - Much larger key sizes (2048+ bits vs. 256 bits)
  - More computationally intensive
  - Higher power consumption
  - Not recommended for battery-powered devices

**X25519 (from gr-nacl)**
- **Alternative to Brainpool**: Also efficient for key exchange
- **Pros**: Often slightly faster than Brainpool on some platforms
- **Note**: Both X25519 and BrainpoolP256r1 are good choices; Brainpool provides algorithm diversity (non-NSA-influenced)

### Best Practices for Battery-Powered Devices

1. **Use ECDH for Key Exchange**: Prefer elliptic curve cryptography (Brainpool or X25519) over RSA
2. **Use ChaCha20 for Encryption**: Prefer ChaCha20Poly1305 over AES when hardware acceleration isn't available
3. **Minimize Key Exchanges**: Establish keys once, reuse for multiple encrypted sessions
4. **Cache Keys Securely**: Use kernel keyring (from gr-linux-crypto) to store derived keys securely
5. **Profile Your Device**: Test actual power consumption; results may vary based on CPU architecture

### When to Use Each

**Use BrainpoolP256r1 + ChaCha20Poly1305 when:**
- Running on battery-powered devices (portable radios, embedded systems, mobile platforms)
- CPU lacks AES-NI hardware acceleration (ARM processors, older x86 CPUs)
- Maximum battery life is critical
- Algorithm diversity is desired (avoiding NSA-influenced algorithms)

**Use AES-GCM when:**
- Running on modern Intel/AMD CPUs with AES-NI support
- Hardware acceleration is available and verified
- Power consumption is less of a concern (desktop/server applications)

**Note**: This combination requires both `gr-linux-crypto` (for Brainpool ECDH) and `gr-nacl` (for ChaCha20Poly1305). Both modules work together seamlessly for battery-efficient cryptography.

---

## Kernel Keyring Operations

The Linux kernel keyring provides secure key storage that is protected by the kernel, making keys harder to steal than if stored in regular files.

### Using KeyringHelper

```python
from gr_linux_crypto.python.keyring_helper import KeyringHelper

helper = KeyringHelper()

# Add a key to the keyring
key_id = helper.add_key('user', 'repeater_session_key', session_key_bytes)

# Read a key from the keyring
key_data = helper.read_key(key_id)

# Search for a key
key_id = helper.search_key('user', 'repeater_session_key')

# List all keys in keyring
keys = helper.list_keys()

# Revoke a key
helper.revoke_key(key_id)

# Unlink a key from keyring
helper.unlink_key(key_id, '@u')
```

### Convenience Functions

```python
from gr_linux_crypto.python.keyring_helper import add_user_key, get_user_key, list_user_keys

# Add a user key
key_id = add_user_key('my_key', b'key_data')

# Get a user key
key_data = get_user_key('my_key')

# List all user keys
keys = list_user_keys()
```

### Key Types

- `user` - User-defined keys (most common)
- `logon` - Logon keys
- `keyring` - Keyring container

### Keyring Locations

- `@u` - User keyring (default)
- `@s` - Session keyring
- `@p` - Process keyring

---

## Hardware Security Modules

### Nitrokey Support

**Why Nitrokey?**

**Firmware Updates Enable Future-Proof Security:**

Nitrokey devices support firmware updates, which is a significant advantage over YubiKeys and GnuPG cards that do not have this function.

**Key Benefits:**
- **Firmware Updates**: Nitrokey devices can be updated with new firmware versions
- **Access to New Ciphers**: By updating the firmware, you gain access to new cryptographic algorithms and ciphers
- **No Need for New Hardware**: Unlike YubiKeys and GnuPG cards, you don't need to purchase a new device to support new cryptographic standards
- **Future-Proof**: As new cryptographic algorithms are developed and standardized, firmware updates allow your Nitrokey to support them
- **Tamper Protection**: Nitrokeys have protection from attempts at tampering, providing additional physical security

**Comparison:**
- **Nitrokey**: Supports firmware updates - can add new ciphers via software update
- **YubiKey**: No firmware updates - requires new hardware for new cipher support
- **GnuPG Cards**: No firmware updates - fixed functionality, cannot add new ciphers

### Using Nitrokey with gr-linux-crypto

The `nitrokey_interface` block provides full Nitrokey hardware security module integration when `libnitrokey` is available at compile time.

**When libnitrokey is available:**
- `is_nitrokey_available()` → Returns `TRUE` if Nitrokey device is connected
- `is_key_loaded()` → Returns `TRUE` if key data is loaded from password safe slot
- `get_key_size()` → Returns size of loaded key data
- `load_key_from_nitrokey()` → Loads key from specified password safe slot (0-15)
- `get_available_slots()` → Returns list of slots that contain data
- `work()` → Outputs key data (repeating or single-shot based on `auto_repeat` setting)

**To use Nitrokey functionality:**
1. Install `libnitrokey-dev` package: `sudo apt-get install libnitrokey-dev` (or equivalent)
2. Ensure CMake detects libnitrokey (should happen automatically via pkg-config)
3. Rebuild the module: `cmake .. && make`
4. Connect a Nitrokey device to your system
5. Store key data in Nitrokey password safe slots (0-15) using Nitrokey App or CLI tools

**Hardware:**
- [Nitrokey Shop](https://shop.nitrokey.com/shop/category/nitrokeys-7) - Purchase Nitrokey devices

---

## References

- [gr-linux-crypto Repository](https://github.com/Supermagnum/gr-linux-crypto)
- [gr-linux-crypto README](https://github.com/Supermagnum/gr-linux-crypto/blob/master/README.md)
- [GnuPG Manual](https://www.gnupg.org/documentation/manuals/gnupg/)
- [Keyserver Network Visualization](https://spider.pgpkeys.eu/graphs/)
- [SKS Peer Servers](https://spider.pgpkeys.eu/sks-peers)

---

## Summary

This guide covers:

1. **Key Creation**: GnuPG keys, Brainpool ECC keys, centralized key generation
2. **Key Handling**: GnuPG agent configuration, PIN protection, hardware device removal
3. **Key Servers**: Distributed key servers, web of trust, key signing parties
4. **Battery-Friendly Cryptography**: BrainpoolP256r1 + ChaCha20Poly1305 combination
5. **Kernel Keyring**: Secure key storage in Linux kernel
6. **Hardware Security Modules**: Nitrokey support and firmware updates

For more detailed information, consult the [gr-linux-crypto documentation](https://github.com/Supermagnum/gr-linux-crypto).

