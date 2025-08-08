# CMake configuration for Google Test
cmake_minimum_required(VERSION 3.10)

project(TestFullLoop)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Download and unpack googletest at configure time
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test source files
add_executable(
  test_full_loop
  test_full_loop.cpp
)

target_link_libraries(
  test_full_loop
  GTest::gtest_main
  # harness_gui_lib
)

include(GoogleTest)
gtest_discover_tests(test_full_loop)
