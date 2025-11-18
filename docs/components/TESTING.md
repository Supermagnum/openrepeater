# Testing Guide

This document describes how to run tests for the SVXLink control interface.

## Test Suite Structure

```
tests/
├── __init__.py
├── conftest.py                    # Shared fixtures
├── test_command_parser.py         # Command parser tests
├── test_config_manager.py         # Configuration manager tests
├── test_control_service.py       # Control service tests
└── test_hardware_interface.py    # Hardware interface tests
```

## Running Tests

### Basic Test Execution

```bash
# Run all tests
pytest

# Run with verbose output
pytest -v

# Run specific test file
pytest tests/test_command_parser.py

# Run specific test
pytest tests/test_command_parser.py::TestCommandParser::test_parse_set_squelch_valid
```

### Coverage Reports

```bash
# Generate coverage report
pytest --cov=. --cov-report=html

# View HTML report
open htmlcov/index.html

# Terminal coverage report
pytest --cov=. --cov-report=term-missing
```

### Test Categories

```bash
# Run only unit tests
pytest -m unit

# Run only integration tests
pytest -m integration

# Run only fast tests
pytest -m "not slow"
```

## Test Requirements

Install test dependencies:

```bash
pip install -r requirements-dev.txt
```

## Code Quality Checks

### Static Analysis

```bash
# Flake8
flake8 --statistics --count .

# MyPy type checking
mypy --strict .

# Bandit security scan
bandit -r . -ll
```

### Code Formatting

```bash
# Check formatting
black --check --diff .

# Format code
black .

# Check import sorting
isort --check-only --diff .

# Sort imports
isort .
```

## Continuous Integration

Tests run automatically on:
- Push to main/develop branches
- Pull requests
- Multiple Python versions (3.8, 3.9, 3.10, 3.11)

See `.github/workflows/ci.yml` for CI configuration.

## Writing Tests

### Test Structure

```python
import pytest
from module import Class

class TestClass:
    """Test suite for Class."""

    def setup_method(self):
        """Set up test fixtures."""
        self.instance = Class()

    def test_feature(self):
        """Test specific feature."""
        result = self.instance.method()
        assert result == expected
```

### Using Fixtures

```python
def test_with_fixture(config_manager):
    """Test using fixture from conftest.py."""
    assert config_manager is not None
```

### Mocking

```python
from unittest.mock import patch, MagicMock

def test_with_mock():
    """Test with mocked dependency."""
    with patch('module.external_function') as mock_func:
        mock_func.return_value = "mocked"
        result = function_under_test()
        assert result == "mocked"
```

## Property-Based Testing

Use Hypothesis for property-based testing:

```python
from hypothesis import given, strategies as st

@given(st.integers(min_value=-130, max_value=-50))
def test_squelch_range(level):
    """Test squelch accepts valid range."""
    success, parsed, error = parser.parse(f"SET_SQUELCH {level}")
    assert success is True
```

## Performance Testing

Measure performance:

```python
import time

def test_performance():
    """Test command parsing performance."""
    start = time.time()
    for _ in range(1000):
        parser.parse("SET_SQUELCH -120")
    elapsed = time.time() - start
    assert elapsed < 0.01  # Must complete in under 10ms
```

## Test Coverage Goals

- Target: 90%+ code coverage
- Critical paths: 100% coverage
- All public APIs: 100% coverage

## Troubleshooting

### Tests Failing

1. Check Python version compatibility
2. Verify all dependencies installed
3. Check test data files exist
4. Review error messages carefully

### Coverage Issues

1. Identify uncovered lines
2. Add tests for missing coverage
3. Exclude intentionally untested code
4. Document exclusions

## Best Practices

1. **Isolation**: Each test should be independent
2. **Clarity**: Test names should describe what they test
3. **Speed**: Tests should run quickly
4. **Determinism**: Tests should produce consistent results
5. **Coverage**: Test both success and failure paths

