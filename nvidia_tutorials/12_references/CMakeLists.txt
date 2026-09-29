cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_references LANGUAGES CXX)

if(NOT COMMAND add_usd_example)
    include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OpenUSDExample.cmake")
endif()

# Two independent examples, one target each. Only example 2 reads the staged
# _assets/cubebox_a02, but both get the copy; it is harmless for example 1.
add_usd_example(12_add_reference example1.cpp LIBS usd usdGeom)
add_usd_example(12_reference_external_asset example2.cpp LIBS usd usdGeom)
