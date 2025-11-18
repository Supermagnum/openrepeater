#!/bin/bash
################################################################################
# Uninstallation Script for Authenticated Repeater Control
#
# This script removes the authenticated control system and all its components.
#
# Usage: sudo ./uninstall_authenticated_modules.sh
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

################################################################################
# Check prerequisites
################################################################################

if [[ $EUID -ne 0 ]]; then
    echo "ERROR: This script must be run as root"
    exit 1
fi

################################################################################
# Confirmation prompt
################################################################################

echo "================================================================"
echo " Authenticated Repeater Control System Uninstallation"
echo "================================================================"
echo ""
echo "This will remove:"
echo "  - Authenticated control service"
echo "  - GNU Radio OOT modules (gr-linux-crypto, gr-packet-protocols, gr-qradiolink)"
echo "  - Configuration files (optional)"
echo "  - Log files (optional)"
echo "  - Authorized keys (optional)"
echo ""
read -r -p "Continue with uninstallation? (y/N): " confirm
if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Uninstallation cancelled."
    exit 0
fi

################################################################################
# Step 1: Stop and disable services
################################################################################

echo ""
echo "================================================================"
echo " Stopping Services"
echo "================================================================"
echo ""

# Stop authenticated control service
if systemctl is-active --quiet authenticated-control 2>/dev/null; then
    echo "Stopping authenticated-control service..."
    systemctl stop authenticated-control
    echo "Service stopped."
else
    echo "Service not running."
fi

# Disable service
if systemctl is-enabled --quiet authenticated-control 2>/dev/null; then
    echo "Disabling authenticated-control service..."
    systemctl disable authenticated-control
    echo "Service disabled."
else
    echo "Service not enabled."
fi

################################################################################
# Step 2: Remove systemd service file
################################################################################

echo ""
echo "================================================================"
echo " Removing Systemd Service"
echo "================================================================"
echo ""

if [ -f "/etc/systemd/system/authenticated-control.service" ]; then
    echo "Removing service file..."
    rm -f /etc/systemd/system/authenticated-control.service
    systemctl daemon-reload
    echo "Service file removed."
else
    echo "Service file not found."
fi

################################################################################
# Step 3: Remove GNU Radio OOT modules
################################################################################

echo ""
echo "================================================================"
echo " Removing GNU Radio OOT Modules"
echo "================================================================"
echo ""

# Remove gr-linux-crypto
if [ -d "/usr/src/gr-linux-crypto/build" ]; then
    echo "Removing gr-linux-crypto..."
    cd /usr/src/gr-linux-crypto/build
    make uninstall 2>/dev/null || true
    echo "gr-linux-crypto removed."
fi

# Remove gr-packet-protocols
if [ -d "/usr/src/gr-packet-protocols/build" ]; then
    echo "Removing gr-packet-protocols..."
    cd /usr/src/gr-packet-protocols/build
    make uninstall 2>/dev/null || true
    echo "gr-packet-protocols removed."
fi

# Remove gr-qradiolink
if [ -d "/usr/src/gr-qradiolink/build" ]; then
    echo "Removing gr-qradiolink..."
    cd /usr/src/gr-qradiolink/build
    make uninstall 2>/dev/null || true
    echo "gr-qradiolink removed."
fi

# Update library cache
echo "Updating library cache..."
ldconfig

################################################################################
# Step 4: Remove Python packages
################################################################################

echo ""
echo "================================================================"
echo " Removing Python Packages"
echo "================================================================"
echo ""

# Remove authenticated control Python package if installed via pip
if pip3 show authenticated-repeater-control >/dev/null 2>&1; then
    echo "Removing authenticated-repeater-control Python package..."
    pip3 uninstall -y authenticated-repeater-control 2>/dev/null || true
    echo "Python package removed."
else
    echo "Python package not installed via pip."
fi

################################################################################
# Step 5: Remove executable scripts
################################################################################

echo ""
echo "================================================================"
echo " Removing Executable Scripts"
echo "================================================================"
echo ""

SCRIPTS=(
    "/usr/local/bin/orp-keygen"
    "/usr/local/bin/orp-keymgmt"
    "/usr/local/bin/orp-sign-command"
    "/usr/local/bin/operator-cli"
    "/usr/local/bin/orp-command-handler"
    "/usr/local/share/authenticated-repeater/authenticated_command_handler.py"
)

for script in "${SCRIPTS[@]}"; do
    if [ -f "$script" ]; then
        echo "Removing $script..."
        rm -f "$script"
    fi
done

echo "Scripts removed."

################################################################################
# Step 6: Remove configuration and data (optional)
################################################################################

echo ""
echo "================================================================"
echo " Configuration and Data Removal"
echo "================================================================"
echo ""
read -r -p "Remove configuration files? (y/N): " remove_config
if [ "$remove_config" = "y" ] || [ "$remove_config" = "Y" ]; then
    if [ -d "/etc/authenticated-repeater" ]; then
        echo "Removing configuration directory..."
        rm -rf /etc/authenticated-repeater
        echo "Configuration removed."
    else
        echo "Configuration directory not found."
    fi
else
    echo "Configuration files preserved."
fi

read -r -p "Remove log files? (y/N): " remove_logs
if [ "$remove_logs" = "y" ] || [ "$remove_logs" = "Y" ]; then
    if [ -d "/var/log/authenticated-repeater" ]; then
        echo "Removing log directory..."
        rm -rf /var/log/authenticated-repeater
        echo "Logs removed."
    else
        echo "Log directory not found."
    fi
else
    echo "Log files preserved."
fi

read -r -p "Remove flowgraphs? (y/N): " remove_flowgraphs
if [ "$remove_flowgraphs" = "y" ] || [ "$remove_flowgraphs" = "Y" ]; then
    if [ -d "/usr/local/share/authenticated-repeater/flowgraphs" ]; then
        echo "Removing flowgraphs..."
        rm -rf /usr/local/share/authenticated-repeater/flowgraphs
        echo "Flowgraphs removed."
    else
        echo "Flowgraphs directory not found."
    fi
    
    # Remove parent directory if empty
    if [ -d "/usr/local/share/authenticated-repeater" ]; then
        rmdir /usr/local/share/authenticated-repeater 2>/dev/null || true
    fi
else
    echo "Flowgraphs preserved."
fi

################################################################################
# Step 7: Remove source directories (optional)
################################################################################

echo ""
echo "================================================================"
echo " Source Directory Removal"
echo "================================================================"
echo ""
read -r -p "Remove source directories? (y/N): " remove_sources
if [ "$remove_sources" = "y" ] || [ "$remove_sources" = "Y" ]; then
    if [ -d "/usr/src/gr-linux-crypto" ]; then
        echo "Removing /usr/src/gr-linux-crypto..."
        rm -rf /usr/src/gr-linux-crypto
    fi
    if [ -d "/usr/src/gr-packet-protocols" ]; then
        echo "Removing /usr/src/gr-packet-protocols..."
        rm -rf /usr/src/gr-packet-protocols
    fi
    if [ -d "/usr/src/gr-qradiolink" ]; then
        echo "Removing /usr/src/gr-qradiolink..."
        rm -rf /usr/src/gr-qradiolink
    fi
    echo "Source directories removed."
else
    echo "Source directories preserved."
fi

################################################################################
# Verification
################################################################################

echo ""
echo "================================================================"
echo " Verification"
echo "================================================================"
echo ""

errors=0

# Check service
if systemctl list-unit-files | grep -q authenticated-control; then
    echo "WARNING: Service still registered"
    errors=$((errors + 1))
else
    echo "OK: Service removed"
fi

# Check Python package
if python3 -c "import authenticated_repeater_control" 2>/dev/null; then
    echo "WARNING: Python package still importable"
    errors=$((errors + 1))
else
    echo "OK: Python package removed"
fi

# Check configuration
if [ -d "/etc/authenticated-repeater" ]; then
    echo "INFO: Configuration directory still exists (preserved)"
else
    echo "OK: Configuration directory removed"
fi

if [ $errors -eq 0 ]; then
    echo ""
    echo "================================================================"
    echo " Uninstallation Complete!"
    echo "================================================================"
    echo ""
    exit 0
else
    echo ""
    echo "================================================================"
    echo " Uninstallation completed with warnings"
    echo "================================================================"
    echo "Some components may still be present. Review the warnings above."
    echo ""
    exit 1
fi

