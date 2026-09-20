cmake_minimum_required(VERSION 3.20)

project(open_usd_tutorial_cpp LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(stage_tutorial stage_tutorial.cpp)
target_compile_features(stage_tutorial PRIVATE cxx_std_20)
target_link_libraries(stage_tutorial PRIVATE usd)
