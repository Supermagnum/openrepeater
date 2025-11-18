#!/bin/bash
################################################################################
# Standalone Installation Script for Authenticated Repeater Control
#
# This script can be run independently to install the authenticated control
# system without running the full OpenRepeater installation.
#
# Usage: sudo ./install_authenticated_modules.sh
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_DIR="/usr/src"

################################################################################
# Check prerequisites
################################################################################

if [[ $EUID -ne 0 ]]; then
    echo "ERROR: This script must be run as root"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is required"
    exit 1
fi

################################################################################
# Source the authenticated control functions
################################################################################

if [ -f "$SCRIPT_DIR/../openrepeater/scripts/functions/functions_authenticated_control.sh" ]; then
    source "$SCRIPT_DIR/../openrepeater/scripts/functions/functions_authenticated_control.sh"
else
    echo "ERROR: functions_authenticated_control.sh not found"
    exit 1
fi

################################################################################
# Main installation
################################################################################

echo "================================================================"
echo " Authenticated Repeater Control System Installation"
echo "================================================================"
echo ""

# Install the system
install_authenticated_control

# Verify installation
echo ""
echo "================================================================"
echo " Verifying Installation"
echo "================================================================"
echo ""

if verify_authenticated_control_installation; then
    echo ""
    echo "================================================================"
    echo " Installation Complete!"
    echo "================================================================"
    echo ""
    echo "Next steps:"
    echo "1. Add authorized operator public keys to:"
    echo "   /etc/authenticated-repeater/authorized_operators/"
    echo ""
    echo "2. Generate repeater keypair (if needed):"
    echo "   See documentation for key generation instructions"
    echo ""
    echo "3. Place your GRC flowgraphs in:"
    echo "   /usr/local/share/authenticated-repeater/flowgraphs/"
    echo ""
    echo "4. Configure the system:"
    echo "   /etc/authenticated-repeater/config.yaml"
    echo ""
    echo "5. Start the service:"
    echo "   sudo systemctl start authenticated-control"
    echo "   sudo systemctl status authenticated-control"
    echo ""
    exit 0
else
    echo ""
    echo "================================================================"
    echo " Installation completed with errors"
    echo "================================================================"
    echo "Please review the errors above and fix them before starting the service"
    echo ""
    exit 1
fi

