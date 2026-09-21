cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_stage LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(01_stage main.cpp)
target_compile_features(01_stage PRIVATE cxx_std_20)
target_link_libraries(01_stage PRIVATE usd)
