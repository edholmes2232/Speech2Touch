cmake_minimum_required(VERSION 3.14)
project(tts)

# Create the library target for your TTS code
add_library(tts_lib STATIC tts.cpp tts.h)

target_include_directories(tts_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)