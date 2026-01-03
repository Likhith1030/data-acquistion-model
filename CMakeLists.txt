cmake_minimum_required(VERSION 3.16)
project(DataAcquisitionModel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Compiler options ---
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
  -Wall -Wextra -Wpedantic
  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:Debug>:-g -O0 -fsanitize=address,undefined>
)
add_link_options(
  $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# --- Library ---
file(GLOB_RECURSE DAQ_SOURCES src/*.cpp)
add_library(daq_lib STATIC ${DAQ_SOURCES})
target_include_directories(daq_lib PUBLIC include)
target_link_libraries(daq_lib PUBLIC pthread)

# --- Main executable ---
add_executable(daq_demo main.cpp)
target_link_libraries(daq_demo PRIVATE daq_lib)

# --- Unit tests (Google Test via FetchContent) ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(daq_tests ${TEST_SOURCES})
target_include_directories(daq_tests PRIVATE include)
target_link_libraries(daq_tests PRIVATE daq_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(daq_tests)
