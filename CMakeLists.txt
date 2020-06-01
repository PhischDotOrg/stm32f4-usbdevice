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
    ${CMAKE_CURRENT_SOURCE_DIR}/UsbDescriptors.cpp
)

# Selection of USB Interface
add_definitions("-DUSB_INTERFACE_VCP")
# add_definitions("-DUSB_INTERFACE_VENDOR")

# Selection of USB Application
add_definitions("-DUSB_APPLICATION_LOOPBACK")
# add_definitions("-DUSB_APPLICATION_UART")

# add_definitions("-DUSB_DEBUG")

###############################################################################
# Include a common CMake File that contains the stuff which would normally go
# in a top-level CMake File.
###############################################################################
add_subdirectory(common)
