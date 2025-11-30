# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Setup library
########################################################################

# List of source files - will be populated as blocks are migrated
set(gr_qradiolink_sources
    # Modulators
    mod_2fsk_impl.cc
    mod_4fsk_impl.cc
    mod_am_impl.cc
    mod_gmsk_impl.cc
    mod_bpsk_impl.cc
    mod_ssb_impl.cc
    mod_qpsk_impl.cc
    mod_nbfm_impl.cc
    mod_dsss_impl.cc
    dsss_encoder_bb_impl.cc
    mod_freedv_impl.cc
    mod_m17_impl.cc
    mod_dmr_impl.cc
    mod_nxdn_impl.cc
    mod_dpmr_impl.cc
    mod_mmdvm_impl.cc
    mod_mmdvm_multi2_impl.cc
    # demodulators
    demod_2fsk_impl.cc
    demod_am_impl.cc
    demod_ssb_impl.cc
    demod_wbfm_impl.cc
    demod_nbfm_impl.cc
    demod_bpsk_impl.cc
    demod_qpsk_impl.cc
    demod_gmsk_impl.cc
    demod_4fsk_impl.cc
    demod_dsss_impl.cc
    dsss_decoder_cc_impl.cc
    ../../src/gr/emphasis.cpp
    demod_m17_impl.cc
    demod_freedv_impl.cc
    demod_dmr_impl.cc
    demod_nxdn_impl.cc
    demod_dpmr_impl.cc
    demod_mmdvm_multi_impl.cc
    demod_mmdvm_multi2_impl.cc
    # Supporting blocks
    gr_4fsk_discriminator_impl.cc
    zero_idle_bursts_impl.cc
    mmdvm_source_impl.cc
    mmdvm_sink_impl.cc
    rssi_tag_block_impl.cc
    m17_deframer_impl.cc
    # Supporting blocks (CESSB)
    clipper_cc_impl.cc
    stretcher_cc_impl.cc
)

add_library(
    gnuradio-qradiolink
    ${gr_qradiolink_sources}
)

# Ensure library is compiled with -fPIC for linking into shared modules
set_target_properties(
    gnuradio-qradiolink
    PROPERTIES POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(
    gnuradio-qradiolink PUBLIC
    ${GR_RUNTIME_LIBRARIES}
    ${GR_BLOCKS_LIBRARIES}
    ${GR_FILTER_LIBRARIES}
    ${GR_DIGITAL_LIBRARIES}
    ${GR_ANALOG_LIBRARIES}
    ${GR_FEC_LIBRARIES}
    ${GR_VOCODER_LIBRARIES}
    Volk::volk
    Boost::boost
)

# Add ZMQ if available (for mmdvm_source)
if(ZMQ_FOUND)
    target_link_libraries(
        gnuradio-qradiolink PUBLIC
        ${ZMQ_LIBRARIES}
    )
    target_include_directories(
        gnuradio-qradiolink PUBLIC
        ${ZMQ_INCLUDE_DIRS}
    )
endif()

target_include_directories(
    gnuradio-qradiolink PUBLIC
    ${GR_RUNTIME_INCLUDE_DIRS}
    ${GR_BLOCKS_INCLUDE_DIRS}
    ${GR_FILTER_INCLUDE_DIRS}
    ${GR_DIGITAL_INCLUDE_DIRS}
    ${GR_ANALOG_INCLUDE_DIRS}
    ${GR_FEC_INCLUDE_DIRS}
    ${GR_VOCODER_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src  # For src/gr/emphasis.h
)

target_include_directories(
    gnuradio-qradiolink
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/gnuradio/dsss> # For DSSS blocks
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../qradiolink/src/gr/cessb> # For CESSB blocks
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../qradiolink/src/gr> # For zero_idle_bursts
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../qradiolink> # For src/bursttimer.h (project root)
        PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src> # For src/gr/emphasis.h
)

# Note: gr_library_foo requires targetConfig.cmake.in template which is not present
# For shared library builds, the library will still work without it
# if(BUILD_SHARED_LIBS)
#     gr_library_foo(gnuradio-qradiolink)
# endif()

########################################################################
# QA C++ Code (if needed)
########################################################################
if(ENABLE_TESTING)
    include(GrTest)
    # Add test sources here when ready
    # list(APPEND test_gr_qradiolink_sources qa_block_name.cc)
    # list(APPEND GR_TEST_TARGET_DEPS gnuradio-qradiolink)
    # foreach(qa_file ${test_gr_qradiolink_sources})
    #     gr_add_cpp_test("qradiolink_${qa_file}" ${CMAKE_CURRENT_SOURCE_DIR}/${qa_file})
    # endforeach(qa_file)
endif(ENABLE_TESTING)

