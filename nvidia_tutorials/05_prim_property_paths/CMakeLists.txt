cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_prim_property_paths LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(05_prim_property_paths main.cpp)
target_compile_features(05_prim_property_paths PRIVATE cxx_std_20)
target_link_libraries(05_prim_property_paths PRIVATE usd usdGeom)
