cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_timecodes_timesamples LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(04_timecodes_timesamples main.cpp)
target_compile_features(04_timecodes_timesamples PRIVATE cxx_std_20)
target_link_libraries(04_timecodes_timesamples PRIVATE usd usdGeom)
