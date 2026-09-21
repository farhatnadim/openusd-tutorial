cmake_minimum_required(VERSION 3.20)
project(open_usd_tutorial LANGUAGES CXX)

# ---------------------------------------------------------------------------
# One build for every example in the repository.
#
#   cmake --preset default
#   cmake --build build
#   ctest --preset default          # builds nothing, runs everything
#
# Two trees are built:
#
#   examples/            scratch space. Drop in a .cpp or a directory of them
#                        and it is picked up automatically — nothing to edit.
#   nvidia_tutorials/    the C++ ports of NVIDIA's Learn OpenUSD lessons, each
#                        listed explicitly so the build mirrors the curriculum.
#
# How an example is actually compiled lives in cmake/OpenUSDExample.cmake, which
# each tutorial also includes when built on its own.
# ---------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(OpenUSDExample)

# --- examples/ ------------------------------------------------------------
# CONFIGURE_DEPENDS re-globs on build, so a newly added .cpp is picked up by
# `cmake --build` without re-running cmake by hand.

file(GLOB example_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/examples/*.cpp")
foreach(src IN LISTS example_sources)
    get_filename_component(name "${src}" NAME_WE)
    add_usd_example(${name} "${src}")
endforeach()

file(GLOB example_dirs CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/examples/*")
foreach(dir IN LISTS example_dirs)
    if(NOT IS_DIRECTORY "${dir}")
        continue()
    endif()
    get_filename_component(name "${dir}" NAME)
    file(GLOB_RECURSE srcs CONFIGURE_DEPENDS "${dir}/*.cpp")
    if(srcs)
        add_usd_example(${name} ${srcs})
        target_include_directories(${name} PRIVATE "${dir}")
    else()
        message(WARNING "examples/${name}/ has no .cpp files; skipping")
    endif()
endforeach()

# --- nvidia_tutorials/ ----------------------------------------------------
add_subdirectory(nvidia_tutorials)
