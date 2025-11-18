# Contributing to Authenticated Repeater Control

Thank you for your interest in contributing to the Authenticated Repeater Control project! This document provides guidelines and instructions for contributing.

## How to Contribute

There are many ways to contribute:

- **Code contributions**: Bug fixes, new features, improvements
- **Documentation**: Improving existing docs, adding examples, fixing typos
- **Testing**: Reporting bugs, testing new features, improving test coverage
- **Feedback**: Suggestions, feature requests, usability improvements

## Getting Started

### Development Setup

1. **Fork the repository** on GitHub

2. **Clone your fork**:
   ```bash
   git clone https://github.com/YOUR_USERNAME/authenticated-repeater-control.git
   cd authenticated-repeater-control
   ```

3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/Supermagnum/openrepeater.git
   ```

4. **Set up development environment**:
   ```bash
   # Install dependencies
   sudo apt-get install -y \
       gnuradio-dev \
       python3-dev \
       python3-pip \
       cmake \
       build-essential
   
   # Install Python development dependencies
   pip3 install -r requirements-dev.txt
   ```

### Development Workflow

1. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bug-fix
   ```

2. **Make your changes** following the coding standards below

3. **Test your changes**:
   ```bash
   # Run code quality checks
   ./integration/code_quality_report.sh
   
   # Run tests (if available)
   pytest
   ```

4. **Commit your changes**:
   ```bash
   git add .
   git commit -m "Description of your changes"
   ```

5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request** on GitHub

## Coding Standards

### Python Code

All Python code must follow these standards:

#### Code Quality Tools

All code must pass:

- **flake8** - PEP 8 style compliance
  ```bash
  flake8 --max-line-length=100 --extend-ignore=E203,W503 integration/
  ```

- **black** - Code formatting
  ```bash
  black --check integration/
  ```

- **isort** - Import sorting
  ```bash
  isort --check-only integration/
  ```

- **mypy** - Type checking (where applicable)
  ```bash
  mypy --ignore-missing-imports integration/
  ```

- **pylint** - Code analysis
  ```bash
  pylint integration/
  ```

- **bandit** - Security scanning
  ```bash
  bandit -r integration/ -ll
  ```

#### Style Guidelines

1. **Line Length**: Maximum 100 characters (enforced by flake8)
2. **Indentation**: 4 spaces (no tabs)
3. **Imports**: Sorted with isort, grouped: stdlib, third-party, local
4. **Type Hints**: Use type hints for function parameters and return values
5. **Docstrings**: All functions and classes must have docstrings
6. **Naming**: 
   - Functions and variables: `snake_case`
   - Classes: `PascalCase`
   - Constants: `UPPER_SNAKE_CASE`

#### Example

```python
#!/usr/bin/env python3
"""Module docstring explaining the purpose of this module."""

from typing import Dict, Optional

import yaml


class ExampleClass:
    """Class docstring explaining the purpose of this class."""

    def __init__(self, config_path: str) -> None:
        """Initialize the class.
        
        Args:
            config_path: Path to configuration file
        """
        self.config_path = config_path
        self.config: Optional[Dict] = None

    def load_config(self) -> Dict:
        """Load configuration from file.
        
        Returns:
            Dictionary containing configuration
            
        Raises:
            FileNotFoundError: If config file doesn't exist
            yaml.YAMLError: If config file is invalid
        """
        with open(self.config_path, "r") as f:
            self.config = yaml.safe_load(f)
        return self.config
```

### Shell Scripts

All shell scripts must:

1. **Start with shebang**: `#!/bin/bash`
2. **Use `set -e`**: Exit on error
3. **Pass shellcheck**: No warnings or errors
   ```bash
   shellcheck script.sh
   ```
4. **Include comments**: Explain complex logic
5. **Use quotes**: Always quote variables

#### Example

```bash
#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function example_function() {
    local var_name="$1"
    
    if [ -z "$var_name" ]; then
        echo "ERROR: Variable is required"
        return 1
    fi
    
    echo "Processing: $var_name"
}
```

### C++ Code (GNU Radio Modules)

C++ code in GNU Radio OOT modules must:

1. **Follow GNU Radio coding standards**
2. **Use CMake** for build system
3. **Include Doxygen comments** for public APIs
4. **Match existing code style** in the module

## Testing Requirements

### Before Submitting

1. **Run all code quality checks**:
   ```bash
   ./integration/code_quality_report.sh
   ```

2. **Test your changes**:
   - Test on actual hardware when possible
   - Test with different configurations
   - Test error conditions

3. **Verify documentation**:
   - Update relevant documentation
   - Check for broken links
   - Verify code examples work

### Test Coverage

- Aim for high test coverage for new code
- Include unit tests for new functions
- Include integration tests for new features
- Test error handling and edge cases

## Documentation Standards

### Code Documentation

- **All functions** must have docstrings
- **All classes** must have docstrings
- **Complex logic** must have inline comments
- **Public APIs** must be documented

### User Documentation

- **New features** require documentation updates
- **Configuration changes** must be documented
- **Breaking changes** must be clearly documented
- **Examples** should be included where helpful

### Documentation Format

- Use Markdown for all documentation
- Follow existing documentation style
- Include code examples where applicable
- Keep documentation up to date

## Commit Messages

### Format

Use clear, descriptive commit messages:

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain what and why, not how.

- Bullet points for multiple changes
- Reference issues: Fixes #123
```

### Examples

**Good:**
```
Fix PTT timing issue in flowgraph

The PTT control block was not properly handling hang time,
causing audio to be cut off at the end of transmissions.

- Added proper hang time delay
- Fixed GPIO cleanup on exit
- Updated documentation

Fixes #45
```

**Bad:**
```
fix bug
```

## Pull Request Process

### Before Submitting

1. **Update your branch** with latest changes:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Ensure all checks pass**:
   - Code quality tools
   - Tests
   - Documentation

3. **Write clear PR description**:
   - What changes were made
   - Why the changes were needed
   - How to test the changes
   - Screenshots (if applicable)

### PR Requirements

- **One logical change per PR**: Keep PRs focused
- **All checks must pass**: CI/CD must be green
- **Code review required**: At least one approval
- **Documentation updated**: If needed for the change

### Review Process

1. **Automated checks** run on PR creation
2. **Maintainers review** the code
3. **Feedback is provided** if changes needed
4. **PR is merged** after approval

## Code of Conduct

### Our Standards

- **Be respectful**: Treat everyone with respect
- **Be inclusive**: Welcome contributors of all backgrounds
- **Be constructive**: Provide helpful feedback
- **Be patient**: Understand that everyone has different skill levels

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or inflammatory comments
- Personal attacks
- Publishing private information

## Development Guidelines

### Adding New Features

1. **Discuss first**: Open an issue to discuss the feature
2. **Get approval**: Wait for maintainer approval before coding
3. **Follow standards**: Use existing code patterns
4. **Add tests**: Include tests for new features
5. **Update docs**: Document new features

### Fixing Bugs

1. **Reproduce the bug**: Create a test case that demonstrates it
2. **Fix the bug**: Make minimal changes to fix the issue
3. **Add regression test**: Prevent the bug from returning
4. **Update docs**: If documentation was incorrect

### Refactoring

1. **Keep functionality**: Don't change behavior
2. **Improve code**: Make code cleaner, faster, or more maintainable
3. **Update tests**: Ensure tests still pass
4. **Document changes**: Explain why refactoring was needed

## Project Structure

Understanding the project structure helps with contributions:

```
authenticated-repeater-control/
├── integration/          # Main integration code
│   ├── authenticated_command_handler.py
│   ├── install_authenticated_modules.sh
│   └── uninstall_authenticated_modules.sh
├── modules/              # GNU Radio OOT modules
│   ├── gr-linux-crypto/
│   ├── gr-packet-protocols/
│   └── gr-qradiolink/
├── openrepeater/         # OpenRepeater integration
│   └── scripts/
├── docs/                 # Documentation
│   ├── additional/       # User guides
│   ├── guides/          # Technical guides
│   └── test-results/    # Test reports
└── flowgraphs/          # GNU Radio flowgraphs
```

## Getting Help

### Questions?

- **Open an issue**: For questions or discussions
- **Check documentation**: Many questions are answered in the docs
- **Search issues**: Your question may have been asked before

### Reporting Bugs

When reporting bugs, include:

1. **Description**: Clear description of the bug
2. **Steps to reproduce**: How to trigger the bug
3. **Expected behavior**: What should happen
4. **Actual behavior**: What actually happens
5. **Environment**: OS, Python version, GNU Radio version
6. **Logs**: Relevant error messages or logs

### Feature Requests

When requesting features, include:

1. **Use case**: Why is this feature needed?
2. **Proposed solution**: How should it work?
3. **Alternatives**: Other solutions considered
4. **Impact**: Who benefits from this feature?

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (GPL-3.0).

## Recognition

Contributors will be:

- Listed in CONTRIBUTORS.md (if file exists)
- Credited in release notes for significant contributions
- Acknowledged in commit messages when appropriate

## Thank You!

Thank you for taking the time to contribute to this project. Your contributions help make amateur radio repeater control safer and more accessible for everyone!

