# CMake configuration for Google Test
cmake_minimum_required(VERSION 3.10)

project(GoogleTestDemo)

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
  basic_gtest
  test_basic.cpp
)

target_link_libraries(
  basic_gtest
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(basic_gtest)
