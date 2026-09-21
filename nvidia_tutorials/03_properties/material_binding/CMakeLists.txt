cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_material_binding LANGUAGES CXX)

# Give editors and language servers the exact include paths and compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pxr REQUIRED CONFIG)

add_executable(03_material_binding main.cpp)
target_compile_features(03_material_binding PRIVATE cxx_std_20)
target_link_libraries(03_material_binding PRIVATE usd usdGeom usdShade)
