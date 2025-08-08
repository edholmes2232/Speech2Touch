cmake_minimum_required(VERSION 3.10)

set(CMAKE_PROJECT_NAME FrankeA600)
enable_language(CXX C)

project(${CMAKE_PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(UI_FILE "${CMAKE_SOURCE_DIR}/Franke-A600.ui")

add_executable(${CMAKE_PROJECT_NAME})

set_source_files_properties(${CMAKE_SOURCE_DIR}/../../../Core/Src/touch_targets.c PROPERTIES LANGUAGE C)


target_sources(${CMAKE_PROJECT_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}/main.cpp
                ${CMAKE_SOURCE_DIR}/mainwindow.cpp
                ${CMAKE_SOURCE_DIR}/mainwindow.h
                ${CMAKE_SOURCE_DIR}/../../../Core/Src/touch_targets.c
                ${UI_FILE})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../Core/Inc
)


# Link Qt libraries
target_link_libraries(${CMAKE_PROJECT_NAME} Qt6::Widgets)
