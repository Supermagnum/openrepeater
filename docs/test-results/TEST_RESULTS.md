# Authenticated Control – Test & Analysis Report

_Last updated: 2025‑11‑17_

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

## 4. Additional Tooling

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

