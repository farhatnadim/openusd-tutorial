cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_primvars LANGUAGES CXX)

if(NOT COMMAND add_usd_example)
    include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/OpenUSDExample.cmake")
endif()

add_usd_example(14_primvar_interpolation example1.cpp LIBS usd usdGeom)
add_usd_example(14_primvar_deformation example2.cpp LIBS usd usdGeom)
