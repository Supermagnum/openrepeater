# gr-qradiolink

GNU Radio out-of-tree (OOT) module for QRadioLink blocks.

## Overview

This module provides GNU Radio blocks for various digital and analog modulation schemes, specialized for amateur radio and digital voice communications.

This module was converted from the [QRadioLink](https://qradiolink.org/) application, which is a multimode SDR transceiver for GNU Radio, ADALM-Pluto, LimeSDR, USRP, and other SDR hardware. The original source code is located at [https://codeberg.org/qradiolink/qradiolink](https://codeberg.org/qradiolink/qradiolink).

The QRadioLink Codeberg page does not mention any crashes or other known issues. The code has been fuzzed extensively using libFuzzer with over 104 million executions across multiple blocks, and no crashes or memory leaks were discovered. However, the codebase could potentially benefit from further fuzzing, particularly for blocks that have not yet been fuzzed (see [fuzzing-results/results.md](fuzzing-results/results.md) for coverage details).

## Features

### Modulation/Demodulation Blocks

- **Digital Modulations**: 2FSK, 4FSK, GMSK, BPSK, QPSK, DSSS
- **Analog Modulations**: AM, SSB (USB/LSB), NBFM
- **Digital Voice**: FreeDV, M17, DMR (Tier I/II/III), dPMR, NXDN, MMDVM
  - **dPMR**: Digital Private Mobile Radio (ETSI TS 102 658), 2400 baud, 6.25 kHz channel spacing
  - **NXDN**: Next Generation Digital Narrowband, supports NXDN48 (2400 baud) and NXDN96 (4800 baud) modes
- **Supporting Blocks**: Audio source/sink, RSSI, FFT, deframer, CESSB

### Python Bindings

All blocks are available through Python bindings, including:
- All modulation/demodulation blocks (2FSK, 4FSK, GMSK, BPSK, QPSK, DSSS, AM, SSB, NBFM)
- All digital voice blocks (FreeDV, M17, DMR, **dPMR**, **NXDN**, MMDVM)
- Supporting blocks (RSSI, M17 deframer, etc.)

The Python bindings enable use in GNU Radio Companion flowgraphs and Python scripts.

## Directory Structure

```
gr-qradiolink/
├── CMakeLists.txt          # Top-level CMake configuration
├── include/                 # Public header files
│   └── gnuradio/
│       └── qradiolink/
├── lib/                    # Implementation files
├── python/                 # Python bindings
│   └── qradiolink/
│       └── bindings/
├── grc/                    # GNU Radio Companion block definitions
├── docs/                   # Documentation
│   ├── doxygen/
│   └── PTT_CONTROL.md      # PTT control with gr-osmosdr
├── examples/               # Example flowgraphs
├── tests/                  # Unit tests
└── cmake/                  # CMake modules
    └── Modules/
```

## Dependencies

See [DEPENDENCIES.md](DEPENDENCIES.md) for a complete list of required and optional dependencies.

**Quick Summary:**
- GNU Radio >= 3.10 (with vocoder component built with Codec2 support)
- CMake >= 3.16
- Boost libraries
- Volk (Vector-Optimized Library of Kernels)
- ZeroMQ (optional, for MMDVM blocks)
- Python 3.x with NumPy (for Python bindings)
- fmt library (for tests)

## Testing

The module includes comprehensive unit tests for all blocks. Tests are run using CTest and Boost.Test framework.

### Test Results

All tests pass successfully:

```
100% tests passed, 0 tests failed out of 20

Test Breakdown:
- 5 Manual tests (with int main): test_mod_2fsk, test_mod_4fsk, test_mod_am, 
  test_mod_gmsk, test_mod_bpsk
- 15 Boost.Test tests (using gr_add_cpp_test): All remaining modulator and 
  demodulator tests

Test Coverage:
- Modulators: 2FSK, 4FSK, AM, GMSK, BPSK, SSB, QPSK, NBFM, DSSS, M17, DMR, dPMR, NXDN
- Demodulators: 2FSK, 4FSK, AM, GMSK, BPSK, SSB, QPSK, NBFM, DSSS, WBFM, M17, DMR, dPMR, NXDN
```

### Running Tests

To build and run the test suite:

```bash
cd build
cmake ..
make
ctest --output-on-failure
```

### Fuzzing Results

The module includes comprehensive fuzzing coverage using libFuzzer. See [fuzzing-results/results.md](fuzzing-results/results.md) for complete fuzzing campaign results including:

- Coverage statistics (757 edges, 893 features discovered)
- Execution metrics (104+ million executions)
- Performance analysis
- Security assessment (0 crashes, 0 memory leaks)

### Python Validation Tests

The module includes Python-based validation tests for all modulation types. See [fuzzing-results/results.md](fuzzing-results/results.md) for validation test results. All digital voice modes (FreeDV, M17, DMR, dPMR, NXDN) now have Python bindings and validation support.

## Documentation

- **[PTT Control Guide](docs/PTT_CONTROL.md)**: Comprehensive guide on controlling PTT (Push-To-Talk) with gr-osmosdr and similar SDR hardware when using gr-qradiolink blocks.

## License

This project is licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later).

See the [LICENSE](../LICENSE) file in the QRadioLink repository for details.

