# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(elec-vehicle C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add libraries // Only add if they have a CMakeLists.txt file
add_subdirectory(libs/mpu_9250)

add_library(pico_ssd1306 STATIC
        libs/pico-ssd1306/ssd1306.c
)
target_include_directories(pico_ssd1306 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/libs/pico-ssd1306
)
target_link_libraries(pico_ssd1306 PUBLIC
        pico_stdlib
        hardware_i2c
)

add_executable(elec-vehicle)

file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)

target_sources(elec-vehicle PRIVATE ${PROJECT_SOURCES})


pico_set_program_name(elec-vehicle "elec-vehicle")
pico_set_program_version(elec-vehicle "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(elec-vehicle 1)
pico_enable_stdio_usb(elec-vehicle 1)

# Add the standard include files to the build
target_include_directories(elec-vehicle PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)


# Add all libraries in the correct order
target_link_libraries(elec-vehicle 
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        hardware_spi
        hardware_i2c
        hardware_pwm
        pico_ssd1306
        pico_mpu_9250
        )

pico_add_extra_outputs(elec-vehicle)
