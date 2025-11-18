# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

'''
Blocks and utilities for QRadioLink modulation and demodulation schemes.
'''

# The presence of this file turns this directory into a Python package

import os

# Import GNU Radio first to ensure hier_block2 and other base types are registered
# This is required for pybind11 to recognize the base types when loading our bindings
import gnuradio

try:
    from .qradiolink_python import *
except ImportError:
    dirname, filename = os.path.split(os.path.abspath(__file__))
    __path__.append(os.path.join(dirname, "bindings"))
    from .qradiolink_python import *

