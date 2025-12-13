#!/bin/bash
# Copyright 2024 QRadioLink Contributors
#
# This script runs the new fuzzing harnesses (m17_deframer, demod_gmsk, demod_dsss)
# for 6 hours each using nohup

set -e

# Configuration
FUZZ_DURATION=21600  # 6 hours in seconds
BUILD_DIR="${BUILD_DIR:-build-fuzz}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CORPUS_BASE_DIR="${SCRIPT_DIR}/corpus"
RESULTS_DIR="${SCRIPT_DIR}/results_new_$(date +%Y%m%d_%H%M%S)"
FUZZER_BIN_DIR="${SCRIPT_DIR}/../${BUILD_DIR}/fuzzing/harnesses"

# New fuzzers to run
NEW_FUZZERS=(
    "fuzz_m17_deframer"
    "fuzz_demod_gmsk"
    "fuzz_demod_dsss"
)

# Create results directory
mkdir -p "${RESULTS_DIR}"
echo "Results directory: ${RESULTS_DIR}"
echo ""

# Function to run a single fuzzer
run_fuzzer() {
    local fuzzer_name="$1"
    local fuzzer_bin="${FUZZER_BIN_DIR}/${fuzzer_name}"
    local corpus_dir="${RESULTS_DIR}/${fuzzer_name}_corpus"
    local seed_file="${CORPUS_BASE_DIR}/${fuzzer_name}_seed"
    local log_file="${RESULTS_DIR}/${fuzzer_name}.log"
    local pid_file="${RESULTS_DIR}/${fuzzer_name}.pid"
    
    # Check if fuzzer binary exists
    if [ ! -f "${fuzzer_bin}" ]; then
        echo "Error: Fuzzer binary not found: ${fuzzer_bin}"
        echo "Please build the fuzzers first:"
        echo "  cd ${BUILD_DIR} && make ${fuzzer_name}"
        return 1
    fi
    
    # Create corpus directory
    mkdir -p "${corpus_dir}"
    
    # Copy seed file if it exists
    if [ -f "${seed_file}" ]; then
        cp "${seed_file}" "${corpus_dir}/"
        echo "Copied seed file for ${fuzzer_name}"
    else
        echo "Warning: Seed file not found: ${seed_file}"
    fi
    
    # Build fuzzer command
    local fuzzer_cmd="${fuzzer_bin}"
    local fuzzer_args=(
        "-print_final_stats=1"
        "-timeout=10"
        "-rss_limit_mb=2000"
        "-max_total_time=${FUZZ_DURATION}"
        "${corpus_dir}/"
    )
    
    # Run fuzzer with nohup
    echo "Starting ${fuzzer_name} for ${FUZZ_DURATION} seconds (6 hours)..."
    nohup timeout "${FUZZ_DURATION}" "${fuzzer_cmd}" "${fuzzer_args[@]}" \
        > "${log_file}" 2>&1 &
    
    local pid=$!
    echo "${pid}" > "${pid_file}"
    echo "  PID: ${pid}"
    echo "  Log: ${log_file}"
    echo "  Corpus: ${corpus_dir}"
    echo ""
}

# Run all new fuzzers
echo "Starting ${#NEW_FUZZERS[@]} new fuzzers for 6 hours each..."
echo ""

for fuzzer in "${NEW_FUZZERS[@]}"; do
    run_fuzzer "${fuzzer}"
    sleep 2  # Small delay between starting fuzzers
done

echo "All fuzzers started!"
echo ""
echo "To check status:"
echo "  ps aux | grep fuzz"
echo ""
echo "To view logs:"
echo "  tail -f ${RESULTS_DIR}/*.log"
echo ""
echo "To stop a fuzzer:"
echo "  kill \$(cat ${RESULTS_DIR}/fuzz_<name>.pid)"
echo ""
echo "Results directory: ${RESULTS_DIR}"

