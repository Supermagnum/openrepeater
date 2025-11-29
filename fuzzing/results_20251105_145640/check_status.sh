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
