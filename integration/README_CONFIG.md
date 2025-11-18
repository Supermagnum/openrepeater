# Configuration File Setup

## Placeholder Config File

A placeholder configuration file has been created at:
- `integration/config.yaml.placeholder` - Template file
- `integration/test_config/config.yaml` - Test location (no sudo required)

## Installation Location

For production use, the config file should be placed at:
```
/etc/authenticated-repeater/config.yaml
```

## Creating the Config File (Requires sudo)

To create the config file in the system location:

```bash
sudo mkdir -p /etc/authenticated-repeater/authorized_operators
sudo mkdir -p /etc/authenticated-repeater/repeater_keys
sudo cp integration/config.yaml.placeholder /etc/authenticated-repeater/config.yaml
sudo chmod 644 /etc/authenticated-repeater/config.yaml
```

## Testing Without sudo

For testing purposes, you can modify the `load_config()` function in `authenticated_command_handler.py` to use a local path, or use environment variable:

```python
# Temporarily modify line 54 in authenticated_command_handler.py:
config_path = os.getenv('AUTHENTICATED_CONFIG', '/etc/authenticated-repeater/config.yaml')
```

Then test with:
```bash
export AUTHENTICATED_CONFIG=integration/test_config/config.yaml
python3 integration/authenticated_command_handler.py
```

## Safety

This config file is in a separate directory (`/etc/authenticated-repeater/`) from OpenRepeater's configuration (`/etc/svxlink/`), so it will not interfere with OpenRepeater installation.

## Next Steps

1. Add authorized operator public keys to `/etc/authenticated-repeater/authorized_operators/`
2. Generate repeater keypair in `/etc/authenticated-repeater/repeater_keys/`
3. Adjust settings in config.yaml as needed for your setup

