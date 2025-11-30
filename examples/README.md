# gr-qradiolink Examples

This directory contains example flowgraphs and scripts demonstrating how to use the gr-qradiolink module.

## Available Examples

### NBFM Example (`nbfm_example.grc`)

A complete GNU Radio Companion flowgraph demonstrating Narrow Band FM (NBFM) modulation and demodulation.

**Features:**
- Audio source (tone generator for testing)
- NBFM modulator
- NBFM demodulator
- Frequency display of modulated signal
- Time domain display of demodulated audio

**Usage:**
1. Install the module first: `cd build && make install` (ensures blocks are in GRC search path)
2. Open `nbfm_example.grc` in GNU Radio Companion: `gnuradio-companion nbfm_example.grc`
3. Optionally replace the audio source with an actual audio input block
4. Generate and run the flowgraph from GRC GUI

**Note:** If `grcc` reports connection errors, this is a known validation issue with GRC 3.10.12.0. The flowgraph is correctly formatted and will work when opened in GRC GUI.

**Parameters:**
- Sample rate: 250000 Hz
- Carrier frequency: 1700 Hz
- Filter width: 8000 Hz
- Samples per symbol: 125

### M17 Demodulation Example (`m17_demod_example.grc`)

A GNU Radio Companion flowgraph for M17 digital voice demodulation.

**Features:**
- M17 demodulator block
- Frequency display of input signal
- Constellation display
- File output for decoded data

**Usage:**
1. Install the module first: `cd build && make install` (ensures blocks are in GRC search path)
2. Open `m17_demod_example.grc` in GNU Radio Companion: `gnuradio-companion m17_demod_example.grc`
3. Replace the signal source with your M17 signal source (SDR, file, etc.)
4. Generate and run the flowgraph from GRC GUI
5. Decoded data will be saved to `/tmp/m17_decoded.bin`

**Note:** If `grcc` reports connection errors, this is a known validation issue with GRC 3.10.12.0. The flowgraph is correctly formatted and will work when opened in GRC GUI.

**Parameters:**
- Sample rate: 1000000 Hz (1 MHz)
- Carrier frequency: 1700 Hz
- Filter width: 9000 Hz
- Samples per symbol: 125

### M17 Modulation Example (`m17_mod_example.py`)

A Python script demonstrating M17 digital voice modulation.

**Features:**
- Audio input (microphone or test signal)
- M17 modulator
- Frequency display
- File output for modulated signal

**Usage:**
```bash
python3 m17_mod_example.py
```

**Note:** This example uses Python blocks since `mod_m17` doesn't have a GRC block definition yet. You can use this as a reference for creating GRC flowgraphs or use it directly as a Python script.

### FreeDV Example (`freedv_example.py`)

A Python script demonstrating FreeDV digital voice modulation and demodulation.

**Features:**
- Audio input (microphone or test signal)
- FreeDV modulator
- FreeDV demodulator
- Frequency display of modulated signal
- Time domain display of demodulated audio
- Audio output

**Usage:**
```bash
python3 freedv_example.py
```

**Note:** This example uses Python blocks since FreeDV blocks don't have GRC definitions yet. You can use this as a reference for creating GRC flowgraphs or use it directly as a Python script.

**FreeDV Modes:**
- MODE_1600: 1600 Hz bandwidth (default)
- MODE_700: 700 Hz bandwidth
- MODE_700B: 700 Hz bandwidth (variant B)
- MODE_700C: 700 Hz bandwidth (variant C)
- MODE_700D: 700 Hz bandwidth (variant D)
- MODE_800XA: 800 Hz bandwidth
- MODE_2400A: 2400 Hz bandwidth
- MODE_2400B: 2400 Hz bandwidth (variant B)

**Parameters:**
- Sample rate: 8000 Hz
- Carrier frequency: 1700 Hz
- Filter width: 2000 Hz
- Low cutoff: 200 Hz
- Sideband: 0 (USB) or 1 (LSB)

## Requirements

All examples require:
- GNU Radio >= 3.10
- gr-qradiolink module installed
- Python 3.x (for Python examples)
- PyQt5 (for GUI examples)
- gnuradio-vocoder with Codec2 support (for FreeDV example)

## Installation

Examples are installed to `${GR_DOC_DIR}/examples/qradiolink` when you build and install the module.

## MMDVM Protocol Blocks

The following MMDVM protocol encoder/decoder blocks are available with GRC block definitions:

- **POCSAG**: `qradiolink_pocsag_encoder`, `qradiolink_pocsag_decoder`
- **D-STAR**: `qradiolink_dstar_encoder`, `qradiolink_dstar_decoder`
- **YSF**: `qradiolink_ysf_encoder`, `qradiolink_ysf_decoder`
- **P25**: `qradiolink_p25_encoder`, `qradiolink_p25_decoder`

These blocks can be used in GNU Radio Companion flowgraphs. See the GRC block definitions in `grc/` for parameter details.

**Example Usage:**
1. Open GNU Radio Companion
2. Search for "qradiolink" in the block library
3. Add the desired encoder or decoder block
4. Configure parameters (baud rate, address, etc.) as needed
5. Connect to appropriate sources/sinks (file, SDR, etc.)

## Notes

- Some blocks (mod_m17, mod_freedv, demod_freedv) don't have GRC block definitions yet, so Python examples are provided for these.
- MMDVM protocol blocks (POCSAG, D-STAR, YSF, P25) have complete GRC block definitions and can be used directly in flowgraphs.
- Audio blocks may require proper audio device configuration on your system.
- File paths in examples use `/tmp/` - adjust as needed for your system.
- Test signal sources can be replaced with actual audio inputs or SDR sources.

