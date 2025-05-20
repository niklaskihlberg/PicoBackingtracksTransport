# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
if(APPLE) # Välj SDK-path beroende på plattform
    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake) # Använd miljövariabel på Mac
else()
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake) # Använd miljövariabel på Linux/GitHub Actions
endif()

# Set name of project (as PROJECT_NAME) and C/C   standards
project(PicoBackingtracksTransport)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Definiera TinyUSB-konfigurationer
add_compile_definitions(CFG_TUSB_RHPORT0_MODE=OPT_MODE_DEVICE)
# add_compile_definitions(CFG_TUSB_RHPORT1_MODE=OPT_MODE_NONE)


# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    main.cpp
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(PicoBackingtracksTransport PUBLIC
${CMAKE_CURRENT_LIST_DIR})

# Inkludera TinyUSB och Pico SDK-bibliotek # Är det den här som sabbar?
target_sources(PicoBackingtracksTransport PUBLIC main.cpp usb_descriptors.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib pico_unique_id tinyusb_device tinyusb_board
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

