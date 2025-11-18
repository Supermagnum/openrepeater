#!/bin/bash
# Copyright 2024 QRadioLink Contributors
#
# This script summarizes the results of a fuzzing campaign and writes to markdown file

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <results_directory> [markdown_file]"
    echo "Example: $0 fuzzing/results_20241105_134800 fuzzing-results/fuzzing_results_20241105_134800.md"
    exit 1
fi

RESULTS_DIR="$1"
RESULTS_MD_FILE="$2"

if [ ! -d "${RESULTS_DIR}" ]; then
    echo "Error: Results directory not found: ${RESULTS_DIR}"
    exit 1
fi

# If markdown file not provided, create one in fuzzing-results directory
if [ -z "${RESULTS_MD_FILE}" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    FUZZING_RESULTS_DIR="${SCRIPT_DIR}/../fuzzing-results"
    mkdir -p "${FUZZING_RESULTS_DIR}"
    CAMPAIGN_ID=$(basename "${RESULTS_DIR}" | sed 's/results_//')
    RESULTS_MD_FILE="${FUZZING_RESULTS_DIR}/fuzzing_results_${CAMPAIGN_ID}.md"
fi

# Create markdown file
cat > "${RESULTS_MD_FILE}" << EOF
# Fuzzing Campaign Results

**Campaign ID:** $(basename "${RESULTS_DIR}" | sed 's/results_//')
**Results Directory:** ${RESULTS_DIR}
**Summary Generated:** $(date)

## Campaign Configuration

EOF

# Read config if available
if [ -f "${RESULTS_DIR}/config.txt" ]; then
    cat "${RESULTS_DIR}/config.txt" | sed 's/^/- /' >> "${RESULTS_MD_FILE}"
fi

cat >> "${RESULTS_MD_FILE}" << EOF

## Fuzzer Results Summary

| Fuzzer | Executions | Exec/sec | New Units | Peak RSS (MB) | Coverage | Status |
|--------|------------|----------|-----------|---------------|----------|--------|
EOF

# Process each log file
for log_file in "${RESULTS_DIR}"/*.log; do
    if [ -f "${log_file}" ]; then
        fuzzer_name=$(basename "${log_file}" .log)
        
        # Extract statistics
        executions=$(grep "stat::number_of_executed_units:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
        exec_per_sec=$(grep "stat::average_exec_per_sec:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
        new_units=$(grep "stat::new_units_added:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
        peak_rss=$(grep "stat::peak_rss_mb:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
        
        # Extract coverage
        coverage="N/A"
        if grep -q "cov:" "${log_file}"; then
            last_cov_line=$(grep "cov:" "${log_file}" | tail -1)
            cov_edges=$(echo "${last_cov_line}" | grep -oP 'cov: \K[0-9]+' || echo "")
            cov_features=$(echo "${last_cov_line}" | grep -oP 'ft: \K[0-9]+' || echo "")
            if [ -n "${cov_edges}" ] && [ -n "${cov_features}" ]; then
                coverage="${cov_edges} edges, ${cov_features} features"
            fi
        fi
        
        # Check status
        status="COMPLETED"
        if [ -f "${RESULTS_DIR}/${fuzzer_name}.pid" ]; then
            pid=$(cat "${RESULTS_DIR}/${fuzzer_name}.pid")
            if ps -p "${pid}" > /dev/null 2>&1; then
                status="RUNNING"
            fi
        fi
        
        # Check for issues
        if grep -qi "crash\|abort\|fault" "${log_file}" 2>/dev/null; then
            status="${status} (ISSUES)"
        fi
        
        echo "| ${fuzzer_name} | ${executions} | ${exec_per_sec} | ${new_units} | ${peak_rss} | ${coverage} | ${status} |" >> "${RESULTS_MD_FILE}"
    fi
done

cat >> "${RESULTS_MD_FILE}" << EOF

## Detailed Statistics

EOF

# Add detailed statistics for each fuzzer
for log_file in "${RESULTS_DIR}"/*.log; do
    if [ -f "${log_file}" ]; then
        fuzzer_name=$(basename "${log_file}" .log)
        cat >> "${RESULTS_MD_FILE}" << EOF
### ${fuzzer_name}

EOF
        
        # Extract all statistics
        if grep -q "stat::" "${log_file}"; then
            cat >> "${RESULTS_MD_FILE}" << EOF
- **Total Executions:** $(grep "stat::number_of_executed_units:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
- **Average Exec/sec:** $(grep "stat::average_exec_per_sec:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
- **New Units Added:** $(grep "stat::new_units_added:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A")
- **Peak RSS:** $(grep "stat::peak_rss_mb:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A") MB
- **Slowest Unit Time:** $(grep "stat::slowest_unit_time_sec:" "${log_file}" 2>/dev/null | awk '{print $2}' || echo "N/A") seconds

EOF
        fi
        
        # Extract coverage progression
        if grep -q "cov:" "${log_file}"; then
            cat >> "${RESULTS_MD_FILE}" << EOF
**Coverage Progression:**
\`\`\`
$(grep "cov:" "${log_file}" | tail -5)
\`\`\`

EOF
        fi
        
        # Check for issues
        if grep -qi "crash\|abort\|fault\|error" "${log_file}" 2>/dev/null; then
            cat >> "${RESULTS_MD_FILE}" << EOF
**WARNING:** Potential issues found in log file.

EOF
        fi
        
        cat >> "${RESULTS_MD_FILE}" << EOF
**Log File:** ${log_file}

EOF
    fi
done

# Corpus summary
cat >> "${RESULTS_MD_FILE}" << EOF
## Corpus Summary

| Fuzzer | Files | Size |
|--------|-------|------|
EOF

total_files=0
total_size=0
for corpus_dir in "${RESULTS_DIR}"/*_corpus; do
    if [ -d "${corpus_dir}" ]; then
        fuzzer_name=$(basename "${corpus_dir}" _corpus)
        file_count=$(find "${corpus_dir}" -type f 2>/dev/null | wc -l)
        dir_size_bytes=$(du -sb "${corpus_dir}" 2>/dev/null | awk '{print $1}' || echo "0")
        
        if [ -n "${dir_size_bytes}" ] && [ "${dir_size_bytes}" != "0" ]; then
            dir_size_human=$(numfmt --to=iec-i --suffix=B "${dir_size_bytes}" 2>/dev/null || echo "${dir_size_bytes} B")
        else
            dir_size_human=$(du -sh "${corpus_dir}" 2>/dev/null | awk '{print $1}' || echo "0 B")
        fi
        
        echo "| ${fuzzer_name} | ${file_count} | ${dir_size_human} |" >> "${RESULTS_MD_FILE}"
        total_files=$((total_files + file_count))
        total_size=$((total_size + dir_size_bytes))
    fi
done

if [ ${total_size} -gt 0 ]; then
    total_size_human=$(numfmt --to=iec-i --suffix=B ${total_size} 2>/dev/null || echo "${total_size} B")
else
    total_size_human="0 B"
fi

cat >> "${RESULTS_MD_FILE}" << EOF
| **Total** | **${total_files}** | **${total_size_human}** |

## Process Status

EOF

running_count=0
for pid_file in "${RESULTS_DIR}"/*.pid; do
    if [ -f "${pid_file}" ]; then
        fuzzer_name=$(basename "${pid_file}" .pid)
        pid=$(cat "${pid_file}")
        if ps -p "${pid}" > /dev/null 2>&1; then
            echo "- **${fuzzer_name}**: RUNNING (PID ${pid})" >> "${RESULTS_MD_FILE}"
            running_count=$((running_count + 1))
        else
            echo "- **${fuzzer_name}**: COMPLETED (was PID ${pid})" >> "${RESULTS_MD_FILE}"
        fi
    fi
done

if [ ${running_count} -eq 0 ]; then
    echo "" >> "${RESULTS_MD_FILE}"
    echo "All fuzzers have completed." >> "${RESULTS_MD_FILE}"
else
    echo "" >> "${RESULTS_MD_FILE}"
    echo "${running_count} fuzzer(s) still running." >> "${RESULTS_MD_FILE}"
fi

cat >> "${RESULTS_MD_FILE}" << EOF

## Notes

- Execution speed target: 100+ exec/sec (minimum), 1,000+ exec/sec (acceptable), 5,000+ exec/sec (good)
- Stability target: 95%+ stability
- Coverage should show growth during the campaign
- Any crashes or errors will be noted in the detailed statistics

---

*Generated by summarize_fuzzing.sh on $(date)*

EOF

echo "Summary written to: ${RESULTS_MD_FILE}"
echo ""
echo "To view results:"
echo "  cat ${RESULTS_MD_FILE}"
