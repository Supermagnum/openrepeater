# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Fuzzing harnesses
########################################################################

# Fuzzing harnesses test the actual implementation code
# Each harness creates a minimal flowgraph to exercise block processing

# Fuzzer for mod_2fsk (byte input -> complex output)
add_executable(fuzz_mod_2fsk fuzz_mod_2fsk.cc)
target_link_libraries(fuzz_mod_2fsk gnuradio-qradiolink
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
                                     ${GR_FEC_LIBRARIES})

# Fuzzer for mod_4fsk
add_executable(fuzz_mod_4fsk fuzz_mod_4fsk.cc)
target_link_libraries(fuzz_mod_4fsk gnuradio-qradiolink
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
                                     ${GR_FEC_LIBRARIES})

# Fuzzer for mod_bpsk
add_executable(fuzz_mod_bpsk fuzz_mod_bpsk.cc)
target_link_libraries(fuzz_mod_bpsk gnuradio-qradiolink
                                      gnuradio-runtime
                                      gnuradio-blocks
                                      gnuradio-digital
                                      gnuradio-filter
                                      gnuradio-fec
                                      ${GR_RUNTIME_LIBRARIES}
                                      ${GR_BLOCKS_LIBRARIES}
                                      ${GR_DIGITAL_LIBRARIES}
                                      ${GR_FILTER_LIBRARIES}
                                      ${GR_FEC_LIBRARIES})

# Fuzzer for mod_qpsk
add_executable(fuzz_mod_qpsk fuzz_mod_qpsk.cc)
target_link_libraries(fuzz_mod_qpsk gnuradio-qradiolink
                                      gnuradio-runtime
                                      gnuradio-blocks
                                      gnuradio-digital
                                      gnuradio-filter
                                      gnuradio-fec
                                      ${GR_RUNTIME_LIBRARIES}
                                      ${GR_BLOCKS_LIBRARIES}
                                      ${GR_DIGITAL_LIBRARIES}
                                      ${GR_FILTER_LIBRARIES}
                                      ${GR_FEC_LIBRARIES})

# Fuzzer for demod_2fsk (complex input -> byte output)
add_executable(fuzz_demod_2fsk fuzz_demod_2fsk.cc)
target_link_libraries(fuzz_demod_2fsk gnuradio-qradiolink
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
                                       ${GR_FEC_LIBRARIES})

# Fuzzer for demod_4fsk
add_executable(fuzz_demod_4fsk fuzz_demod_4fsk.cc)
target_link_libraries(fuzz_demod_4fsk gnuradio-qradiolink
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
                                       ${GR_FEC_LIBRARIES})

# Fuzzer for demod_bpsk
add_executable(fuzz_demod_bpsk fuzz_demod_bpsk.cc)
target_link_libraries(fuzz_demod_bpsk gnuradio-qradiolink
                                        gnuradio-runtime
                                        gnuradio-blocks
                                        gnuradio-digital
                                        gnuradio-filter
                                        gnuradio-fec
                                        ${GR_RUNTIME_LIBRARIES}
                                        ${GR_BLOCKS_LIBRARIES}
                                        ${GR_DIGITAL_LIBRARIES}
                                        ${GR_FILTER_LIBRARIES}
                                        ${GR_FEC_LIBRARIES})

# Fuzzer for demod_qpsk
add_executable(fuzz_demod_qpsk fuzz_demod_qpsk.cc)
target_link_libraries(fuzz_demod_qpsk gnuradio-qradiolink
                                       gnuradio-runtime
                                       gnuradio-blocks
                                       gnuradio-digital
                                       gnuradio-filter
                                       gnuradio-fec
                                       ${GR_RUNTIME_LIBRARIES}
                                       ${GR_BLOCKS_LIBRARIES}
                                       ${GR_DIGITAL_LIBRARIES}
                                       ${GR_FILTER_LIBRARIES}
                                       ${GR_FEC_LIBRARIES})

# Fuzzer for CESSB clipper_cc
add_executable(fuzz_clipper_cc fuzz_clipper_cc.cc)
target_link_libraries(fuzz_clipper_cc gnuradio-qradiolink
                                     gnuradio-runtime
                                     gnuradio-blocks
                                     ${GR_RUNTIME_LIBRARIES}
                                     ${GR_BLOCKS_LIBRARIES})

# Fuzzer for DSSS encoder
add_executable(fuzz_dsss_encoder fuzz_dsss_encoder.cc)
target_link_libraries(fuzz_dsss_encoder gnuradio-qradiolink
                                         gnuradio-runtime
                                         gnuradio-blocks
                                         ${GR_RUNTIME_LIBRARIES}
                                         ${GR_BLOCKS_LIBRARIES})

# Fuzzer for m17_deframer (byte input -> byte output)
add_executable(fuzz_m17_deframer fuzz_m17_deframer.cc)
target_link_libraries(fuzz_m17_deframer gnuradio-qradiolink
                                         gnuradio-runtime
                                         gnuradio-blocks
                                         ${GR_RUNTIME_LIBRARIES}
                                         ${GR_BLOCKS_LIBRARIES})

# Fuzzer for demod_gmsk (complex input -> byte output)
add_executable(fuzz_demod_gmsk fuzz_demod_gmsk.cc)
target_link_libraries(fuzz_demod_gmsk gnuradio-qradiolink
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
                                      ${GR_FEC_LIBRARIES})

# Fuzzer for demod_dsss (complex input -> byte output)
add_executable(fuzz_demod_dsss fuzz_demod_dsss.cc)
target_link_libraries(fuzz_demod_dsss gnuradio-qradiolink
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
                                      ${GR_FEC_LIBRARIES})

