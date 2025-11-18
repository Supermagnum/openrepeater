"""Pytest configuration for AX.25 protocol tests."""

import sys
from pathlib import Path

# Ensure repository root is on sys.path for package imports
PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))
