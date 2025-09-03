cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

add_compile_options(-fdiagnostics-color=always)


# Conditional toolchain inclusion based on build type
if(${CMAKE_BUILD_TYPE} STREQUAL "Test")
    set(CMAKE_PROJECT_NAME Speech2Touch_Test)
    # Don't include ARM toolchain for tests - use system default
    message("Building tests with system toolchain")
else()
    set(CMAKE_PROJECT_NAME Speech2Touch)
    # Include ARM toolchain for firmware builds
    include("cmake/gcc-arm-none-eabi.cmake")
    message("Building firmware with ARM toolchain")
endif()

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

if(${CMAKE_BUILD_TYPE} STREQUAL "Test")
    # Test build configuration
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    enable_language(C CXX)
    enable_testing()
    
    # Add test subdirectories
    add_subdirectory(Test)
    
else()
    # Firmware build configuration
    enable_language(C ASM)
    
    # Create an executable object type
    add_executable(${CMAKE_PROJECT_NAME})
    
    # Add STM32CubeMX generated sources
    add_subdirectory(cmake/stm32cubemx)
    
    # Link directories setup
    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
    )

    # Segger RTT sources
    set(SEGGER_RTT_SOURCES
        ${CMAKE_SOURCE_DIR}/Core/Lib/segger-rtt/SEGGER_RTT.c
        ${CMAKE_SOURCE_DIR}/Core/Lib/segger-rtt/SEGGER_RTT_printf.c
        # ${CMAKE_SOURCE_DIR}/Core/Lib/segger-rtt/SEGGER_RTT_Syscalls_GCC.c
    )

    # Add sources to executable
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here
        ${CMAKE_SOURCE_DIR}/Core/Src/audio.c
        ${CMAKE_SOURCE_DIR}/Core/Src/led.c
        ${CMAKE_SOURCE_DIR}/Core/Src/speech.c
        ${CMAKE_SOURCE_DIR}/Core/Src/touch.c
        ${CMAKE_SOURCE_DIR}/Core/Src/touch_targets.c
        ${CMAKE_SOURCE_DIR}/Core/Src/touch_mapper.c
        ${CMAKE_SOURCE_DIR}/Core/Lib/log/log.c
        ${SEGGER_RTT_SOURCES}
    )
    
    # Add include paths
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined include paths
        ${CMAKE_SOURCE_DIR}/Core/Lib/picovoice/include
        ${CMAKE_SOURCE_DIR}/Core/Lib/log
        ${CMAKE_SOURCE_DIR}/Core/Lib/segger-rtt
    )
    
    # Add project symbols (macros)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols
        __PV_LANGUAGE_ENGLISH__
        # Send AUDIO over USART
        # AUDIO_OVER_USART
        # Log colour output
        # LOG_USE_COLOR # not supported for RTT
    )
    
    # Add linked libraries
    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        ${CMAKE_SOURCE_DIR}/Core/Lib/picovoice/libpicovoice.a
        # Add user defined libraries
    )
endif()
