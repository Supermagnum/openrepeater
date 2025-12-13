# Integration Summary: New Functions from Synced Modules

This document summarizes the new functions and features that have been integrated from the synced upstream repositories.

## Date: 2025-01-08

## Modules Synced

1. **gr-linux-crypto** - https://github.com/Supermagnum/gr-linux-crypto
2. **gr-packet-protocols** - https://github.com/Supermagnum/gr-packet-protocols
3. **gr-qradiolink** - https://github.com/Supermagnum/gr-qradiolink

## New Features Integrated

### gr-packet-protocols

#### PTT Control Functions
- `kiss_tnc.set_ptt_enabled(enabled)` - Enable/disable automatic PTT control
- `kiss_tnc.set_ptt_use_dtr(use_dtr)` - Use DTR line instead of RTS for PTT
- `kiss_tnc.set_ptt(state)` - Manually control PTT (True = key, False = unkey)
- `kiss_tnc.get_ptt()` - Get current PTT state

#### PlutoSDR PTT Control
- `pluto_ptt_control` - New Python block for controlling PTT on PlutoSDR via IIO GPIO
  - Parameters: pluto_uri, gpio_pin, tx_delay_ms, tx_tail_ms, invert
  - Methods: `set_ptt(state)`, `get_ptt()`

#### Adaptive Features
- `link_quality_monitor` - Real-time SNR, BER, and frame error rate monitoring
- `adaptive_rate_control` - Automatic modulation mode adjustment based on link quality
- `modulation_switch` - Switch between multiple modulation inputs
- `adaptive_modulator` - Python hierarchical block for automatic modulation switching

**Location**: `modules/gr-packet-protocols/python/packet_protocols/`

### gr-qradiolink

#### New Digital Voice Protocols

**dPMR (Digital Private Mobile Radio)**
- `mod_dpmr` - dPMR modulator (ETSI TS 102 658, 2400 baud, 6.25 kHz spacing)
- `demod_dpmr` - dPMR demodulator

**NXDN (Next Generation Digital Narrowband)**
- `mod_nxdn` - NXDN modulator (supports NXDN48/2400 baud and NXDN96/4800 baud)
- `demod_nxdn` - NXDN demodulator

**MMDVM Protocols**
- `pocsag_encoder` / `pocsag_decoder` - POCSAG paging protocol (ITU-R M.584-2)
- `dstar_encoder` / `dstar_decoder` - D-STAR protocol with Golay FEC
- `ysf_encoder` / `ysf_decoder` - Yaesu System Fusion (C4FM)
- `p25_encoder` / `p25_decoder` - Project 25 Phase 1 C4FM

**Location**: `modules/gr-qradiolink/lib/` and `modules/gr-qradiolink/python/qradiolink/`

### gr-linux-crypto

#### Security Improvements
- Enhanced type hints and linting fixes
- Improved security annotations for deprecated algorithms (SHA1, ECB mode)
- Better error handling and documentation

**Location**: `modules/gr-linux-crypto/python/`

## Documentation Updates

### Updated Files
- `docs/additional/MODULE_USAGE.md` - Added documentation for:
  - PTT control functions in KISS TNC
  - PlutoSDR PTT control block
  - Adaptive features (link quality, rate control, modulation switching)
  - New gr-qradiolink protocols (dPMR, NXDN, MMDVM)

## Integration Points

### Examples Using Modules
- `examples/tx_audio_signed.py` - Uses linux_crypto and packet_protocols
- `examples/signed_message_tx.py` - Uses linux_crypto, packet_protocols, and qradiolink
- `examples/tx_audio_signed_nogui.py` - Uses linux_crypto and packet_protocols
- `validate_ax25_flowgraph.py` - Uses packet_protocols

### Integration Files
- `integration/authenticated_command_handler.py` - Command handler (no direct module usage)
- Examples in `modules/gr-packet-protocols/examples/` show adaptive features usage

## Next Steps

1. **Build and Test**: Rebuild the modules to ensure all Python bindings are available
   ```bash
   cd modules/gr-packet-protocols && mkdir -p build && cd build && cmake .. && make
   cd modules/gr-qradiolink && mkdir -p build && cd build && cmake .. && make
   cd modules/gr-linux-crypto && mkdir -p build && cd build && cmake .. && make
   ```

2. **Update Examples**: Consider updating examples to demonstrate:
   - PTT control in KISS TNC blocks
   - Adaptive modulation features
   - New dPMR/NXDN protocols
   - MMDVM protocol usage

3. **Testing**: Run tests to verify integration:
   ```bash
   cd modules/gr-packet-protocols && python3 -m pytest tests/
   cd modules/gr-qradiolink && python3 -m pytest tests/
   ```

## Notes

- All new functions are available through Python bindings
- GRC (GNU Radio Companion) blocks are available for all new features
- See module README files for detailed usage examples:
  - `modules/gr-packet-protocols/README.md`
  - `modules/gr-qradiolink/README.md`
  - `modules/gr-linux-crypto/README.md`

