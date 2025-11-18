# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Fuzzing configuration
########################################################################

# Check if libFuzzer is available (requires Clang with -fsanitize=fuzzer)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(ENABLE_FUZZING "Enable libFuzzer-based fuzzing targets" OFF)
    
    if(ENABLE_FUZZING)
        # Set compiler flags for libFuzzer
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer,address -g -O1")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=fuzzer,address -g -O1")
        
        # Add fuzzing harnesses subdirectory
        add_subdirectory(harnesses)
    endif()
else()
    message(STATUS "libFuzzer requires Clang compiler. Fuzzing disabled.")
endif()

