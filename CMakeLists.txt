# Copyright 2024 QRadioLink Contributors
#
# This file is part of gr-qradiolink
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# Select the release build type by default
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")

########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.16)
project(gr-qradiolink CXX C)
enable_testing()

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)

# Find gnuradio to get access to the cmake modules
find_package(Gnuradio "3.10" REQUIRED)

# Set version information
set(VERSION_MAJOR 1)
set(VERSION_API   0)
set(VERSION_ABI   0)
set(VERSION_PATCH 0)

include(GrVersion)
include(GNUInstallDirs)

if(NOT CMAKE_MODULES_DIR)
    set(CMAKE_MODULES_DIR ${CMAKE_INSTALL_LIBDIR}/cmake)
endif()

set(GR_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/gnuradio/qradiolink)
set(GR_CMAKE_DIR ${CMAKE_MODULES_DIR}/gnuradio-qradiolink)
set(GR_PKG_DATA_DIR ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})

########################################################################
# Setup dependencies
########################################################################
include(GrBoost)
include(GrMinReq)
include(GrCompilerSettings)

find_package(Volk REQUIRED)

########################################################################
# Check for required GNU Radio components via pkg-config
########################################################################
include(FindPkgConfig)
pkg_check_modules(GR_RUNTIME REQUIRED gnuradio-runtime)
pkg_check_modules(GR_BLOCKS REQUIRED gnuradio-blocks)
pkg_check_modules(GR_FILTER REQUIRED gnuradio-filter)
pkg_check_modules(GR_DIGITAL REQUIRED gnuradio-digital)
pkg_check_modules(GR_ANALOG REQUIRED gnuradio-analog)
pkg_check_modules(GR_FEC REQUIRED gnuradio-fec)
pkg_check_modules(GR_VOCODER REQUIRED gnuradio-vocoder)

########################################################################
# Check for optional ZMQ dependency (for mmdvm_source)
########################################################################
pkg_check_modules(ZMQ libzmq)

########################################################################
# Register component
########################################################################
include(GrComponent)
gr_register_component(
    "gr-qradiolink"
    ENABLE_GR_QRADIOLINK
    Boost_FOUND
    GR_RUNTIME_FOUND
    GR_BLOCKS_FOUND
    GR_FILTER_FOUND
    GR_DIGITAL_FOUND
    GR_ANALOG_FOUND
    GR_FEC_FOUND
    GR_VOCODER_FOUND
    Volk_FOUND)

########################################################################
# Begin conditional configuration
########################################################################
if(ENABLE_GR_QRADIOLINK)

    ########################################################################
    # Add subdirectories
    ########################################################################
    add_subdirectory(include/gnuradio/qradiolink)
    add_subdirectory(lib)
    add_subdirectory(docs)
    if(ENABLE_PYTHON)
        add_subdirectory(python/qradiolink)
        if(ENABLE_EXAMPLES)
            add_subdirectory(examples)
        endif(ENABLE_EXAMPLES)
    endif(ENABLE_PYTHON)
    if(ENABLE_GRC)
        add_subdirectory(grc)
    endif(ENABLE_GRC)
    if(ENABLE_TESTING)
        add_subdirectory(tests)
    endif(ENABLE_TESTING)
    if(ENABLE_FUZZING)
        add_subdirectory(fuzzing)
    endif(ENABLE_FUZZING)

    ########################################################################
    # Create Pkg Config File
    ########################################################################
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gnuradio-qradiolink.pc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/gnuradio-qradiolink.pc @ONLY)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gnuradio-qradiolink.pc
            DESTINATION ${GR_LIBRARY_DIR}/pkgconfig)

    ########################################################################
    # Install cmake search helper
    ########################################################################
    # Note: Will add cmake config files when ready

    ########################################################################
    # Create uninstall target
    ########################################################################
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/cmake_uninstall.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
        IMMEDIATE @ONLY
    )

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
        COMMENT "Uninstalling gr-qradiolink"
    )

endif(ENABLE_GR_QRADIOLINK)

