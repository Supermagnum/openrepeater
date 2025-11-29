# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

include(GrPybind)

########################################################################
# Python bindings
########################################################################

list(
    APPEND
    qradiolink_python_files
    mod_2fsk_python.cc
    mod_4fsk_python.cc
    mod_am_python.cc
    mod_gmsk_python.cc
    mod_bpsk_python.cc
    mod_ssb_python.cc
    mod_qpsk_python.cc
    mod_nbfm_python.cc
    mod_dsss_python.cc
    mod_m17_python.cc
    mod_dmr_python.cc
    mod_freedv_python.cc
    mod_mmdvm_python.cc
    demod_2fsk_python.cc
    demod_am_python.cc
    demod_ssb_python.cc
    demod_wbfm_python.cc
    demod_nbfm_python.cc
    demod_bpsk_python.cc
    demod_qpsk_python.cc
    demod_gmsk_python.cc
    demod_4fsk_python.cc
    demod_dsss_python.cc
    demod_m17_python.cc
    demod_dmr_python.cc
    demod_freedv_python.cc
    rssi_tag_block_python.cc
    m17_deframer_python.cc
    python_bindings.cc)

pybind11_add_module(qradiolink_python ${qradiolink_python_files})

target_include_directories(
    qradiolink_python
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
            ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_link_libraries(
    qradiolink_python
    PRIVATE pybind11::pybind11
            Python::Module
            Python::NumPy
            -Wl,--whole-archive
            gnuradio-qradiolink
            -Wl,--no-whole-archive
            ${GR_RUNTIME_LIBRARIES}
            ${GR_BLOCKS_LIBRARIES}
            ${GR_FILTER_LIBRARIES}
            ${GR_DIGITAL_LIBRARIES}
            ${GR_ANALOG_LIBRARIES}
            ${GR_FEC_LIBRARIES}
            ${GR_VOCODER_LIBRARIES})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(qradiolink_python PRIVATE
        -Wno-unused-variable
        -Wno-error=deprecated-declarations
    )
endif()

install(
    TARGETS qradiolink_python
    DESTINATION ${GR_PYTHON_DIR}/gnuradio/qradiolink
    COMPONENT pythonapi)

