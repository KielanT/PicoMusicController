# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(PicoMusicControllerUSB C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PicoMusicControllerUSB PicoMusicControllerUSB.cpp usb_descriptors.cpp)


pico_set_program_name(PicoMusicControllerUSB "PicoMusicControllerUSB")
pico_set_program_version(PicoMusicControllerUSB "0.1")

# Generate PIO header

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoMusicControllerUSB 0)
pico_enable_stdio_usb(PicoMusicControllerUSB 0)

# Add the standard library to the build
target_link_libraries(PicoMusicControllerUSB
        pico_stdlib pico_unique_id tinyusb_device tinyusb_board)

# Add the standard include files to the build
target_include_directories(PicoMusicControllerUSB PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

add_subdirectory(external/pico-ssd1306)

# Add any user requested libraries
target_link_libraries(PicoMusicControllerUSB 
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_adc
        pico_multicore
        pico_ssd1306
        )

pico_add_extra_outputs(PicoMusicControllerUSB)

