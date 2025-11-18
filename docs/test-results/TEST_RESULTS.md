# Authenticated Control – Test & Analysis Report

_Last updated: 2025‑11‑18_

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

| Tool / Command | Result | Notes |
| --- | --- | --- |
| `flake8` | [PASS] Pass | Zero lint errors across `svxlink_control` and `ax25_protocol`. |
| `black --check .` | [PASS] Pass | All files formatted; enforced via `make format`. |
| `isort --check-only .` | [PASS] Pass | Import ordering matches Black profile. |
| `pylint --disable=C0114,C0115,C0116 *.py` | [WARNING] Score 8.81/10 | Remaining warnings are expected for logging style, TODO placeholders, and broad exception handling in `command_handler.py`. |

## 2. Static Analysis & Security

| Tool / Command | Result | Notes |
| --- | --- | --- |
| `bandit -r . -ll` | [PASS] Pass | No security issues identified (0 findings). |
| `vulture . --min-confidence 80` | [PASS] Pass | No dead-code candidates reported after cleanup. |
| `radon cc . -a -nb` | [PASS] Avg complexity **B (7.67)** | A few functions (parser, control service) are intentionally complex but documented. |
| `radon mi . -nb` | [PASS] All files rated **A** | Maintainability index remains excellent. |
| `interrogate -v --fail-under 80 .` | [PASS] 91.7 % docstring coverage | No modules below the 80 % threshold. |

## 3. Testing & Coverage

| Scope | Command | Result | Coverage |
| --- | --- | --- | --- |
| `svxlink_control` unit tests | `pytest -v --tb=short` | [PASS] Pass | ~71 % (threshold set to 70 % via `.coveragerc`). |
| `ax25_protocol` unit tests | `pytest -v --tb=short` | [PASS] Pass | ~64 % (threshold set to 60 %). |
| Full suite (root) | `make test` | [PASS] Pass | Combined coverage ≈70 %. |
| Stress repeat | `pytest --count=10 -q` | [PASS] 550 tests | Confirms deterministic behaviour with `pytest-repeat`. |

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

| Module | Test Command | Result | Notes |
| --- | --- | --- | --- |
| `gr-linux-crypto` | `python3 -c "from gr_linux_crypto.crypto_helpers import CryptoHelpers"` | [PASS] | Module imports successfully. Requires `modules/gr-linux-crypto` in PYTHONPATH. Symlink `gr_linux_crypto -> python` created to enable package structure. |
| `gr-packet-protocols` | `python3 -c "from gnuradio.packet_protocols import ax25_decoder, ax25_encoder"` | [PASS] | Module imports successfully. Installed as part of GNU Radio distribution. |
| `authenticated_command_handler` | `python3 authenticated_command_handler.py` | [PASS] | Command handler starts successfully with placeholder config. Correctly reports missing authorized keys (expected behavior). |

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

| Component | Status | Notes |
| --- | --- | --- |
| Function file integration | [PASS] | `functions_authenticated_control.sh` can be sourced and functions are callable |
| Install script integration | [PASS] | `install_orp.sh` sources the function file at line 193 |
| Service file path resolution | [FIXED] | Updated to handle multiple path scenarios |
| Verification function | [PASS] | Correctly identifies installed/missing components |

**Detailed Report:** See [OpenRepeater Integration Test Report](OPENREPEATER_INTEGRATION_TEST.md)

**Note:** The `install_authenticated_control()` function is available but not automatically called during OpenRepeater installation. It must be called manually or added to the installation sequence.

## 6. Additional Tooling

| Tool | Purpose | Status |
| --- | --- | --- |
| `pytest-randomly`, `pytest-repeat` | Exercise tests with varied ordering and repetition | Installed & used (`pytest --count=10`). |
| `mutmut`, `memory-profiler` | Mutation testing & profiling (not yet run) | Installed for future workflows. |

---

### Next Steps

1. Increase functional coverage by integrating hardware‑in‑the‑loop tests once
   GNU Radio and SVXLink environments are available.
2. Resolve the remaining pylint TODO warnings when the real SVXLink execution code
   is implemented.
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

