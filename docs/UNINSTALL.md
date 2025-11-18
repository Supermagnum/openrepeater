# Uninstallation Guide

Complete guide for uninstalling OpenRepeater and all components.

## Quick Uninstall

### Using Makefile

```bash
# From repository root
sudo make uninstall
```

### Using Individual Components

```bash
# Uninstall authenticated control
cd scripts/authenticated_control
sudo make uninstall
```

## Detailed Uninstallation

### Step 1: Stop Services

```bash
# Stop authenticated control service
sudo systemctl stop authenticated-control 2>/dev/null || true
sudo systemctl disable authenticated-control 2>/dev/null || true

# Stop SVXLink service
sudo systemctl stop svxlink 2>/dev/null || true
```

### Step 2: Remove Python Packages

```bash
# Remove authenticated control package
sudo pip3 uninstall openrepeater-authenticated-control

# Verify removal
python3 -c "from authenticated_control import svxlink_control" 2>&1 | grep -q "No module" && echo "Removed" || echo "Still installed"
```

### Step 3: Remove Executable Scripts

```bash
# Remove key management tools
sudo rm -f /usr/local/bin/orp-keygen
sudo rm -f /usr/local/bin/orp-keymgmt
sudo rm -f /usr/local/bin/orp-sign-command

# Remove operator CLI
sudo rm -f /usr/local/bin/operator-cli

# Remove command handler
sudo rm -f /usr/local/bin/orp-command-handler
```

### Step 4: Remove Systemd Service Files

```bash
# Remove service file
sudo rm -f /etc/systemd/system/authenticated-control.service

# Reload systemd
sudo systemctl daemon-reload
```

### Step 5: Remove Configuration Files (Optional)

```bash
# Remove configuration directory
sudo rm -rf /etc/openrepeater

# Remove data directory
sudo rm -rf /var/lib/openrepeater

# Remove log directory
sudo rm -rf /var/log/openrepeater
```

**Warning**: This will delete all configuration, keys, and logs. Make backups first!

### Step 6: Remove OpenRepeater Web UI (Optional)

```bash
# Remove web UI files
sudo rm -rf /var/www/openrepeater

# Remove database (if using SQLite)
sudo rm -f /var/www/openrepeater/*.db
```

### Step 7: Remove SVXLink (Optional)

```bash
# Remove SVXLink configuration
sudo rm -rf /etc/svxlink

# Remove SVXLink service
sudo systemctl stop svxlink
sudo systemctl disable svxlink
sudo rm -f /etc/systemd/system/svxlink.service

# Remove SVXLink package (if installed via package manager)
sudo apt-get remove --purge svxlink 2>/dev/null || true
```

### Step 8: Remove GNU Radio OOT Modules (Optional)

If you installed the GNU Radio OOT modules manually:

```bash
# Remove gr-linux-crypto
cd /usr/src/gr-linux-crypto/build
sudo make uninstall 2>/dev/null || true

# Remove gr-packet-protocols
cd /usr/src/gr-packet-protocols/build
sudo make uninstall 2>/dev/null || true

# Remove gr-qradiolink
cd /usr/src/gr-qradiolink/build
sudo make uninstall 2>/dev/null || true

# Update library cache
sudo ldconfig
```

## Verification

After uninstallation, verify removal:

```bash
# Check for installed scripts
which orp-keygen operator-cli 2>&1 | grep -q "not found" && echo "Scripts removed" || echo "Scripts still present"

# Check for Python packages
python3 -c "import authenticated_control" 2>&1 | grep -q "No module" && echo "Packages removed" || echo "Packages still present"

# Check for configuration
[ ! -d /etc/openrepeater ] && echo "Config removed" || echo "Config still present"

# Check for services
systemctl list-unit-files | grep -q authenticated-control && echo "Service still present" || echo "Service removed"
```

## Complete Removal Script

For a complete automated removal:

```bash
#!/bin/bash
# Complete uninstallation script

set -e

echo "Stopping services..."
sudo systemctl stop authenticated-control 2>/dev/null || true
sudo systemctl disable authenticated-control 2>/dev/null || true

echo "Removing Python packages..."
sudo pip3 uninstall -y openrepeater-authenticated-control 2>/dev/null || true

echo "Removing scripts..."
sudo rm -f /usr/local/bin/orp-keygen
sudo rm -f /usr/local/bin/orp-keymgmt
sudo rm -f /usr/local/bin/orp-sign-command
sudo rm -f /usr/local/bin/operator-cli
sudo rm -f /usr/local/bin/orp-command-handler

echo "Removing systemd service..."
sudo rm -f /etc/systemd/system/authenticated-control.service
sudo systemctl daemon-reload

echo "Removing configuration (optional)..."
read -p "Remove configuration files? (y/N): " confirm
if [ "$confirm" = "y" ]; then
    sudo rm -rf /etc/openrepeater
    sudo rm -rf /var/lib/openrepeater
    sudo rm -rf /var/log/openrepeater
fi

echo "Uninstallation complete!"
```

## Backup Before Uninstallation

Before uninstalling, consider backing up:

```bash
# Create backup directory
mkdir -p ~/openrepeater-backup

# Backup configuration
sudo cp -r /etc/openrepeater ~/openrepeater-backup/config

# Backup data
sudo cp -r /var/lib/openrepeater ~/openrepeater-backup/data

# Backup logs
sudo cp -r /var/log/openrepeater ~/openrepeater-backup/logs

# Backup keys (IMPORTANT!)
sudo cp -r /etc/openrepeater/keys ~/openrepeater-backup/keys
sudo chmod -R 700 ~/openrepeater-backup/keys
```

## Troubleshooting

### Cannot Remove Python Package

```bash
# Force removal
sudo pip3 uninstall -y openrepeater-authenticated-control

# Check for multiple installations
pip3 list | grep openrepeater

# Remove manually
sudo rm -rf /usr/local/lib/python3.*/site-packages/authenticated_control*
```

### Service Won't Stop

```bash
# Force stop
sudo systemctl kill authenticated-control

# Remove service file anyway
sudo rm -f /etc/systemd/system/authenticated-control.service
sudo systemctl daemon-reload
```

### Files Still Present

```bash
# Find all related files
sudo find /usr -name "*authenticated*control*" 2>/dev/null
sudo find /etc -name "*openrepeater*" 2>/dev/null
sudo find /var -name "*openrepeater*" 2>/dev/null

# Remove manually
sudo rm -rf <found_paths>
```

## Reinstallation

After uninstallation, you can reinstall:

```bash
# From repository root
sudo make install

# Or from authenticated control directory
cd scripts/authenticated_control
sudo make install
```

## Support

If you encounter issues during uninstallation:

1. Check logs: `journalctl -u authenticated-control`
2. Verify file permissions
3. Check for running processes: `ps aux | grep authenticated`
4. Review this guide for missed steps

