#!/bin/bash
# Monitor output file and validate when it's complete

OUTPUT_FILE="/home/haaken/Musikk/ax25+cq.wav"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Monitoring: ${OUTPUT_FILE}"
echo "Please run the GRC flowgraph and click Start"
echo "This script will validate the file when it's complete"
echo ""

# Remove old file if it exists
if [ -f "${OUTPUT_FILE}" ]; then
    OLD_SIZE=$(stat -c%s "${OUTPUT_FILE}")
    if [ "$OLD_SIZE" -le 44 ]; then
        echo "Removing incomplete file (${OLD_SIZE} bytes)..."
        rm -f "${OUTPUT_FILE}"
    fi
fi

# Monitor file size
PREV_SIZE=0
STABLE_COUNT=0

while true; do
    if [ -f "${OUTPUT_FILE}" ]; then
        CURRENT_SIZE=$(stat -c%s "${OUTPUT_FILE}")
        
        if [ "$CURRENT_SIZE" -gt 44 ]; then
            if [ "$CURRENT_SIZE" -eq "$PREV_SIZE" ]; then
                STABLE_COUNT=$((STABLE_COUNT + 1))
                if [ $STABLE_COUNT -ge 5 ]; then
                    echo ""
                    echo "File size stable at ${CURRENT_SIZE} bytes for 5 checks"
                    echo "Assuming flowgraph completed. Validating..."
                    echo ""
                    break
                fi
            else
                STABLE_COUNT=0
                echo "File growing: ${CURRENT_SIZE} bytes..."
            fi
            PREV_SIZE=$CURRENT_SIZE
        fi
    fi
    sleep 2
done

# Validate the file
cd "${SCRIPT_DIR}"
python3 validate_ax25_flowgraph.py "${OUTPUT_FILE}"

exit $?

