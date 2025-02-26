# Generated CMake Pico project file

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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Codigo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(Codigo main.c inc/ssd1306.c)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# Generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(Codigo ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Inclua o diretório inc
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

# Habilita saída USB para o Codigo
pico_enable_stdio_usb(Codigo 1)
pico_enable_stdio_uart(Codigo 0)

# Link com bibliotecas necessárias
target_link_libraries(Codigo PRIVATE pico_stdlib hardware_pio hardware_i2c hardware_adc hardware_timer hardware_pwm hardware_irq)

pico_add_extra_outputs(Codigo)

# Adicione um segundo executável (Codigo_parallel)
add_executable(Codigo_parallel)

pico_generate_pio_header(Codigo_parallel ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(Codigo_parallel PRIVATE ws2812_parallel.c)

target_compile_definitions(Codigo_parallel PRIVATE PIN_DBG1=3)

pico_enable_stdio_usb(Codigo_parallel 1)
pico_enable_stdio_uart(Codigo_parallel 0)

target_link_libraries(Codigo_parallel PRIVATE pico_stdlib hardware_pio hardware_dma hardware_i2c)

pico_add_extra_outputs(Codigo_parallel)

# Additionally generate python and hex pioasm outputs for inclusion in the RP2040 datasheet
add_custom_target(Codigo_datasheet DEPENDS ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
        COMMAND pioasm -o python ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.py
        VERBATIM)
add_dependencies(Codigo Codigo_datasheet)
