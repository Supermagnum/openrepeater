# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Example flowgraphs
########################################################################

# Install example flowgraphs and Python scripts
install(FILES
    nbfm_example.grc
    m17_demod_example.grc
    freedv_example.py
    m17_mod_example.py
    DESTINATION ${GR_DOC_DIR}/examples/qradiolink
    COMPONENT examples
)

