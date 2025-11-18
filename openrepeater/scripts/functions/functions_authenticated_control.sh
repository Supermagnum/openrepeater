#!/bin/bash
################################################################################
# DEFINE AUTHENTICATED CONTROL FUNCTIONS
################################################################################
# This function file installs and configures the authenticated repeater
# control system using GNU Radio cryptographic modules
################################################################################

function install_authenticated_control () {
	#####################################################################
	echo "--------------------------------------------------------------"
	echo " Installing Authenticated Repeater Control System"
	echo "--------------------------------------------------------------"
	#####################################################################

	##########################################
	echo "-----------------------------------"
	echo " Checking prerequisites"
	echo "-----------------------------------"
	##########################################
	
	# Check for GNU Radio
	if ! command -v gnuradio-config-info &> /dev/null; then
		echo "GNU Radio not found. Installing GNU Radio..."
		install_gnuradio
	else
		GNURADIO_VERSION=$(gnuradio-config-info --version | cut -d' ' -f2)
		echo "GNU Radio version $GNURADIO_VERSION found"
		
		# Check minimum version (3.10.12.0)
		REQUIRED_VERSION="3.10.12.0"
		if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$GNURADIO_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
			echo "WARNING: GNU Radio version $GNURADIO_VERSION is below required $REQUIRED_VERSION"
			echo "Attempting to install/upgrade GNU Radio..."
			install_gnuradio
		fi
	fi

	##########################################
	echo "-----------------------------------"
	echo " Installing system dependencies"
	echo "-----------------------------------"
	##########################################
	
	apt update
	
	args=(
	--assume-yes
	--fix-missing
	libgcrypt20-dev		# Required for cryptographic operations
	python3-cryptography	# Python cryptography library
	python3-pkcs11		# PKCS#11 support for hardware tokens
	python3-zmq		# ZeroMQ for IPC
	python3-yaml		# YAML configuration parsing
	cmake			# Build system
	g++			# C++ compiler
	make			# Build tool
	pkg-config		# Package configuration
	libboost-dev		# Boost libraries
	libboost-system-dev
	libboost-thread-dev
	libboost-filesystem-dev
	swig			# Python bindings generator
	# Web stack dependencies (for OpenRepeater integration)
	nginx			# Web server
	nginx-extras		# Extended nginx features
	php8.2-fpm		# PHP FastCGI Process Manager
	php8.2-common		# Common PHP files
	php8.2-cli		# PHP command line interface
	php8.2-curl		# PHP cURL extension
	php8.2-dev		# PHP development files
	php8.2-gd		# PHP GD graphics library
	php8.2-mbstring		# PHP multibyte string support
	php8.2-sqlite3		# PHP SQLite3 support
	php8.2-xml		# PHP XML support
	php8.2-xmlrpc		# PHP XML-RPC support
	php8.2-zip		# PHP ZIP support
	php-memcached		# PHP memcached extension
	php-imagick		# PHP ImageMagick extension
	php-ssh2		# PHP SSH2 extension
	sqlite3			# SQLite database
	memcached		# Memcached server
	ssl-cert		# SSL certificates
	)
	
	apt install "${args[@]}"

	##########################################
	echo "-----------------------------------"
	echo " Setting up key management directories"
	echo "-----------------------------------"
	##########################################
	
	mkdir -p /etc/authenticated-repeater/authorized_operators
	mkdir -p /etc/authenticated-repeater/repeater_keys
	mkdir -p /var/log/authenticated-repeater
	
	# Set proper permissions
	chmod 755 /etc/authenticated-repeater
	chmod 755 /etc/authenticated-repeater/authorized_operators
	chmod 700 /etc/authenticated-repeater/repeater_keys
	chmod 755 /var/log/authenticated-repeater
	
	# Create default config file if it doesn't exist
	if [ ! -f /etc/authenticated-repeater/config.yaml ]; then
		cat > /etc/authenticated-repeater/config.yaml << 'EOF'
# Authenticated Repeater Control Configuration

# IPC mechanism: 'zmq', 'fifo', or 'socket'
ipc_mechanism: zmq

# ZMQ socket paths (if using zmq)
zmq_rx_socket: ipc:///tmp/authenticated_rx.sock
zmq_tx_socket: ipc:///tmp/authenticated_tx.sock

# SVXLink control method: 'tcp', 'config', or 'dtmf'
svxlink_control: tcp

# SVXLink TCP control port (if using tcp)
svxlink_tcp_host: localhost
svxlink_tcp_port: 5210

# SVXLink config file path (if using config)
svxlink_config: /etc/svxlink/svxlink.conf

# Logging
log_file: /var/log/authenticated-repeater/commands.log
log_level: INFO

# Security settings
replay_protection_window: 300  # seconds
max_commands_per_minute: 10
command_timeout: 30  # seconds

# Key management
authorized_keys_dir: /etc/authenticated-repeater/authorized_operators
repeater_keys_dir: /etc/authenticated-repeater/repeater_keys
EOF
		chmod 644 /etc/authenticated-repeater/config.yaml
	fi

	##########################################
	echo "-----------------------------------"
	echo " Building GNU Radio OOT modules"
	echo "-----------------------------------"
	##########################################
	
	cd "$base_dir"
	
	# Build gr-linux-crypto
	if [ ! -d "gr-linux-crypto" ]; then
		echo "Cloning gr-linux-crypto..."
		git clone https://github.com/Supermagnum/gr-linux-crypto.git
	fi
	
	cd gr-linux-crypto
	git pull
	mkdir -p build
	cd build
	cmake ..
	make -j$(nproc)
	make install
	ldconfig
	cd "$base_dir"
	
	# Build gr-packet-protocols
	if [ ! -d "gr-packet-protocols" ]; then
		echo "Cloning gr-packet-protocols..."
		git clone https://github.com/Supermagnum/gr-packet-protocols.git
	fi
	
	cd gr-packet-protocols
	git pull
	mkdir -p build
	cd build
	cmake ..
	make -j$(nproc)
	make install
	ldconfig
	cd "$base_dir"
	
	# Build gr-qradiolink
	if [ ! -d "gr-qradiolink" ]; then
		echo "Cloning gr-qradiolink..."
		git clone https://github.com/Supermagnum/gr-qradiolink.git
	fi
	
	cd gr-qradiolink
	git pull
	mkdir -p build
	cd build
	cmake ..
	make -j$(nproc)
	make install
	ldconfig
	cd "$base_dir"

	##########################################
	echo "-----------------------------------"
	echo " Installing Python dependencies"
	echo "-----------------------------------"
	##########################################
	
	pip3 install --upgrade pip
	pip3 install cryptography pyyaml pyzmq

	##########################################
	echo "-----------------------------------"
	echo " Installing flowgraphs"
	echo "-----------------------------------"
	##########################################
	
	# Create flowgraph directory
	mkdir -p /usr/local/share/authenticated-repeater/flowgraphs
	
	# Copy flowgraphs if they exist in the integration directory
	# Try multiple possible paths for flowgraphs
	FLOWGRAPH_DIR=""
	if [ -d "$SCRIPT_DIR/../integration/flowgraphs" ]; then
		FLOWGRAPH_DIR="$SCRIPT_DIR/../integration/flowgraphs"
	elif [ -d "$SCRIPT_DIR/../../integration/flowgraphs" ]; then
		FLOWGRAPH_DIR="$SCRIPT_DIR/../../integration/flowgraphs"
	fi
	
	if [ -n "$FLOWGRAPH_DIR" ] && [ -d "$FLOWGRAPH_DIR" ]; then
		cp -r "$FLOWGRAPH_DIR"/* /usr/local/share/authenticated-repeater/flowgraphs/ 2>/dev/null || true
	fi
	
	# Set permissions
	chmod 755 /usr/local/share/authenticated-repeater/flowgraphs

	##########################################
	echo "-----------------------------------"
	echo " Installing systemd service"
	echo "-----------------------------------"
	##########################################
	
	# Copy service file if it exists
	# Try multiple possible paths for the service file
	SERVICE_FILE=""
	if [ -f "$SCRIPT_DIR/../integration/authenticated-control.service" ]; then
		SERVICE_FILE="$SCRIPT_DIR/../integration/authenticated-control.service"
	elif [ -f "$SCRIPT_DIR/../../integration/authenticated-control.service" ]; then
		SERVICE_FILE="$SCRIPT_DIR/../../integration/authenticated-control.service"
	elif [ -f "/home/haaken/github-projects/authenticated-repeater-control/integration/authenticated-control.service" ]; then
		SERVICE_FILE="/home/haaken/github-projects/authenticated-repeater-control/integration/authenticated-control.service"
	fi
	
	if [ -n "$SERVICE_FILE" ] && [ -f "$SERVICE_FILE" ]; then
		cp "$SERVICE_FILE" /etc/systemd/system/
		systemctl daemon-reload
		systemctl enable authenticated-control.service
		echo "Service installed and enabled (not started yet)"
	fi

	##########################################
	echo "-----------------------------------"
	echo " Installation complete"
	echo "-----------------------------------"
	##########################################
	
	echo "Next steps:"
	echo "1. Add authorized operator public keys to /etc/authenticated-repeater/authorized_operators/"
	echo "2. Generate repeater keypair (if not already done)"
	echo "3. Place your GRC flowgraphs in /usr/local/share/authenticated-repeater/flowgraphs/"
	echo "4. Configure /etc/authenticated-repeater/config.yaml"
	echo "5. Start service: systemctl start authenticated-control"
}

function install_gnuradio () {
	#####################################################################
	echo "--------------------------------------------------------------"
	echo " Installing GNU Radio"
	echo "--------------------------------------------------------------"
	#####################################################################
	
	apt update
	
	args=(
	--assume-yes
	--fix-missing
	gnuradio-dev
	gnuradio-runtime
	python3-gnuradio
	)
	
	apt install "${args[@]}"
	
	# Verify installation
	if command -v gnuradio-config-info &> /dev/null; then
		GNURADIO_VERSION=$(gnuradio-config-info --version | cut -d' ' -f2)
		echo "GNU Radio $GNURADIO_VERSION installed successfully"
	else
		echo "ERROR: GNU Radio installation failed"
		return 1
	fi
}

function verify_authenticated_control_installation () {
	#####################################################################
	echo "--------------------------------------------------------------"
	echo " Verifying Authenticated Control Installation"
	echo "--------------------------------------------------------------"
	#####################################################################
	
	local errors=0
	
	# Check GNU Radio
	if ! command -v gnuradio-config-info &> /dev/null; then
		echo "ERROR: GNU Radio not found"
		errors=$((errors + 1))
	else
		echo "OK: GNU Radio installed"
	fi
	
	# Check Python modules
	if python3 -c "import cryptography" 2>/dev/null; then
		echo "OK: Python cryptography module available"
	else
		echo "ERROR: Python cryptography module not found"
		errors=$((errors + 1))
	fi
	
	if python3 -c "import zmq" 2>/dev/null; then
		echo "OK: Python zmq module available"
	else
		echo "ERROR: Python zmq module not found"
		errors=$((errors + 1))
	fi
	
	# Check directories
	if [ -d "/etc/authenticated-repeater" ]; then
		echo "OK: Configuration directory exists"
	else
		echo "ERROR: Configuration directory missing"
		errors=$((errors + 1))
	fi
	
	if [ -d "/etc/authenticated-repeater/authorized_operators" ]; then
		echo "OK: Authorized operators directory exists"
	else
		echo "ERROR: Authorized operators directory missing"
		errors=$((errors + 1))
	fi
	
	# Check service file
	if [ -f "/etc/systemd/system/authenticated-control.service" ]; then
		echo "OK: Systemd service file installed"
	else
		echo "WARNING: Systemd service file not found"
	fi
	
	if [ $errors -eq 0 ]; then
		echo "--------------------------------------------------------------"
		echo " Verification complete - No errors found"
		echo "--------------------------------------------------------------"
		return 0
	else
		echo "--------------------------------------------------------------"
		echo " Verification complete - $errors error(s) found"
		echo "--------------------------------------------------------------"
		return 1
	fi
}

