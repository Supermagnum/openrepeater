# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install DSSS header files
########################################################################
install(
    FILES api.h
          dsss_encoder_bb.h
          dsss_decoder_cc.h
    DESTINATION ${GR_INCLUDE_DIR}/gnuradio/dsss)

