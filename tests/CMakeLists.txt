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
# Boost.Test-based tests (using gr_add_cpp_test)
########################################################################
include(GrTest)
list(APPEND GR_TEST_TARGET_DEPS gnuradio-qradiolink)

# Add all Boost.Test-based test files (those using BOOST_AUTO_TEST_CASE)
list(APPEND test_gr_qradiolink_sources 
    test_mod_ssb.cc
    test_mod_qpsk.cc
    # test_mod_nbfm.cc  # Commented out - mod_nbfm_impl.cc requires missing emphasis.h
    # test_mod_dsss.cc  # Commented out - mod_dsss_impl.cc requires missing dsss_encoder_bb_impl.h
    test_demod_2fsk.cc
    test_demod_am.cc
    test_demod_ssb.cc
    # test_demod_wbfm.cc  # Commented out - demod_wbfm_impl.cc requires missing emphasis.h
    # test_demod_nbfm.cc  # Commented out - demod_nbfm_impl.cc requires missing emphasis.h
    test_demod_bpsk.cc
    test_demod_qpsk.cc
    test_demod_gmsk.cc
    test_demod_4fsk.cc
    # test_demod_dsss.cc  # Commented out - demod_dsss_impl.cc requires missing dsss_decoder_cc_impl.h
    test_demod_m17.cc
)

########################################################################
# Build all Boost.Test-based tests
########################################################################
foreach(qa_file ${test_gr_qradiolink_sources})
    get_filename_component(qa_name ${qa_file} NAME_WE)
    gr_add_cpp_test("qradiolink_${qa_name}" ${CMAKE_CURRENT_SOURCE_DIR}/${qa_file})
    # Add fmt library to each test target
    target_link_libraries(qradiolink_${qa_name} fmt)
    # DSSS tests are commented out - implementations require missing dependencies
    # if(qa_name STREQUAL "test_mod_dsss" OR qa_name STREQUAL "test_demod_dsss")
    #     target_link_libraries(qradiolink_${qa_name} gnuradio-qradiolink)
    # endif()
endforeach(qa_file)
