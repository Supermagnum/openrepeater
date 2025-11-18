# Fuzzing Script Usage

## Basic Usage

Run all fuzzers for 6 hours:

```bash
./run_fuzzers_6h.sh
```

## Configuration Options

### Environment Variables

You can customize the fuzzing campaign using environment variables:

```bash
# Set number of CPU cores available (default: 15)
export AVAILABLE_CORES=15

# Set parallel jobs per fuzzer (default: 1)
# With 15 cores and 10 fuzzers, you can use:
# - JOBS_PER_FUZZER=1: 10 fuzzers, 1 worker each = 10 total processes
# - JOBS_PER_FUZZER=2: 10 fuzzers, 2 workers each = 20 total processes (may oversubscribe)
export JOBS_PER_FUZZER=1

# Set custom build directory (default: build-fuzz)
export BUILD_DIR=build-fuzz

# Run with custom settings
AVAILABLE_CORES=15 JOBS_PER_FUZZER=1 ./run_fuzzers_6h.sh
```

## Timeout Configuration

The script automatically uses extended timeouts for fuzzers that have FFT filter initialization issues:

- `fuzz_demod_2fsk` - 60s timeout (custom, struggling fuzzer)
- `fuzz_demod_bpsk` - 30s timeout
- `fuzz_demod_qpsk` - 30s timeout

All other fuzzers use the standard 10-second timeout.

## Per-Fuzzer Optimizations

The script includes per-fuzzer optimizations for fuzzers that need special handling:

- `fuzz_demod_2fsk`: 60s timeout + 2 parallel workers (optimized for better throughput)
- `fuzz_demod_bpsk`: 30s timeout + 1 worker (stable configuration)
- `fuzz_demod_qpsk`: 30s timeout + 1 worker (stable configuration)

These optimizations can be adjusted in the `run_fuzzers_6h.sh` script by modifying the `FUZZER_TIMEOUTS` and `FUZZER_JOBS` arrays.

## Parallel Fuzzing

With 15 CPU cores available:

- **Conservative (recommended)**: `JOBS_PER_FUZZER=1`
  - 10 fuzzers × 1 worker = 10 processes
  - Leaves 5 cores for system overhead
  - Best for stability

- **Aggressive**: `JOBS_PER_FUZZER=2`
  - 10 fuzzers × 2 workers = 20 processes
  - May oversubscribe cores (hyperthreading helps)
  - Higher throughput but more resource contention

- **Maximum**: `JOBS_PER_FUZZER=2` for some fuzzers
  - Can be configured per-fuzzer if needed

## Monitoring

Check fuzzer status:

```bash
cd fuzzing/results_YYYYMMDD_HHMMSS
./check_status.sh
```

View live logs:

```bash
tail -f fuzzing/results_YYYYMMDD_HHMMSS/*.log
```

## Results

Results are saved to:
- Individual logs: `fuzzing/results_YYYYMMDD_HHMMSS/*.log`
- Markdown summary: `fuzzing-results/fuzzing_results_YYYYMMDD_HHMMSS.md`
- Corpus files: `fuzzing/results_YYYYMMDD_HHMMSS/*_corpus/`

Generate final summary:

```bash
cd fuzzing
./summarize_fuzzing.sh results_YYYYMMDD_HHMMSS
```
