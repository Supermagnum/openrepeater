#!/bin/bash
################################################################################
# Code Quality Report Generator
# Runs all code quality tools on Python and shell scripts
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
REPORT_FILE="$PROJECT_ROOT/CODE_QUALITY_REPORT.md"

echo "================================================================"
echo " Code Quality Report"
echo "================================================================"
echo ""
echo "Generating report: $REPORT_FILE"
echo ""

# Start report
cat > "$REPORT_FILE" << 'EOF'
# Code Quality Report

Generated: $(date)

This report contains results from all code quality tools run on the authenticated repeater control system.

## Summary

EOF

# Count files
PYTHON_FILES=$(find "$PROJECT_ROOT" -name "*.py" -type f | grep -v __pycache__ | grep -v ".pyc" | wc -l)
SHELL_FILES=$(find "$PROJECT_ROOT" -name "*.sh" -type f | wc -l)

echo "Found $PYTHON_FILES Python files and $SHELL_FILES shell scripts"
echo ""

# Python files
PYTHON_FILES_LIST=$(find "$PROJECT_ROOT" -name "*.py" -type f | grep -v __pycache__ | grep -v ".pyc")

# Shell files
SHELL_FILES_LIST=$(find "$PROJECT_ROOT" -name "*.sh" -type f)

# Initialize counters
TOTAL_ISSUES=0
CRITICAL_ISSUES=0
HIGH_ISSUES=0

################################################################################
# Python Code Quality
################################################################################

echo "## Python Code Quality" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# Bandit - Security scanner
echo "Running bandit (security scanner)..."
{
    echo "### Bandit (Security Scanner)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    bandit -r "$PROJECT_ROOT/integration" -f json 2>&1 | tee /tmp/bandit_output.json || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Count issues
    if [ -f /tmp/bandit_output.json ]; then
        HIGH_COUNT=$(python3 -c "import json; d=json.load(open('/tmp/bandit_output.json')); print(sum(1 for r in d.get('results', []) if r.get('issue_severity') == 'HIGH'))" 2>/dev/null || echo "0")
        MEDIUM_COUNT=$(python3 -c "import json; d=json.load(open('/tmp/bandit_output.json')); print(sum(1 for r in d.get('results', []) if r.get('issue_severity') == 'MEDIUM'))" 2>/dev/null || echo "0")
        CRITICAL_COUNT=$(python3 -c "import json; d=json.load(open('/tmp/bandit_output.json')); print(sum(1 for r in d.get('results', []) if r.get('issue_severity') == 'CRITICAL'))" 2>/dev/null || echo "0")
        echo "**Issues found:** CRITICAL: $CRITICAL_COUNT, HIGH: $HIGH_COUNT, MEDIUM: $MEDIUM_COUNT" >> "$REPORT_FILE"
        CRITICAL_ISSUES=$((CRITICAL_ISSUES + CRITICAL_COUNT))
        HIGH_ISSUES=$((HIGH_ISSUES + HIGH_COUNT))
    fi
    echo "" >> "$REPORT_FILE"
} || echo "Bandit failed or found issues"

# Flake8 - Style guide
echo "Running flake8 (PEP 8 style checker)..."
{
    echo "### Flake8 (PEP 8 Style Checker)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    flake8 "$PROJECT_ROOT/integration" --max-line-length=120 --extend-ignore=E501,W503 2>&1 | tee /tmp/flake8_output.txt || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    FLAKE8_COUNT=$(wc -l < /tmp/flake8_output.txt 2>/dev/null || echo "0")
    echo "**Issues found:** $FLAKE8_COUNT" >> "$REPORT_FILE"
    TOTAL_ISSUES=$((TOTAL_ISSUES + FLAKE8_COUNT))
    echo "" >> "$REPORT_FILE"
} || echo "Flake8 failed or found issues"

# Pylint - Comprehensive analysis
echo "Running pylint (comprehensive analysis)..."
{
    echo "### Pylint (Comprehensive Analysis)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    pylint "$PROJECT_ROOT/integration/authenticated_command_handler.py" --max-line-length=120 --disable=C0103,C0114,C0115,C0116 2>&1 | tail -50 | tee /tmp/pylint_output.txt || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    PYLINT_ERRORS=$(grep -c "error" /tmp/pylint_output.txt 2>/dev/null || echo "0")
    PYLINT_WARNINGS=$(grep -c "warning" /tmp/pylint_output.txt 2>/dev/null || echo "0")
    echo "**Issues found:** Errors: $PYLINT_ERRORS, Warnings: $PYLINT_WARNINGS" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
} || echo "Pylint failed or found issues"

# Vulture - Dead code detection
echo "Running vulture (dead code detection)..."
{
    echo "### Vulture (Dead Code Detection)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    vulture "$PROJECT_ROOT/integration" --min-confidence=80 2>&1 | tee /tmp/vulture_output.txt || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    VULTURE_COUNT=$(wc -l < /tmp/vulture_output.txt 2>/dev/null || echo "0")
    echo "**Unused code found:** $VULTURE_COUNT items" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
} || echo "Vulture failed or found issues"

# Black - Code formatting check
echo "Running black (code formatting check)..."
{
    echo "### Black (Code Formatting Check)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    black --check "$PROJECT_ROOT/integration" 2>&1 | tee /tmp/black_output.txt || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    BLACK_COUNT=$(grep -c "would reformat" /tmp/black_output.txt 2>/dev/null || echo "0")
    echo "**Files needing reformatting:** $BLACK_COUNT" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
} || echo "Black failed or found issues"

# isort - Import sorting check
echo "Running isort (import sorting check)..."
{
    echo "### isort (Import Sorting Check)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    isort --check-only "$PROJECT_ROOT/integration" 2>&1 | tee /tmp/isort_output.txt || true
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    ISORT_COUNT=$(grep -c "would be reformatted" /tmp/isort_output.txt 2>/dev/null || echo "0")
    echo "**Files needing import sorting:** $ISORT_COUNT" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
} || echo "isort failed or found issues"

################################################################################
# Shell Script Quality
################################################################################

echo "## Shell Script Quality" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# Shellcheck
echo "Running shellcheck (shell script analysis)..."
{
    echo "### Shellcheck (Shell Script Analysis)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "\`\`\`" >> "$REPORT_FILE"
    for file in $SHELL_FILES_LIST; do
        echo "Checking: $file"
        shellcheck "$file" 2>&1 || true
    done | tee /tmp/shellcheck_output.txt
    echo "\`\`\`" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    SHELLCHECK_COUNT=$(grep -c "SC" /tmp/shellcheck_output.txt 2>/dev/null || echo "0")
    echo "**Issues found:** $SHELLCHECK_COUNT" >> "$REPORT_FILE"
    TOTAL_ISSUES=$((TOTAL_ISSUES + SHELLCHECK_COUNT))
    echo "" >> "$REPORT_FILE"
} || echo "Shellcheck failed or found issues"

################################################################################
# Summary
################################################################################

cat >> "$REPORT_FILE" << EOF

## Summary

- **Total Issues Found:** $TOTAL_ISSUES
- **Critical Issues:** $CRITICAL_ISSUES
- **High Priority Issues:** $HIGH_ISSUES

### Recommendations

1. **Security Issues (CRITICAL/HIGH):** Must be fixed before production deployment
2. **Functional Bugs:** Must be fixed before production deployment
3. **Style Issues:** Should be fixed for code maintainability
4. **Dead Code:** Document why it exists or remove it

### Next Steps

1. Review all CRITICAL and HIGH priority issues
2. Fix security vulnerabilities
3. Address functional bugs
4. Apply code formatting (black, isort)
5. Remove or document dead code

EOF

echo ""
echo "================================================================"
echo " Code Quality Report Complete"
echo "================================================================"
echo ""
echo "Report saved to: $REPORT_FILE"
echo ""
echo "Summary:"
echo "  Total Issues: $TOTAL_ISSUES"
echo "  Critical: $CRITICAL_ISSUES"
echo "  High Priority: $HIGH_ISSUES"
echo ""

