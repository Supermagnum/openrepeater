# Dependencies for gr-qradiolink

This document lists all dependencies required to build and use the gr-qradiolink GNU Radio module.

## Required Build Tools

- **CMake** >= 3.16
- **C++ Compiler** with C++17 support (GCC >= 7 or Clang >= 5)
- **pkg-config** (for dependency detection)
- **Make** or **Ninja** (build system)

## Required GNU Radio Components

The following GNU Radio components are required via pkg-config:

- **gnuradio-runtime** - Core runtime infrastructure
- **gnuradio-blocks** - Basic block library
- **gnuradio-filter** - Filter signal processing blocks
- **gnuradio-digital** - Digital communications blocks
- **gnuradio-analog** - Analog communications blocks
- **gnuradio-fec** - Forward Error Correction blocks
- **gnuradio-vocoder** - Voice codec blocks (must be built with Codec2 support)

### GNU Radio Version

Minimum required: GNU Radio >= 3.10

The gnuradio-vocoder component must be built with Codec2 support. Verify this by checking that the following was printed during GNU Radio's build:

```
--   * gr-vocoder
--   * * codec2
--   * * freedv
```

## Required System Libraries

### Boost Libraries

Required Boost components:
- **boost-date_time**
- **boost-program_options**
- **boost-system**
- **boost-regex**
- **boost-thread**
- **boost-unit_test_framework** (for testing)

Boost is typically provided via GNU Radio's build system.

### Volk

**Volk** (Vector-Optimized Library of Kernels) - Required for optimized signal processing.

Package names:
- Debian/Ubuntu: `libvolk2-dev` (or `libvolk-dev` depending on version)
- Fedora: `volk-devel`

## Optional Dependencies

### ZeroMQ (ZMQ)

Required for MMDVM source/sink blocks (`mmdvm_source`, `mmdvm_sink`).

Package names:
- Debian/Ubuntu: `libzmq3-dev` (development headers), `libzmq5` (runtime)
- Fedora: `zeromq-devel`

The build system will detect ZMQ automatically. If not found, MMDVM blocks will not be compiled.

## Python Bindings Dependencies

If building Python bindings (enabled by default):

- **Python 3.x** (3.6 or later)
- **pybind11** (provided via GNU Radio's GrPybind module)
- **NumPy** (for Python interface)

## Testing Dependencies

For unit tests:

- **Boost.Test** (unit_test_framework component) - Part of Boost
- **fmt** library - Used by test files

Package names:
- Debian/Ubuntu: `libfmt-dev`
- Fedora: `fmt-devel`

## Codec2 Dependency

Codec2 support comes through `gnuradio-vocoder`. The vocoder component must be built with Codec2 support enabled.

If you need to build GNU Radio with Codec2 support, ensure:

- **libcodec2-dev** (development headers) is installed
- **libcodec2** (runtime library) is installed

Package names:
- Debian/Ubuntu: `libcodec2-dev`, `libcodec2-0.9` (or similar version)
- Fedora: `codec2-devel`

## Package Installation Examples

### Debian/Ubuntu

```bash
sudo apt-get update
sudo apt-get install \
    cmake \
    build-essential \
    pkg-config \
    gnuradio-dev \
    gnuradio \
    libgnuradio-runtime3.10 \
    libgnuradio-blocks3.10 \
    libgnuradio-filter3.10 \
    libgnuradio-digital3.10 \
    libgnuradio-analog3.10 \
    libgnuradio-fec3.10 \
    libgnuradio-vocoder3.10 \
    libvolk2-dev \
    libboost-dev \
    libzmq3-dev \
    libzmq5 \
    libfmt-dev \
    python3-dev \
    python3-numpy
```

### Fedora

```bash
sudo dnf install \
    cmake \
    gcc-c++ \
    pkg-config \
    gnuradio-devel \
    gnuradio \
    volk-devel \
    boost-devel \
    zeromq-devel \
    fmt-devel \
    python3-devel \
    python3-numpy
```

## Verification

After installing dependencies, verify they are available:

```bash
# Check GNU Radio version
gnuradio-config-info --version

# Check pkg-config availability
pkg-config --exists gnuradio-runtime && echo "GNU Radio runtime found"
pkg-config --exists gnuradio-vocoder && echo "GNU Radio vocoder found"
pkg-config --exists libzmq && echo "ZMQ found" || echo "ZMQ not found (optional)"

# Check Volk
pkg-config --exists volk && echo "Volk found"
```

## Build Configuration

The module uses CMake's automatic dependency detection. Dependencies are checked via:

- `find_package()` for CMake-based packages (GNU Radio, Volk, Boost)
- `pkg_check_modules()` for pkg-config based packages (GNU Radio components, ZMQ)

Optional dependencies (like ZMQ) are automatically detected and enabled if available.

## Notes

- GNU Radio must be installed before building this module
- The module links against GNU Radio's shared libraries, so they must be available at runtime
- Python bindings require Python development headers (`python3-dev` or `python3-devel`)
- Codec2 support is provided through gnuradio-vocoder, not as a direct dependency

