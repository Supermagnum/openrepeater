# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Setup testing
########################################################################
include(CTest)

########################################################################
# Test mod_2fsk
########################################################################
add_executable(test_mod_2fsk test_mod_2fsk.cc)
target_link_libraries(test_mod_2fsk gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-digital
                                 gnuradio-filter
                                 gnuradio-analog
                                 gnuradio-fec
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_DIGITAL_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 ${GR_FEC_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_2fsk COMMAND test_mod_2fsk)

########################################################################
# Test mod_4fsk
########################################################################
add_executable(test_mod_4fsk test_mod_4fsk.cc)
target_link_libraries(test_mod_4fsk gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-digital
                                 gnuradio-filter
                                 gnuradio-analog
                                 gnuradio-fec
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_DIGITAL_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 ${GR_FEC_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_4fsk COMMAND test_mod_4fsk)

########################################################################
# Test mod_am
########################################################################
add_executable(test_mod_am test_mod_am.cc)
target_link_libraries(test_mod_am gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-filter
                                 gnuradio-analog
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_am COMMAND test_mod_am)

########################################################################
# Test mod_gmsk
########################################################################
add_executable(test_mod_gmsk test_mod_gmsk.cc)
target_link_libraries(test_mod_gmsk gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-digital
                                 gnuradio-filter
                                 gnuradio-analog
                                 gnuradio-fec
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_DIGITAL_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 ${GR_FEC_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_gmsk COMMAND test_mod_gmsk)

########################################################################
# Test mod_bpsk
########################################################################
add_executable(test_mod_bpsk test_mod_bpsk.cc)
target_link_libraries(test_mod_bpsk gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-digital
                                 gnuradio-filter
                                 gnuradio-fec
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_DIGITAL_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_FEC_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_bpsk COMMAND test_mod_bpsk)

########################################################################
# Test mod_mmdvm
########################################################################
add_executable(test_mod_mmdvm test_mod_mmdvm.cc)
target_link_libraries(test_mod_mmdvm gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-digital
                                 gnuradio-filter
                                 gnuradio-analog
                                 gnuradio-fec
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_DIGITAL_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 ${GR_FEC_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_mmdvm COMMAND test_mod_mmdvm)

########################################################################
# Test mod_freedv
########################################################################
add_executable(test_mod_freedv test_mod_freedv.cc)
target_link_libraries(test_mod_freedv gnuradio-qradiolink
                                 gnuradio-runtime
                                 gnuradio-blocks
                                 gnuradio-filter
                                 gnuradio-analog
                                 gnuradio-vocoder
                                 ${GR_RUNTIME_LIBRARIES}
                                 ${GR_BLOCKS_LIBRARIES}
                                 ${GR_FILTER_LIBRARIES}
                                 ${GR_ANALOG_LIBRARIES}
                                 ${GR_VOCODER_LIBRARIES}
                                 fmt)

add_test(NAME test_mod_freedv COMMAND test_mod_freedv)

########################################################################
# Boost.Test-based tests (using gr_add_cpp_test)
########################################################################
include(GrTest)
list(APPEND GR_TEST_TARGET_DEPS gnuradio-qradiolink)

# Add all Boost.Test-based test files (those using BOOST_AUTO_TEST_CASE)
list(APPEND test_gr_qradiolink_sources 
    test_mod_ssb.cc
    test_mod_qpsk.cc
    test_mod_nbfm.cc
    test_mod_dsss.cc
    test_demod_2fsk.cc
    test_demod_am.cc
    test_demod_ssb.cc
    test_demod_wbfm.cc
    test_demod_nbfm.cc
    test_demod_bpsk.cc
    test_demod_qpsk.cc
    test_demod_gmsk.cc
    test_demod_4fsk.cc
    test_demod_dsss.cc
    test_demod_m17.cc
    test_demod_dmr.cc
    test_demod_freedv.cc
    test_demod_mmdvm_multi.cc
    test_demod_mmdvm_multi2.cc
    test_rssi_tag_block.cc
)

########################################################################
# Build all Boost.Test-based tests
########################################################################
foreach(qa_file ${test_gr_qradiolink_sources})
    get_filename_component(qa_name ${qa_file} NAME_WE)
    gr_add_cpp_test("qradiolink_${qa_name}" ${CMAKE_CURRENT_SOURCE_DIR}/${qa_file})
    # Add fmt library to each test target
    target_link_libraries(qradiolink_${qa_name} fmt)
    # Add vocoder library for FreeDV tests
    if(qa_file MATCHES "freedv")
        target_link_libraries(qradiolink_${qa_name} gnuradio-vocoder ${GR_VOCODER_LIBRARIES})
    endif()
    # Add test_bursttimer.h include path for MMDVM multi tests
    if(qa_file MATCHES "mmdvm_multi")
        target_include_directories(qradiolink_${qa_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endforeach(qa_file)
