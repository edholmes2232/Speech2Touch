cmake_minimum_required(VERSION 3.10)

project(HarnessGuiLib VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(UI_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Franke-A600.ui")

add_library(harness_gui_lib)

set_source_files_properties(${CMAKE_SOURCE_DIR}/../../../Core/Src/touch_targets.c PROPERTIES LANGUAGE C)


target_sources(harness_gui_lib PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
                ${CMAKE_SOURCE_DIR}/../../../Core/Src/touch_targets.c
                ${UI_FILE})

target_include_directories(harness_gui_lib PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../Core/Inc
)


# Link Qt libraries
target_link_libraries(harness_gui_lib Qt6::Widgets)
