#!/bin/bash
# Copyright 2024 QRadioLink Contributors
#
# This script runs all fuzzing harnesses for 6 hours each using nohup
# Each fuzzer runs in the background and logs to a separate file

set -e

# Configuration
FUZZ_DURATION=21600  # 6 hours in seconds
BUILD_DIR="${BUILD_DIR:-build-fuzz}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HARNESSES_DIR="${SCRIPT_DIR}/harnesses"
CORPUS_BASE_DIR="${SCRIPT_DIR}/corpus"
RESULTS_DIR="${SCRIPT_DIR}/results_$(date +%Y%m%d_%H%M%S)"
FUZZER_BIN_DIR="${SCRIPT_DIR}/../${BUILD_DIR}/fuzzing/harnesses"
FUZZING_RESULTS_DIR="${SCRIPT_DIR}/../fuzzing-results"
RESULTS_MD_FILE="${FUZZING_RESULTS_DIR}/fuzzing_results_$(date +%Y%m%d_%H%M%S).md"

# CPU cores available for parallel fuzzing
# Each fuzzer can use -jobs=N to spawn multiple workers
# With 15 cores and 10 fuzzers, we can use -jobs=1 per fuzzer (10 total) or -jobs=2 for some
AVAILABLE_CORES="${AVAILABLE_CORES:-15}"
JOBS_PER_FUZZER="${JOBS_PER_FUZZER:-1}"  # Jobs per fuzzer (1 = no parallel workers within fuzzer)

# Fuzzers that need longer timeout due to FFT filter initialization
FUZZERS_LONG_TIMEOUT=("fuzz_demod_2fsk" "fuzz_demod_bpsk" "fuzz_demod_qpsk")
LONG_TIMEOUT=30  # seconds
STANDARD_TIMEOUT=10  # seconds

# Per-fuzzer job configuration (for fuzzers that need more parallelism)
# Default is JOBS_PER_FUZZER, but can override per-fuzzer
declare -A FUZZER_JOBS
# Increase jobs for struggling fuzzers (may help with throughput, but be careful with mutex contention)
FUZZER_JOBS["fuzz_demod_2fsk"]=2  # Struggling fuzzer - try 2 workers for better throughput
FUZZER_JOBS["fuzz_demod_bpsk"]=1  # Keep at 1 due to mutex contention
FUZZER_JOBS["fuzz_demod_qpsk"]=1  # Keep at 1 due to mutex contention

# Per-fuzzer timeout override (for fuzzers that need even longer timeout)
declare -A FUZZER_TIMEOUTS
FUZZER_TIMEOUTS["fuzz_demod_2fsk"]=60  # Struggling fuzzer - increase to 60s

# List of all fuzzers
FUZZERS=(
    "fuzz_mod_2fsk"
    "fuzz_mod_4fsk"
    "fuzz_mod_bpsk"
    "fuzz_mod_qpsk"
    "fuzz_demod_2fsk"
    "fuzz_demod_4fsk"
    "fuzz_demod_bpsk"
    "fuzz_demod_qpsk"
    "fuzz_clipper_cc"
    "fuzz_dsss_encoder"
)

# Create results directories
mkdir -p "${RESULTS_DIR}"
mkdir -p "${FUZZING_RESULTS_DIR}"
echo "Results directory: ${RESULTS_DIR}"
echo "Results markdown file: ${RESULTS_MD_FILE}"

# Check if fuzzer binaries exist
if [ ! -d "${FUZZER_BIN_DIR}" ]; then
    echo "Error: Fuzzer binary directory not found: ${FUZZER_BIN_DIR}"
    echo "Please build the fuzzers first (cd .. && mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR} && cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && make)"
    exit 1
fi

# Function to check if fuzzer needs long timeout
needs_long_timeout() {
    local fuzzer_name=$1
    for long_timeout_fuzzer in "${FUZZERS_LONG_TIMEOUT[@]}"; do
        if [ "${fuzzer_name}" = "${long_timeout_fuzzer}" ]; then
            return 0
        fi
    done
    return 1
}

# Function to run a single fuzzer
run_fuzzer() {
    local fuzzer_name=$1
    local fuzzer_bin="${FUZZER_BIN_DIR}/${fuzzer_name}"
    local corpus_dir="${RESULTS_DIR}/${fuzzer_name}_corpus"
    local log_file="${RESULTS_DIR}/${fuzzer_name}.log"
    local seed_file="${CORPUS_BASE_DIR}/${fuzzer_name}_seed"
    
    # Determine timeout value
    local timeout_value="${STANDARD_TIMEOUT}"
    if needs_long_timeout "${fuzzer_name}"; then
        timeout_value="${LONG_TIMEOUT}"
    fi
    
    # Check for per-fuzzer timeout override
    if [ -n "${FUZZER_TIMEOUTS[${fuzzer_name}]}" ]; then
        timeout_value="${FUZZER_TIMEOUTS[${fuzzer_name}]}"
        echo "Using custom timeout (${timeout_value}s) for ${fuzzer_name}"
    elif needs_long_timeout "${fuzzer_name}"; then
        echo "Using extended timeout (${timeout_value}s) for ${fuzzer_name}"
    fi
    
    # Check if fuzzer binary exists
    if [ ! -f "${fuzzer_bin}" ]; then
        echo "Warning: Fuzzer binary not found: ${fuzzer_bin}"
        echo "Skipping ${fuzzer_name}"
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
    
    # Build fuzzer command with optional parallel jobs
    local fuzzer_cmd="${fuzzer_bin}"
    local fuzzer_args=(
        "-print_final_stats=1"
        "-timeout=${timeout_value}"
        "-rss_limit_mb=2000"
        "-max_total_time=${FUZZ_DURATION}"
    )
    
    # Determine jobs for this specific fuzzer
    local fuzzer_jobs="${JOBS_PER_FUZZER}"
    if [ -n "${FUZZER_JOBS[${fuzzer_name}]}" ]; then
        fuzzer_jobs="${FUZZER_JOBS[${fuzzer_name}]}"
    fi
    
    # Add parallel jobs if configured
    if [ "${fuzzer_jobs}" -gt 1 ]; then
        fuzzer_args+=("-jobs=${fuzzer_jobs}")
        echo "Using ${fuzzer_jobs} parallel workers for ${fuzzer_name}"
    fi
    
    fuzzer_args+=("${corpus_dir}/")
    
    # Run fuzzer with nohup
    echo "Starting ${fuzzer_name} for ${FUZZ_DURATION} seconds (6 hours)..."
    nohup timeout "${FUZZ_DURATION}" "${fuzzer_cmd}" "${fuzzer_args[@]}" \
        > "${log_file}" 2>&1 &
    
    local pid=$!
    echo "${fuzzer_name} started with PID ${pid}"
    echo "${pid}" > "${RESULTS_DIR}/${fuzzer_name}.pid"
    
    return 0
}

# Main execution
echo "=========================================="
echo "QRadioLink Fuzzing Campaign (6 hours)"
echo "=========================================="
echo "Start time: $(date)"
echo "Duration: ${FUZZ_DURATION} seconds (6 hours)"
echo "Fuzzers: ${#FUZZERS[@]}"
echo "Available CPU cores: ${AVAILABLE_CORES}"
echo "Jobs per fuzzer: ${JOBS_PER_FUZZER}"
echo "Extended timeout fuzzers: ${FUZZERS_LONG_TIMEOUT[*]} (${LONG_TIMEOUT}s)"
echo "Standard timeout: ${STANDARD_TIMEOUT}s"
echo ""

# Start all fuzzers
for fuzzer in "${FUZZERS[@]}"; do
    run_fuzzer "${fuzzer}"
    sleep 1  # Small delay between starting fuzzers
done

echo ""
echo "=========================================="
echo "All fuzzers started"
echo "=========================================="
echo "Results directory: ${RESULTS_DIR}"
echo "Log files: ${RESULTS_DIR}/*.log"
echo "PID files: ${RESULTS_DIR}/*.pid"
echo "Corpus directories: ${RESULTS_DIR}/*_corpus/"
echo ""
echo "To check status:"
echo "  tail -f ${RESULTS_DIR}/*.log"
echo ""
echo "To stop a fuzzer:"
echo "  kill \$(cat ${RESULTS_DIR}/<fuzzer_name>.pid)"
echo ""
echo "To stop all fuzzers:"
echo "  for pid in ${RESULTS_DIR}/*.pid; do kill \$(cat \$pid) 2>/dev/null; done"
echo ""
echo "To view summary after completion:"
echo "  ${SCRIPT_DIR}/summarize_fuzzing.sh ${RESULTS_DIR}"
echo ""

# Create a status check script
cat > "${RESULTS_DIR}/check_status.sh" << 'EOF'
#!/bin/bash
RESULTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "=== Fuzzer Status ==="
for pid_file in "${RESULTS_DIR}"/*.pid; do
    if [ -f "${pid_file}" ]; then
        fuzzer_name=$(basename "${pid_file}" .pid)
        pid=$(cat "${pid_file}")
        if ps -p "${pid}" > /dev/null 2>&1; then
            echo "${fuzzer_name}: RUNNING (PID ${pid})"
        else
            echo "${fuzzer_name}: STOPPED (was PID ${pid})"
        fi
    fi
done
EOF

chmod +x "${RESULTS_DIR}/check_status.sh"

echo "Status check script created: ${RESULTS_DIR}/check_status.sh"
echo ""

# Save configuration
cat > "${RESULTS_DIR}/config.txt" << EOF
Fuzzing Campaign Configuration
==============================
Start time: $(date)
Duration: ${FUZZ_DURATION} seconds (6 hours)
Build directory: ${BUILD_DIR}
Fuzzer binary directory: ${FUZZER_BIN_DIR}
Fuzzers: ${#FUZZERS[@]}
EOF

echo "Configuration saved to: ${RESULTS_DIR}/config.txt"
echo ""

# Create initial markdown results file
cat > "${RESULTS_MD_FILE}" << EOF
# Fuzzing Campaign Results

**Campaign ID:** $(date +%Y%m%d_%H%M%S)
**Start Time:** $(date)
**Duration:** ${FUZZ_DURATION} seconds (6 hours)
**Fuzzers:** ${#FUZZERS[@]}

## Campaign Configuration

- Build directory: ${BUILD_DIR}
- Fuzzer binary directory: ${FUZZER_BIN_DIR}
- Results directory: ${RESULTS_DIR}
- Available CPU cores: ${AVAILABLE_CORES}
- Jobs per fuzzer: ${JOBS_PER_FUZZER}
- Extended timeout fuzzers: ${FUZZERS_LONG_TIMEOUT[*]} (${LONG_TIMEOUT}s timeout)
- Standard timeout: ${STANDARD_TIMEOUT}s
- Expected end time: $(date -d "+${FUZZ_DURATION} seconds" 2>/dev/null || echo "N/A")

## Fuzzers Status

| Fuzzer | Status | PID | Log File |
|--------|--------|-----|----------|
EOF

# Add initial status for each fuzzer
for fuzzer in "${FUZZERS[@]}"; do
    echo "| ${fuzzer} | STARTING | - | ${RESULTS_DIR}/${fuzzer}.log |" >> "${RESULTS_MD_FILE}"
done

cat >> "${RESULTS_MD_FILE}" << EOF

## Results Summary

Results will be updated as fuzzers complete. Check back after the campaign finishes.

## Detailed Logs

See individual log files in: ${RESULTS_DIR}/

EOF

echo "Results markdown file created: ${RESULTS_MD_FILE}"
echo ""
echo "Campaign started successfully!"
echo "End time (expected): $(date -d "+${FUZZ_DURATION} seconds" 2>/dev/null || echo "N/A")"
echo ""
echo "To view results:"
echo "  cat ${RESULTS_MD_FILE}"
echo ""
echo "To update results after completion:"
echo "  ${SCRIPT_DIR}/summarize_fuzzing.sh ${RESULTS_DIR} ${RESULTS_MD_FILE}"

