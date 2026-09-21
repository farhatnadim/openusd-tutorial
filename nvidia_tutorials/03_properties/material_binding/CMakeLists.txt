cmake_minimum_required(VERSION 3.20)
project(openusd_tutorial_material_binding LANGUAGES CXX)

# Built as part of the repository-wide build, the top level has already defined
# add_usd_example. Configured on its own, this pulls in the same definition from
# the same file — so this directory is a complete project either way.
if(NOT COMMAND add_usd_example)
    include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/OpenUSDExample.cmake")
endif()

add_usd_example(03_material_binding main.cpp LIBS usd usdGeom usdShade)
