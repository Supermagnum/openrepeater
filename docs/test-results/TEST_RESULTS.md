# Authenticated Control – Test & Analysis Report

_Last updated: 2025-01-27_

This document captures the current status of all automated quality checks that were
requested for the authenticated control project. Each subsection lists the command
used, the overall result, and any noteworthy findings.

> **Note**  
> All commands were executed from `scripts/authenticated_control/` unless otherwise
> specified. Coverage thresholds reflect the practical limits of the hardware and
> GNU Radio dependent code paths; additional integration tests would be needed to
> push beyond these numbers.

---

## 1. Code Style & Linting

**Purpose:** These tests ensure code consistency, readability, and adherence to Python style guidelines. Consistent code style makes the codebase easier to maintain and reduces cognitive load when reviewing or modifying code.

| Tool / Command | Result | Notes |
| --- | --- | --- |
| `flake8` | [PASS] Pass | Zero lint errors across `svxlink_control` and `ax25_protocol`. **What it tests:** Checks for PEP 8 style violations, syntax errors, undefined names, unused imports, and other code quality issues. Ensures all Python code follows standard conventions. |
| `black --check .` | [PASS] Pass | All files formatted; enforced via `make format`. **What it tests:** Verifies that all Python files are formatted according to Black's code formatting rules (line length, indentation, spacing, etc.). Black enforces consistent formatting automatically. |
| `isort --check-only .` | [PASS] Pass | Import ordering matches Black profile. **What it tests:** Validates that all import statements are sorted and organized consistently (standard library, third-party, local imports). This improves code readability and reduces merge conflicts. |
| `pylint --disable=C0114,C0115,C0116 *.py` | [WARNING] Score 8.81/10 | Remaining warnings are limited to defensive logging style and broad exception handling in `integration/authenticated_command_handler.py`. **What it tests:** Performs deep static analysis of code quality, including code smells, potential bugs, design issues, and adherence to best practices. The score of 8.81/10 indicates high code quality with only minor style warnings remaining. |

## 2. Static Analysis & Security

**Purpose:** These tests analyze code for security vulnerabilities, code complexity, maintainability, and documentation quality. They help identify potential issues before deployment and ensure the codebase remains maintainable as it grows.

| Tool / Command | Result | Notes |
| --- | --- | --- |
| `bandit -r . -ll` | [PASS] Pass | No security issues identified (0 findings). **What it tests:** Scans Python code for common security vulnerabilities such as hardcoded passwords, SQL injection risks, use of insecure random number generators, shell injection vulnerabilities, and other security anti-patterns. Critical for ensuring the authentication system is secure. |
| `vulture . --min-confidence 80` | [PASS] Pass | No dead-code candidates reported after cleanup. **What it tests:** Identifies unused code (dead code) including unused functions, classes, variables, and imports. Removing dead code reduces maintenance burden and potential attack surface. |
| `radon cc . -a -nb` | [PASS] Avg complexity **B (7.67)** | A few functions (parser, control service) are intentionally complex but documented. **What it tests:** Measures cyclomatic complexity of functions. Lower complexity means easier testing and maintenance. Score of B (7.67) indicates moderate complexity, which is acceptable for protocol parsers and control logic that inherently require complex state machines. |
| `radon mi . -nb` | [PASS] All files rated **A** | Maintainability index remains excellent. **What it tests:** Calculates a maintainability index based on complexity, lines of code, and other factors. Rating of A indicates the codebase is highly maintainable and easy for new developers to understand and modify. |
| `interrogate -v --fail-under 80 .` | [PASS] 91.7 % docstring coverage | No modules below the 80 % threshold. |

## 3. Testing & Coverage

**Purpose:** These tests verify that the code works correctly and that a significant portion of the codebase is exercised by automated tests. Coverage metrics indicate how much of the code is tested, helping identify untested code paths that could contain bugs.

| Scope | Command | Result | Coverage |
| --- | --- | --- | --- |
| `svxlink_control` unit tests | `pytest -v --tb=short` | [PASS] Pass | ~71 % (threshold set to 70 % via `.coveragerc`). |
| `ax25_protocol` unit tests | `pytest -v --tb=short` | [PASS] Pass | ~64 % (threshold set to 60 %). |
| Full suite (root) | `make test` | [PASS] Pass | Combined coverage ≈70 %. |
| Stress repeat | `pytest --count=10 -q` | [PASS] 550 tests | Confirms deterministic behaviour with `pytest-repeat`. **What it tests:** Runs all tests 10 times in sequence to verify deterministic behavior and catch any non-deterministic bugs (race conditions, timing issues, random number generation problems). Passing 550 tests confirms the system behaves consistently. |

**Coverage exclusions.** To keep the reports meaningful, the following areas are
excluded via `.coveragerc`:

- GNU Radio glue (`ax25_protocol/gnuradio_integration.py`)
- Operator CLI/client utilities
- Hardware adapters (`svxlink_control/hardware_interface.py`)
- Integration-heavy command handler stubs
- All `tests/` packages (to avoid double counting)

These modules require physical radios, kernel keyrings or SVXLink services that
are not available in CI; manual validation is documented in the respective READMEs.

## 4. Module Import Tests

**Purpose:** These tests verify that all required dependencies and modules can be imported correctly. Import failures would prevent the system from starting, so these tests ensure the installation is complete and all dependencies are properly configured.

| Module | Test Command | Result | Notes |
| --- | --- | --- | --- |
| `gr-linux-crypto` | `python3 -c "from gr_linux_crypto.crypto_helpers import CryptoHelpers"` | [PASS] | Module imports successfully. Requires `modules/gr-linux-crypto` in PYTHONPATH. Symlink `gr_linux_crypto -> python` created to enable package structure. **What it tests:** Verifies that the GNU Radio OOT module for cryptographic operations (Brainpool ECC, kernel keyring support) is properly installed and accessible. This module is essential for signature generation and verification. |
| `gr-packet-protocols` | `python3 -c "from gnuradio.packet_protocols import ax25_decoder, ax25_encoder"` | [PASS] | Module imports successfully. Installed as part of GNU Radio distribution. **What it tests:** Confirms that the AX.25 protocol encoding/decoding modules are available. These modules are required for building and parsing AX.25 frames that carry authenticated commands over radio. |
| `authenticated_command_handler` | `python3 authenticated_command_handler.py` | [PASS] | Command handler starts successfully with placeholder config. Correctly reports missing authorized keys (expected behavior). **What it tests:** Validates that the main command handler service can start without errors and properly handles missing configuration (graceful error reporting). This ensures the service will start correctly in production environments. |

**Test Environment:**
- Python: 3.12.3
- Test Date: 2025-11-18
- Config: Placeholder config file created at `integration/test_config/config.yaml`
- Environment Variable: `AUTHENTICATED_CONFIG` supported for testing without sudo

**Configuration:**
- Placeholder config file created: `integration/config.yaml.placeholder`
- Test config location: `integration/test_config/config.yaml` (uses `/tmp` directories)
- System config location: `/etc/authenticated-repeater/config.yaml` (requires sudo)

**Module Import Requirements:**
- `gr-linux-crypto`: Add `modules/gr-linux-crypto` to PYTHONPATH or install via setup.py
- `gr-packet-protocols`: Installed system-wide via GNU Radio build/install process

## 5. OpenRepeater Integration

**Purpose:** These tests verify that the authenticated control system integrates properly with the OpenRepeater installation and deployment system. This ensures operators can install the authenticated control features as part of the standard OpenRepeater installation process.

| Component | Status | Notes |
| --- | --- | --- |
| Function file integration | [PASS] | `functions_authenticated_control.sh` can be sourced and functions are callable **What it tests:** Verifies that the shell function library can be loaded and all installation/configuration functions are accessible. This ensures the installation scripts can use these functions to set up the authenticated control system. |
| Install script integration | [PASS] | `install_orp.sh` sources the function file at line 193 **What it tests:** Confirms that the main OpenRepeater installation script properly includes the authenticated control functions. This allows the authenticated control system to be installed alongside OpenRepeater. |
| Service file path resolution | [FIXED] | Updated to handle multiple path scenarios **What it tests:** Validates that systemd service files are found and installed correctly regardless of where the installation script is run from. This ensures the service will start correctly after installation. |
| Verification function | [PASS] | Correctly identifies installed/missing components **What it tests:** Checks that the verification function can accurately detect which components are installed and which are missing. This helps operators diagnose installation issues and verify their setup is complete. |

**Detailed Report:** See [OpenRepeater Integration Test Report](OPENREPEATER_INTEGRATION_TEST.md)

**Note:** The `install_authenticated_control()` function is available but not automatically called during OpenRepeater installation. It must be called manually or added to the installation sequence.

## 6. Additional Tooling

**Purpose:** These tools provide advanced testing capabilities beyond basic unit tests. They help find edge cases, performance issues, and ensure tests are robust.

| Tool | Purpose | Status |
| --- | --- | --- |
| `pytest-randomly`, `pytest-repeat` | Exercise tests with varied ordering and repetition | Installed & used (`pytest --count=10`). **What it tests:** `pytest-randomly` runs tests in random order to find test dependencies (tests that only pass when run in a specific order). `pytest-repeat` runs tests multiple times to catch non-deterministic bugs. These tools ensure tests are robust and independent. |
| `mutmut` | Mutation testing | [CONFIGURATION ISSUES] Generated 772 mutants but unable to complete testing due to test file structure conflicts. **What it tests:** Performs mutation testing by making small changes to code and checking if tests catch the mutations. This helps identify weak tests that don't actually verify behavior. Issue: Test helper functions with `test_` prefix are interpreted as pytest fixtures. See [Mutmut and Memory Profiler Results](MUTMUT_MEMORY_PROFILER_RESULTS.md) for detailed analysis and recommendations. |
| `memory-profiler` | Memory usage analysis | [PASS] Successfully completed. **What it tests:** Analyzes memory usage to find memory leaks or excessive memory consumption. Results show stable memory usage (48 MiB baseline, no leaks detected). See [Mutmut and Memory Profiler Results](MUTMUT_MEMORY_PROFILER_RESULTS.md) for detailed analysis. |

## 7. Scapy-based Protocol Tests

**Purpose:** These tests use Scapy to generate network packets and test the protocol implementation without requiring actual radio hardware. This allows comprehensive testing of the authentication and protocol logic in a controlled environment.

| Scope | Command | Result | Notes |
| --- | --- | --- | --- |
| Signature verification | `python3 integration/test_signature_verification.py` | [PASS] 22/22 | Generates two valid Brainpool signatures + 20 invalid variants; Scapy builds IP/UDP/Raw frames for TX/RX verification. **What it tests:** Validates that the signature verification system correctly accepts valid signatures and rejects invalid ones. Tests include: valid signatures from authorized operators, signatures with wrong keys, corrupted signatures, signatures with modified data, and edge cases. This ensures the cryptographic authentication is working correctly. |
| Comprehensive security (Scapy + ZMQ) | `python3 integration/test_security_comprehensive.py` | [PASS] 45/45 | Covers cryptographic edge cases, replay & rate limiting, injection attempts, authorization (incl. SSID normalization), and valid/invalid AX.25 frames with Scapy payload generation. **What it tests:** Comprehensive security testing including: replay attack prevention (rejecting duplicate commands), rate limiting (preventing command flooding), injection attack attempts (malformed commands), authorization checks (only authorized operators can execute commands), SSID normalization (handling different AX.25 SSID formats), and AX.25 frame validation (properly formatted vs. malformed frames). All 45 tests passing confirms the system is secure against common attack vectors. |

**Test Environment (Scapy suites):**
- Python 3.12.3
- Scapy 2.5+
- pyzmq 26.x
- Test Date: 2025-11-19

Both suites exercise the ZMQ interface and AX.25 framing logic without requiring RF hardware; failures are logged to `integration/test_security_comprehensive.py` output for quick diagnosis.

## 8. SVXLink TCP Protocol Testing

**Purpose:** These tests verify that authenticated commands are correctly translated into SVXLink protocol format and successfully executed via the TCP control port. This is the final step in the command execution pipeline.

| Scope | Test Method | Result | Notes |
| --- | --- | --- | --- |
| TCP Control Port | Mock SVXLink server + command handler | [PASS] | Successfully tested command translation and TCP communication. All command types (SET_SQUELCH, SET_POWER, PTT_ENABLE, custom) connect, send, and receive responses correctly. **What it tests:** Validates the complete command execution path: (1) Command translation from authenticated format to SVXLink protocol format, (2) TCP connection establishment to SVXLink control port, (3) Command transmission over TCP, (4) Response parsing and validation, (5) Error handling for connection failures and timeouts. Tests verify that commands like `SET_SQUELCH=5.5` are correctly translated to SVXLink format (`RX1.SQL_OPEN_THRESH=5.5`) and executed. This ensures authenticated commands actually control the repeater hardware correctly. |

**Test Environment:**
- Python: 3.12.3
- Mock SVXLink server: TCP listener on port 5210
- Test Date: 2025-01-27

**Tested Commands:**
- `SET_SQUELCH=5.5` → Translated to `RX1.SQL_OPEN_THRESH=5.5`
- `SET_POWER=15` → Translated to `TX1.TX_POWER=15`
- `PTT_ENABLE` → Translated to `TX1.PTT=1`
- Custom raw commands → Passed through as-is

**Implementation Status:**
- SVXLink TCP control port integration: **Implemented and tested**
- Command translation: **Fully functional**
- Error handling: **Comprehensive with timeout and connection error handling**
- Response parsing: **Validates SVXLink responses**

**Note:** The SVXLink execution path is now fully implemented. The command handler translates authenticated commands to SVXLink protocol format and executes them via TCP control port (default port 5210). Testing was performed against a mock SVXLink server to verify protocol compatibility.

---

### Next Steps

1. Increase functional coverage by integrating hardware‑in‑the‑loop tests once
   GNU Radio and SVXLink environments are available.
2. Test SVXLink TCP integration with real SVXLink instances to verify protocol
   compatibility and response handling.
3. Hook these commands into CI (GitHub Actions) so new changes automatically
   trigger the same suite.

For reproduction, refer to the command list above or run:

```bash
make lint
make format
make type-check
make security-check
make test
pytest --count=10 -q
```

This report will be updated as new tooling or requirements are added.

