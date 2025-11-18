# Fuzzing Framework for gr-qradiolink

This directory contains libFuzzer-based fuzzing harnesses for testing the gr-qradiolink module.

## Overview

The fuzzing framework uses LLVM's libFuzzer to perform coverage-guided fuzzing of the actual block implementations. Each fuzzing harness creates a minimal GNU Radio flowgraph to exercise the block processing code.

## Requirements

- **Clang compiler** (libFuzzer is part of LLVM/Clang)
- **AddressSanitizer** (ASAN) - enabled automatically
- GNU Radio >= 3.10 with all dependencies

## Building Fuzzers

To build the fuzzing targets, configure CMake with Clang and enable fuzzing:

```bash
cd build
CC=clang CXX=clang++ cmake -DENABLE_FUZZING=ON ..
make
```

The fuzzing targets will be built in `build/fuzzing/harnesses/`.

## Available Fuzzers

### Modulator Fuzzers

- **fuzz_mod_2fsk** - Tests `mod_2fsk` with byte input data
- **fuzz_mod_4fsk** - Tests `mod_4fsk` with byte input data
- **fuzz_mod_bpsk** - Tests `mod_bpsk` with byte input data
- **fuzz_mod_qpsk** - Tests `mod_qpsk` with byte input data

### Demodulator Fuzzers

- **fuzz_demod_2fsk** - Tests `demod_2fsk` with complex input data
- **fuzz_demod_4fsk** - Tests `demod_4fsk` with complex input data
- **fuzz_demod_bpsk** - Tests `demod_bpsk` with complex input data
- **fuzz_demod_qpsk** - Tests `demod_qpsk` with complex input data

### Supporting Block Fuzzers

- **fuzz_clipper_cc** - Tests CESSB clipper block with complex input
- **fuzz_dsss_encoder** - Tests DSSS encoder with byte input

## Running Fuzzers

### Basic Fuzzing

Run a fuzzer with a seed corpus:

```bash
cd build/fuzzing/harnesses
./fuzz_mod_2fsk ../../fuzzing/corpus/fuzz_mod_2fsk_seed
```

### Fuzzing with Corpus Directory

Create a corpus directory and run fuzzing:

```bash
mkdir -p corpus/fuzz_mod_2fsk
cp ../../fuzzing/corpus/fuzz_mod_2fsk_seed corpus/fuzz_mod_2fsk/
./fuzz_mod_2fsk corpus/fuzz_mod_2fsk/
```

### Advanced Options

```bash
# Run with timeout and memory limit
./fuzz_mod_2fsk -timeout=10 -rss_limit_mb=2000 corpus/fuzz_mod_2fsk/

# Run with specific number of runs
./fuzz_mod_2fsk -runs=100000 corpus/fuzz_mod_2fsk/

# Run with verbose output
./fuzz_mod_2fsk -print_final_stats=1 corpus/fuzz_mod_2fsk/
```

## Seed Corpus

The `corpus/` directory contains seed input files for each fuzzer. These are minimal valid inputs that help guide the fuzzer toward interesting code paths.

## Fuzzing Harnesses

Each harness:

1. Creates a GNU Radio top_block
2. Instantiates the block under test
3. Creates a vector_source from fuzzer input
4. Uses a head block to limit processing
5. Connects to a null_sink to discard output
6. Runs the flowgraph until completion

The harnesses catch all exceptions to prevent crashes from stopping fuzzing, allowing libFuzzer to continue exploring the input space.

## Performance Guidelines

Based on AFL++ performance rules:

- **Execution Speed**: Target 100+ exec/sec (minimum), 1,000+ exec/sec (acceptable), 5,000+ exec/sec (good)
- **Stability**: Minimum 95%+ stability
- **Coverage Progress**: Should see new paths in first hour
- **Red Flags**: < 100 exec/sec, < 95% stability, zero path discovery after initial run

## Interpreting Results

- **Crashes**: Check `crash-*` files for inputs that caused crashes
- **Timeout**: Check `timeout-*` files for inputs that took too long
- **Coverage**: Use `-dump_coverage=1` to generate coverage reports
- **Stats**: Monitor `-print_final_stats=1` for execution statistics

## Long-Running Fuzzing Campaigns

### Running All Fuzzers for 6 Hours

Use the automated script to run all fuzzers for 6 hours in the background:

```bash
cd fuzzing
./run_fuzzers_6h.sh
```

This script will:
- Start all 10 fuzzers using `nohup` (detached from terminal)
- Run each fuzzer for 6 hours (21600 seconds)
- Create separate corpus directories for each fuzzer
- Log output to individual log files
- Save PID files for process management

The script creates a timestamped results directory (e.g., `results_20241105_134800/`) containing:
- Individual log files for each fuzzer (`*.log`)
- Corpus directories (`*_corpus/`)
- PID files (`*.pid`)
- Status check script (`check_status.sh`)
- Configuration file (`config.txt`)

### Monitoring Fuzzing Progress

Check the status of all fuzzers:

```bash
cd fuzzing/results_YYYYMMDD_HHMMSS
./check_status.sh
```

View live logs:

```bash
tail -f fuzzing/results_YYYYMMDD_HHMMSS/*.log
```

### Stopping Fuzzers

Stop a specific fuzzer:

```bash
kill $(cat fuzzing/results_YYYYMMDD_HHMMSS/fuzz_mod_2fsk.pid)
```

Stop all fuzzers:

```bash
for pid in fuzzing/results_YYYYMMDD_HHMMSS/*.pid; do
    kill $(cat "$pid") 2>/dev/null
done
```

### Summarizing Results

After the fuzzing campaign completes, generate a summary:

```bash
cd fuzzing
./summarize_fuzzing.sh results_YYYYMMDD_HHMMSS
```

The summary includes:
- Execution statistics for each fuzzer
- Coverage information
- Corpus file counts and sizes
- Process status

## Continuous Fuzzing

For continuous fuzzing, consider:

1. Running fuzzers in parallel instances
2. Using `-jobs=N` for parallel execution
3. Periodically syncing corpus directories
4. Monitoring coverage growth over time

## Troubleshooting

### Fuzzer is too slow

- Reduce input size limits in harness
- Simplify the flowgraph
- Check for infinite loops (use head blocks)

### No coverage growth

- Verify seed corpus files are valid
- Check that blocks are actually processing data
- Review harness logic for correctness

### Build failures

- Ensure Clang compiler is being used
- Verify `-fsanitize=fuzzer,address` flags are set
- Check GNU Radio dependencies are available

