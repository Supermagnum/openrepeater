#!/bin/bash
# Test script to run the GRC flowgraph and validate output

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="${SCRIPT_DIR}/examples"
INPUT_AUDIO="/home/haaken/Musikk/cq.wav"
OUTPUT_AUDIO="/tmp/test_output_ax25.wav"

echo "======================================================================"
echo "Running GRC Flowgraph Test"
echo "======================================================================"
echo "Input audio: ${INPUT_AUDIO}"
echo "Output audio: ${OUTPUT_AUDIO}"
echo ""

# Check if input file exists
if [ ! -f "${INPUT_AUDIO}" ]; then
    echo "ERROR: Input audio file not found: ${INPUT_AUDIO}"
    exit 1
fi

# Remove old output file if it exists
rm -f "${OUTPUT_AUDIO}"

# Run the flowgraph (it will open a GUI, so we'll need to set parameters)
# For now, let's modify the Python script to accept command-line arguments
cd "${EXAMPLES_DIR}"

echo "Running flowgraph..."
echo "Note: This will open a GUI window. Please:"
echo "  1. Set 'Audio File Path' to: ${INPUT_AUDIO}"
echo "  2. Set 'Output Audio File Path' to: ${OUTPUT_AUDIO}"
echo "  3. Click 'Start' button"
echo "  4. Wait for completion"
echo "  5. Close the window"
echo ""
echo "Press Enter when ready to start..."
read

python3 tx_audio_signed.py

echo ""
echo "Checking if output file was created..."
if [ ! -f "${OUTPUT_AUDIO}" ]; then
    echo "ERROR: Output file was not created: ${OUTPUT_AUDIO}"
    exit 1
fi

echo "Output file created: ${OUTPUT_AUDIO}"
echo "File size: $(stat -c%s "${OUTPUT_AUDIO}") bytes"
echo ""

# Now validate the output
echo "======================================================================"
echo "Validating Output WAV File"
echo "======================================================================"
cd "${SCRIPT_DIR}"
python3 validate_ax25_flowgraph.py "${OUTPUT_AUDIO}"

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "======================================================================"
    echo "TEST PASSED"
    echo "======================================================================"
else
    echo ""
    echo "======================================================================"
    echo "TEST FAILED"
    echo "======================================================================"
fi

exit $EXIT_CODE

