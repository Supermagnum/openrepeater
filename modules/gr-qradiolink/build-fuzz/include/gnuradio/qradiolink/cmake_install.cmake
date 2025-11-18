# Install script for directory: /home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gnuradio/qradiolink/gnuradio/qradiolink" TYPE FILE FILES
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/api.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_2fsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_4fsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_am.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_gmsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_bpsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_ssb.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_qpsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_nbfm.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_dsss.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_freedv.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_m17.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_dmr.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_mmdvm.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mod_mmdvm_multi2.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_2fsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_am.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_ssb.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_wbfm.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_nbfm.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_bpsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_qpsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_gmsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_4fsk.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_dsss.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_m17.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_freedv.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_dmr.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_mmdvm_multi.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/demod_mmdvm_multi2.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/gr_4fsk_discriminator.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/zero_idle_bursts.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mmdvm_source.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/mmdvm_sink.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/rssi_tag_block.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/clipper_cc.h"
    "/home/haaken/github-projects/qradiolink/gr-qradiolink/include/gnuradio/qradiolink/stretcher_cc.h"
    )
endif()

