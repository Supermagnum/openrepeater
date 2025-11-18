# Authenticated Control Troubleshooting Guide

This guide helps diagnose and resolve common issues with the authenticated control system.

## Common Issues

### Installation Problems

#### GNU Radio Version Too Old

**Symptoms:**
- Build errors during compilation
- Import errors when testing modules
- "GNU Radio version check failed" message

**Solution:**
1. Check current GNU Radio version:
   ```bash
   gnuradio-config-info --version
   ```
2. Upgrade GNU Radio if version < 3.10.12.0:
   ```bash
   sudo apt-get update
   sudo apt-get install gnuradio-dev gnuradio-runtime
   ```

#### Build Failures

**Symptoms:**
- `make` fails with compilation errors
- Missing header files or libraries

**Solution:**
1. Install all required dependencies:
   ```bash
   sudo apt-get install -y libkeyutils-dev gnuradio-dev cmake build-essential pkg-config python3-dev libssl-dev libsodium-dev
   ```
2. Clean build directory and rebuild:
   ```bash
   cd /usr/src/gr-*
   rm -rf build
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```
3. Check build logs for specific error messages

#### Python Import Errors

**Symptoms:**
- `ImportError: No module named 'gnuradio.linux_crypto'`
- Modules not found after installation

**Solution:**
1. Verify modules are installed:
   ```bash
   find /usr/local -name "*linux_crypto*" -o -name "*packet_protocols*" -o -name "*qradiolink*"
   ```
2. Update library cache:
   ```bash
   sudo ldconfig
   ```
3. Check Python path:
   ```bash
   python3 -c "import sys; print(sys.path)"
   ```
4. Rebuild and reinstall problematic module

### Configuration Problems

#### Authorized Keys Not Loading

**Symptoms:**
- "Unknown sender" errors
- Commands rejected even with valid signatures

**Solution:**
1. Verify keys file exists and is readable:
   ```bash
   ls -l /etc/openrepeater/keys/authorized_keys.json
   ```
2. Check file format (should be valid JSON):
   ```bash
   python3 -m json.tool /etc/openrepeater/keys/authorized_keys.json
   ```
3. Verify key is active:
   ```bash
   orp_keymgmt list
   ```
4. Check file permissions:
   ```bash
   chmod 600 /etc/openrepeater/keys/authorized_keys.json
   ```

#### Command Verification Failing

**Symptoms:**
- "Invalid signature" errors
- Commands rejected with valid keys

**Solution:**
1. Verify public key matches private key:
   ```bash
   orp_keymgmt verify /path/to/public_key.pem
   ```
2. Check timestamp is within replay protection window
3. Verify command format is correct:
   ```
   timestamp:command:signature:key_id
   ```
4. Regenerate keys if necessary

### Runtime Problems

#### Command Handler Not Running

**Symptoms:**
- Commands not being processed
- No log entries

**Solution:**
1. Check if process is running:
   ```bash
   ps aux | grep command_handler
   ```
2. Check systemd service status:
   ```bash
   systemctl status authenticated-control
   ```
3. Start service if not running:
   ```bash
   sudo systemctl start authenticated-control
   ```
4. Check logs for errors:
   ```bash
   journalctl -u authenticated-control -n 50
   ```

#### Flowgraph Not Receiving

**Symptoms:**
- No commands received
- No signal detected

**Solution:**
1. Verify SDR device is connected:
   ```bash
   lsusb | grep -i rtl
   ```
2. Check device permissions:
   ```bash
   ls -l /dev/bus/usb/
   ```
3. Test SDR with other software (e.g., `rtl_test`)
4. Verify frequency configuration matches transmitter
5. Check signal strength and quality

#### Commands Not Executing

**Symptoms:**
- Commands verified but not executed
- SVXLink not responding

**Solution:**
1. Verify SVXLink is running:
   ```bash
   systemctl status svxlink
   ```
2. Check SVXLink logs:
   ```bash
   journalctl -u svxlink -n 50
   ```
3. Test SVXLink interface manually
4. Verify command handler has permissions to interface with SVXLink

### Security Issues

#### Replay Attacks

**Symptoms:**
- Old commands being replayed
- Timestamp validation failing

**Solution:**
1. Check replay protection window setting
2. Verify timestamp validation is working
3. Review processed timestamps cache
4. Adjust replay protection window if needed

#### Unauthorized Access Attempts

**Symptoms:**
- Failed authentication attempts in logs
- Unknown key IDs attempting commands

**Solution:**
1. Review failed attempt logs:
   ```bash
   grep "FAILED" /var/log/openrepeater/authenticated_control.log
   ```
2. Verify authorized keys list is secure
3. Check for compromised keys
4. Rotate keys if necessary

## Diagnostic Commands

### Check System Status

```bash
# Check GNU Radio modules
python3 -c "from gnuradio import qradiolink, linux_crypto; import gnuradio.packet_protocols; print('All modules OK')"

# Check key management utilities
which orp_keygen orp_keymgmt orp_sign_command

# Check authorized keys
orp_keymgmt list

# Check command handler
systemctl status authenticated-control

# Check logs
tail -f /var/log/openrepeater/authenticated_control.log
```

### Test Key Generation

```bash
# Generate test keys
mkdir -p /tmp/test_keys
orp_keygen --output-dir /tmp/test_keys --curve brainpoolP256r1

# Verify keys
orp_keymgmt verify /tmp/test_keys/operator_public.pem
```

### Test Command Signing

```bash
# Sign a test command
orp_sign_command "STATUS" --private-key /tmp/test_keys/operator_private.pem
```

### Test Command Verification

```bash
# Add test key
sudo orp_keymgmt add TEST /tmp/test_keys/operator_public.pem --description "Test"

# Test command handler
python3 /usr/local/bin/authenticated_control/command_handler.py \
    --authorized-keys /etc/openrepeater/keys/authorized_keys.json \
    "$(orp_sign_command "STATUS" --output raw):TEST"
```

## Log Analysis

### Command Handler Logs

Location: `/var/log/openrepeater/authenticated_control.log`

**Successful command:**
```
2024-01-01 12:00:00 - AuthenticatedCommandHandler - INFO - Command executed: SET_SQUELCH -120
```

**Failed authentication:**
```
2024-01-01 12:00:00 - AuthenticatedCommandHandler - WARNING - Command verification failed: Invalid signature
```

**Replay attack:**
```
2024-01-01 12:00:00 - AuthenticatedCommandHandler - WARNING - Timestamp outside replay protection window
```

### System Logs

Check system logs for related errors:

```bash
# View recent system logs
journalctl -n 100

# Filter for authenticated control
journalctl | grep -i "authenticated"

# View SVXLink logs
journalctl -u svxlink -n 50
```

## Performance Issues

### High CPU Usage

**Symptoms:**
- System slow or unresponsive
- High CPU usage from command handler

**Solution:**
1. Check command handler process:
   ```bash
   top -p $(pgrep -f command_handler)
   ```
2. Reduce command processing rate
3. Optimize signature verification
4. Check for command flooding

### Memory Issues

**Symptoms:**
- Out of memory errors
- System crashes

**Solution:**
1. Check memory usage:
   ```bash
   free -h
   ```
2. Clean up processed timestamps cache
3. Reduce replay protection window
4. Limit number of authorized keys

## Module Usage Issues

### Module Not Found

**Symptoms:**
- Import errors when running flowgraphs
- "No module named 'gnuradio.packet_protocols'" errors

**Solution:**
1. Verify modules are installed:
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

4. Reinstall modules if needed:
   ```bash
   cd /usr/src/gr-packet-protocols/build
   sudo make install
   sudo ldconfig
   ```

### Module Import Errors

**Symptoms:**
- "Failed to import linux_crypto_python module" errors
- Generic type errors during import

**Solution:**
1. Rebuild the module:
   ```bash
   cd /usr/src/gr-linux-crypto/build
   rm -rf *
   cmake ..
   make -j$(nproc)
   sudo make install
   sudo ldconfig
   ```

2. Check GNU Radio version compatibility:
   ```bash
   gnuradio-config-info --version
   ```
   Must be >= 3.10.12.0

3. Verify dependencies:
   ```bash
   pkg-config --modversion gnuradio-runtime
   ```

For detailed module usage, see [Module Usage Guide](MODULE_USAGE.md).

## PTT Button Issues

### PTT Not Activating

**Symptoms:**
- Radio does not key when button is pressed
- No transmission occurs

**Solution:**
1. Check GPIO pin configuration:
   ```bash
   # Test GPIO manually
   echo 18 > /sys/class/gpio/export
   echo out > /sys/class/gpio/gpio18/direction
   echo 1 > /sys/class/gpio/gpio18/value  # PTT ON
   echo 0 > /sys/class/gpio/gpio18/value  # PTT OFF
   ```

2. Verify wiring and connections
3. Check radio PTT input polarity (active high vs active low)
4. Verify user permissions (may need `sudo` or add to `gpio` group)

### PTT Stuck On

**Symptoms:**
- Radio remains keyed after transmission
- Cannot receive signals

**Solution:**
1. Manually reset GPIO:
   ```bash
   echo 0 > /sys/class/gpio/gpio18/value
   ```

2. Restart flowgraph
3. Check for software crashes
4. Verify hardware connections

### Timing Issues

**Symptoms:**
- Audio cut off at start of transmission
- Audio cut off at end of transmission

**Solution:**
1. Increase TX delay (pre-key time)
2. Increase hang time (post-key time)
3. Check radio specifications for timing requirements
4. Adjust timing values in PTT control block

For detailed PTT setup, see [PTT Button Usage Guide](PTT_BUTTON_USAGE.md).

## Testing Without Radio Hardware

### Simulating Radio Link with Audio Cables

It is possible to simulate a radio link between two Raspberry Pis or laptops using two cheap audio cables, one from mic input to speaker, one from speaker to mic. One only needs to have the needed sound interfaces. Noise can be generated in GNU Radio.

**Setup:**

1. **Hardware Requirements:**
   - Two devices (Raspberry Pis or laptops) with audio input/output
   - Two audio cables:
     - Cable 1: Connect Device A's microphone input to Device B's speaker/headphone output
     - Cable 2: Connect Device B's microphone input to Device A's speaker/headphone output

2. **Software Configuration:**
   - Configure GNU Radio flowgraphs to use audio interfaces instead of SDR hardware
   - Set audio source/sink blocks to use the appropriate audio devices
   - Use GNU Radio noise generation blocks to simulate radio noise and interference

3. **Testing Workflow:**
   - Device A: Run transmitter flowgraph (signed-message-tx.grc) using audio output
   - Device B: Run receiver flowgraph (signed-message-rx.grc) using audio input
   - Commands transmitted from Device A will be received by Device B via the audio cable link
   - This allows full end-to-end testing of the authenticated control system without radio hardware

**Benefits:**
- Test the complete system without requiring radio hardware or licenses
- Debug protocol issues in a controlled environment
- Add controlled noise and interference using GNU Radio blocks
- Verify command signing, verification, and execution without RF concerns

**Limitations:**
- Does not test actual RF propagation characteristics
- Audio bandwidth limitations may affect high-speed packet transmission
- Cable length and quality may introduce signal degradation

## Getting Help

### Information to Provide

When seeking help, provide:

1. **System Information:**
   - OS version and architecture
   - GNU Radio version
   - Hardware (SDR device, etc.)

2. **Error Messages:**
   - Full error output
   - Relevant log entries

3. **Configuration:**
   - Module settings
   - Authorized keys configuration
   - Flowgraph settings

4. **Steps to Reproduce:**
   - What you were doing
   - Expected behavior
   - Actual behavior

### Resources

- [Installation Guide](INSTALLATION.md)
- [Configuration Guide](CONFIGURATION.md)
- [Operator Guide](OPERATOR_GUIDE.md)
- [Security Considerations](SECURITY.md)
- OpenRepeater GitHub Issues
- OpenRepeater Development Team

## Prevention

### Regular Maintenance

1. **Monitor Logs:** Regularly check logs for errors or suspicious activity
2. **Update Software:** Keep GNU Radio and modules updated
3. **Rotate Keys:** Periodically rotate operator keys
4. **Review Access:** Regularly review authorized keys list
5. **Test System:** Periodically test command execution

### Best Practices

1. **Secure Storage:** Keep private keys secure
2. **Access Control:** Limit access to key management
3. **Monitoring:** Set up monitoring and alerts
4. **Documentation:** Document configuration changes
5. **Backup:** Regularly backup authorized keys and configuration

