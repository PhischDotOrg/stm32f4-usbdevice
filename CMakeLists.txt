#-
# $Copyright$
#
cmake_minimum_required(VERSION 3.13)
project(stm32f4-usbdevice)

###############################################################################
# Project-specific configuration
###############################################################################
set(PROJECT_NAME    stm32f4-minimal)
set(TARGET_NAME     firmware)
set(TARGET_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# add_definitions("-DUSB_DEBUG")

###############################################################################
# Include a common CMake File that contains the stuff which would normally go
# in a top-level CMake File.
###############################################################################
add_subdirectory(common)
