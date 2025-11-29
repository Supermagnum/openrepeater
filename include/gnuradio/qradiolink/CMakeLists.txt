# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Install header files
########################################################################
install(
    FILES api.h
          mod_2fsk.h
          mod_4fsk.h
          mod_am.h
          mod_gmsk.h
          mod_bpsk.h
          mod_ssb.h
          mod_qpsk.h
          mod_nbfm.h
          mod_dsss.h
          mod_freedv.h
          mod_m17.h
          mod_dmr.h
          mod_nxdn.h
          mod_dpmr.h
          mod_mmdvm.h
          mod_mmdvm_multi2.h
          demod_2fsk.h
          demod_am.h
          demod_ssb.h
          demod_wbfm.h
          demod_nbfm.h
          demod_bpsk.h
          demod_qpsk.h
          demod_gmsk.h
          demod_4fsk.h
          demod_dsss.h
          demod_m17.h
          demod_freedv.h
          demod_dmr.h
          demod_nxdn.h
          demod_dpmr.h
          demod_mmdvm_multi.h
          demod_mmdvm_multi2.h
          gr_4fsk_discriminator.h
          zero_idle_bursts.h
          mmdvm_source.h
          mmdvm_sink.h
          rssi_tag_block.h
          m17_deframer.h
          clipper_cc.h
          stretcher_cc.h
    DESTINATION ${GR_INCLUDE_DIR}/gnuradio/qradiolink)

# Note: Additional header files will be added here as blocks are migrated

