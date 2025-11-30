# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

"""
Blocks and utilities for QRadioLink modulation and demodulation schemes.
"""

# The presence of this file turns this directory into a Python package

import os
import sys

# Import GNU Radio first to ensure hier_block2 and other base types are registered
# This is required for pybind11 to recognize the base types when loading our bindings
import gnuradio  # noqa: F401

# Check if the bindings module is already loaded to avoid re-registration conflicts
# This is critical: pybind11 types can only be registered once per Python process
_bindings_module_key = "gnuradio.qradiolink.qradiolink_python"
_bindings_loaded = _bindings_module_key in sys.modules

# Also check if the parent module is already loaded
_parent_module_key = "gnuradio.qradiolink"
_parent_loaded = _parent_module_key in sys.modules

if _bindings_loaded:
    # Module already loaded - import symbols from existing module
    _existing_bindings = sys.modules[_bindings_module_key]
    # Copy public symbols to current namespace
    for name in dir(_existing_bindings):
        if not name.startswith("_"):
            globals()[name] = getattr(_existing_bindings, name)
elif _parent_loaded:
    # Parent module loaded but bindings not - this should not happen normally
    # Try to get bindings from parent module
    try:
        _parent_module = sys.modules[_parent_module_key]
        if hasattr(_parent_module, "qradiolink_python"):
            # Copy symbols from parent's bindings
            _existing_bindings = _parent_module.qradiolink_python
            for name in dir(_existing_bindings):
                if not name.startswith("_"):
                    globals()[name] = getattr(_existing_bindings, name)
            _bindings_loaded = True
        else:
            # Try normal import
            from .qradiolink_python import *  # noqa: F403  # noqa: F403, F405

            _bindings_loaded = True
    except (ImportError, RuntimeError, AttributeError):
        _bindings_loaded = False
else:
    # Module not loaded - try to load it
    try:
        from .qradiolink_python import *  # noqa: F403

        _bindings_loaded = True
    except (ImportError, RuntimeError) as e:
        # Handle pybind11 registration conflicts
        error_str = str(e)
        if "already registered" in error_str or "generic_type" in error_str:
            # Registration conflict - types are already registered
            # This can happen if the module was loaded in a previous import attempt
            # Check if the module is accessible despite the error
            if _bindings_module_key in sys.modules:
                _existing_bindings = sys.modules[_bindings_module_key]
                for name in dir(_existing_bindings):
                    if not name.startswith("_"):
                        globals()[name] = getattr(_existing_bindings, name)
                _bindings_loaded = True
            else:
                # Try fallback path
                dirname, filename = os.path.split(os.path.abspath(__file__))
                bindings_path = os.path.join(dirname, "bindings")
                if bindings_path not in __path__:  # noqa: F405
                    __path__.append(bindings_path)  # noqa: F405
                try:
                    from .qradiolink_python import *  # noqa: F403  # noqa: F403, F405

                    _bindings_loaded = True
                except (ImportError, RuntimeError):
                    # If that also fails, we can't load the module
                    # Don't raise - let the caller handle it gracefully
                    _bindings_loaded = False
        else:
            # Other ImportError - try fallback path
            dirname, filename = os.path.split(os.path.abspath(__file__))
            bindings_path = os.path.join(dirname, "bindings")
            if bindings_path not in __path__:  # noqa: F405
                __path__.append(bindings_path)  # noqa: F405
            try:
                from .qradiolink_python import *  # noqa: F403  # noqa: F403, F405

                _bindings_loaded = True
            except (ImportError, RuntimeError):
                # If both fail, re-raise the original error
                raise

