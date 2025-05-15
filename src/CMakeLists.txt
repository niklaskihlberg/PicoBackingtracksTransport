# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake
		$ENV{PICO_SDK_PATH}lib/tinyusb/examples/device/midi_test/src/tusb_config.h
		$ENV{PICO_SDK_PATH}lib/tinyusb/examples/device/midi_test/src/usb_descriptors.c
)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(PicoBackingtracksTransport)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    main.cpp
)

# Inkludera TinyUSB och Pico SDK-bibliotek # Är det den här som sabbar?
# target_sources(PicoBackingtracksTransport PRIVATE main.cpp)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib pico_unique_id tinyusb_device tinyusb_board
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
