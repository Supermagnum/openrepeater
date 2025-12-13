#!/bin/bash
# Wait for output file and validate it

OUTPUT_FILE="/tmp/test_output_ax25.wav"
MAX_WAIT=300  # 5 minutes max wait
CHECK_INTERVAL=2  # Check every 2 seconds

echo "Waiting for output file: ${OUTPUT_FILE}"
echo "Please start the flowgraph GUI and click 'Start'"
echo ""

elapsed=0
while [ $elapsed -lt $MAX_WAIT ]; do
    if [ -f "${OUTPUT_FILE}" ]; then
        file_size=$(stat -c%s "${OUTPUT_FILE}" 2>/dev/null || echo "0")
        if [ "$file_size" -gt 44 ]; then
            echo "Output file detected: ${OUTPUT_FILE} (${file_size} bytes)"
            echo ""
            break
        fi
    fi
    sleep $CHECK_INTERVAL
    elapsed=$((elapsed + CHECK_INTERVAL))
    if [ $((elapsed % 10)) -eq 0 ]; then
        echo "Still waiting... (${elapsed}s elapsed)"
    fi
done

if [ ! -f "${OUTPUT_FILE}" ] || [ "$(stat -c%s "${OUTPUT_FILE}" 2>/dev/null || echo "0")" -le 44 ]; then
    echo "ERROR: Output file not created or too small after ${elapsed} seconds"
    exit 1
fi

echo "Running validation..."
cd /home/haaken/github-projects/authenticated-repeater-control
python3 validate_ax25_flowgraph.py "${OUTPUT_FILE}"

exit $?

