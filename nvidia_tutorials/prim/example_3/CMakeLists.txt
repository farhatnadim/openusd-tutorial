cmake_minimum_required(VERSION 3.20)
project(pixar_openusd_tutorials LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(example3 example3.cpp)
target_compile_features(example3 PRIVATE cxx_std_20)
target_link_libraries(example3 PRIVATE usd usdGeom)
