# dudect Side-Channel Analysis for gr-linux-crypto

This directory contains dudect (dude, is my code constant time?) tests adapted for gr-linux-crypto encryption functions.

## Overview

dudect performs statistical timing analysis to detect side-channel vulnerabilities in cryptographic code. It uses Welch's t-test to determine if execution time varies with input data, which would indicate timing side-channels.

**Reference Paper:**
> Oscar Reparaz, Josep Balasch and Ingrid Verbauwhede  
> [dude, is my code constant time?](https://eprint.iacr.org/2016/1123.pdf)  
> DATE 2017

## Test Targets

### 1. Authentication Tag Comparison (`dut_linux_crypto_auth.c`)

**Purpose:** Test whether authentication tag comparison is constant-time.

**Criticality:** HIGH - Authentication tag comparison must be constant-time to prevent timing attacks.

**Test Function:** `constant_time_compare()` - Constant-time tag comparison implementation

**Test Methodology:**
- Class 0: Identical tags (matching case)
- Class 1: Different tags (non-matching case)
- Measures timing difference between matching and non-matching comparisons

**Results Interpretation:**
- `max t < 5`: No evidence of timing leakage (good)
- `5 <= max t < 10`: Possibly not constant-time (warning)
- `max t >= 10`: Definitely not constant-time (FAIL)

### 2. Encryption Timing (`dut_linux_crypto_encrypt.c`)

**Purpose:** Test whether encryption timing varies with input data patterns.

**Criticality:** MEDIUM - Encryption should ideally be constant-time, but some variation may be acceptable.

**Test Function:** OpenSSL AES-256-GCM encryption (simulating gr-linux-crypto encryption path)

**Test Methodology:**
- Class 0: All-zero input (special pattern)
- Class 1: Random input (normal case)
- Measures timing difference between special patterns and random data

**Results Interpretation:**
- `max t < 5`: No evidence of timing leakage (good)
- `5 <= max t < 10`: Possibly not constant-time (warning)
- `max t >= 10`: Definitely not constant-time (FAIL)

## Building and Running

### Prerequisites

```bash
# Install dependencies
sudo apt-get install libssl-dev  # For OpenSSL

# dudect is already cloned in tests/dudect/
```

### Build

```bash
cd tests/dudect
make -f Makefile.gr-linux-crypto all
```

This builds:
- `dudect_linux_crypto_auth` - Authentication tag comparison test
- `dudect_linux_crypto_encrypt` - Encryption timing test

### Run Tests

```bash
# Run both tests (with timeout)
make -f Makefile.gr-linux-crypto test

# Or run individually:
timeout 300 ./dudect_linux_crypto_auth
timeout 300 ./dudect_linux_crypto_encrypt
```

**Note:** Tests may run indefinitely if no leakage is detected. Use `timeout` to limit execution time.

## Test Results

### Authentication Tag Comparison Test

**Date:** 2025-01-XX  
**Test Duration:** 60 seconds  
**Measurements:** ~17.5 million

**Results:**
- **Max t-statistic:** +1.37 (at 17.5M measurements)
- **Conclusion:** No timing leakage detected
- **Status:** PASS - Well below threshold (t < 5)

**Analysis:**
- Maximum t-statistic remained well below 5 throughout testing
- No evidence of timing leakage in tag comparison
- Constant-time comparison implementation appears effective

**Sample Output:**
```
meas:   17.50 M, max t:   +1.34, max tau: 3.21e-04, (5/tau)^2: 2.30e+08. 
For the moment, maybe constant time.
```

### Encryption Timing Test

**Date:** 2025-01-XX  
**Test Duration:** 60 seconds  
**Measurements:** ~17.5 million

**Results:**
- **Max t-statistic:** +2.30 (at 6.5M measurements)
- **Conclusion:** No significant timing leakage detected
- **Status:** PASS - Well below threshold (t < 5)

**Analysis:**
- Maximum t-statistic remained below 5 throughout testing
- Some timing variation may exist but is minimal
- OpenSSL AES-256-GCM appears to have low timing leakage

**Sample Output:**
```
meas:   16.99 M, max t:   +1.95, max tau: 4.67e-04, (5/tau)^2: 1.15e+08. 
For the moment, maybe constant time.
```

## Interpretation Guidelines

### t-Statistic Values

- **t < 5:** No evidence of timing leakage (safe)
- **5 ≤ t < 10:** Possibly not constant-time (investigate)
- **t ≥ 10:** Definitely not constant-time (timing attack possible)

### Measurement Requirements

- **Minimum:** 10,000 measurements before drawing conclusions
- **Recommended:** 1+ million measurements for confidence
- **Extended:** 10+ million for high-security applications

### Important Notes

1. **No Leakage Detected ≠ Constant-Time Guaranteed**
   - dudect tests for statistical evidence of leakage
   - Absence of evidence is not evidence of absence
   - Multiple test runs recommended

2. **Platform Dependencies**
   - Results may vary by CPU architecture
   - Compiler optimization affects timing
   - OS scheduling can introduce noise

3. **Limitations**
   - Tests simplified models (not full gr-linux-crypto path)
   - Python overhead not included in these tests
   - Actual implementation may differ

## Recommendations

### For Authentication Tag Comparison
**Current Status:** No timing leakage detected

**Recommendations:**
- Continue using constant-time comparison in production code
- Verify that Python cryptography library uses constant-time comparison
- Review actual tag verification code path

### For Encryption Operations
**Current Status:** Low timing leakage detected

**Recommendations:**
- OpenSSL AES-256-GCM shows minimal timing variation (acceptable)
- Consider hardware acceleration (AES-NI) which is typically constant-time
- For critical applications, consider verified constant-time implementations

### General Recommendations

1. **Regular Testing:**
   - Run dudect tests periodically
   - Test after compiler/OS updates
   - Verify on target hardware

2. **Multiple Platforms:**
   - Test on x86_64 (current)
   - Test on ARM64 if applicable
   - Test on embedded platforms if used

3. **Extended Testing:**
   - Run longer tests (hours) for high-security applications
   - Test with crafted input vectors
   - Consider power analysis for hardware implementations

## Files

- `dut_linux_crypto_auth.c` - Authentication tag comparison test
- `dut_linux_crypto_encrypt.c` - Encryption timing test
- `Makefile.gr-linux-crypto` - Build configuration
- `dudect_auth_results.txt` - Authentication test output
- `dudect_encrypt_results.txt` - Encryption test output

## References

- dudect Repository: https://github.com/oreparaz/dudect
- Original Paper: https://eprint.iacr.org/2016/1123.pdf
- dudect Documentation: See `README.md` in this directory

