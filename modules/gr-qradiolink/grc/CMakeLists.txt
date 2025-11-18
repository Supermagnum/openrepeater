# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install GRC block definitions
########################################################################
install(FILES
    qradiolink_mod_2fsk.block.yml
    qradiolink_mod_4fsk.block.yml
    qradiolink_mod_am.block.yml
    qradiolink_mod_gmsk.block.yml
    qradiolink_mod_bpsk.block.yml
    qradiolink_mod_ssb.block.yml
    qradiolink_mod_qpsk.block.yml
    qradiolink_mod_nbfm.block.yml
    qradiolink_mod_dsss.block.yml
    qradiolink_demod_2fsk.block.yml
    qradiolink_demod_am.block.yml
    qradiolink_demod_ssb.block.yml
    qradiolink_demod_wbfm.block.yml
    qradiolink_demod_nbfm.block.yml
    qradiolink_demod_bpsk.block.yml
    qradiolink_demod_qpsk.block.yml
    qradiolink_demod_gmsk.block.yml
    qradiolink_demod_4fsk.block.yml
    qradiolink_demod_dsss.block.yml
    qradiolink_demod_m17.block.yml
    qradiolink.tree.yml
    DESTINATION ${GRC_BLOCKS_DIR})

