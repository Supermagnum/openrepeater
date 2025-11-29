# Install script for directory: /home/haaken/github-projects/gr-qradiolink/grc

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
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/gnuradio/grc/blocks" TYPE FILE FILES
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_2fsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_4fsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_am.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_gmsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_bpsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_ssb.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_qpsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_nbfm.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_dsss.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_nxdn.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_mod_dpmr.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_2fsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_am.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_ssb.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_wbfm.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_nbfm.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_bpsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_qpsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_gmsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_4fsk.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_dsss.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_m17.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_nxdn.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_demod_dpmr.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink_m17_deframer.block.yml"
    "/home/haaken/github-projects/gr-qradiolink/grc/qradiolink.tree.yml"
    )
endif()

