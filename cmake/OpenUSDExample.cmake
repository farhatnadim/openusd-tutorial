# ---------------------------------------------------------------------------
# OpenUSDExample.cmake — the single definition of how an example is built.
#
# This file is included from two directions, by design:
#
#   * the top-level CMakeLists.txt, which builds every example in the
#     repository — examples/ and nvidia_tutorials/ — in one tree;
#   * an individual tutorial's CMakeLists.txt, but only when that tutorial is
#     configured on its own. Each lesson directory stays a self-contained CMake
#     project a reader can copy out and build, without duplicating any of the
#     logic below.
#
# Adding an example never means editing this file.
# ---------------------------------------------------------------------------

include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd / IDE completion

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# --- locate OpenUSD --------------------------------------------------------
# -DUSD_ROOT wins, then $USD_ROOT, then a few conventional locations. An
# install is identified by its pxrConfig.cmake.

set(USD_ROOT "$ENV{USD_ROOT}" CACHE PATH "Root of the OpenUSD install")

if(NOT USD_ROOT)
    foreach(candidate
            "/media/nadim/Data/OpenUSD"
            "/media/nadim/Data/Source/OpenUSD-install/build/OpenUSD"
            "$ENV{HOME}/OpenUSDBuild"
            "$ENV{HOME}/USD"
            "/usr/local/USD"
            "/opt/USD")
        if(EXISTS "${candidate}/pxrConfig.cmake")
            set(USD_ROOT "${candidate}" CACHE PATH "Root of the OpenUSD install" FORCE)
            message(STATUS "Auto-detected USD_ROOT: ${USD_ROOT}")
            break()
        endif()
    endforeach()
endif()

if(NOT USD_ROOT OR NOT EXISTS "${USD_ROOT}/pxrConfig.cmake")
    message(FATAL_ERROR
        "No OpenUSD install found (looked for pxrConfig.cmake).\n"
        "Set it explicitly:\n"
        "  cmake --preset default -DUSD_ROOT=/path/to/OpenUSD\n"
        "or export USD_ROOT in your environment.")
endif()

list(APPEND CMAKE_PREFIX_PATH "${USD_ROOT}")

# The imaging targets in pxrTargets.cmake reference OpenGL::GL but pxrConfig
# never finds it, so the import fails at generate time unless we do it here.
find_package(OpenGL REQUIRED)

find_package(pxr REQUIRED CONFIG)
message(STATUS "OpenUSD ${PXR_VERSION} from ${USD_ROOT}")

# OpenUSD's own headers drag in two pieces of deprecated third-party code, and
# each announces itself on every translation unit:
#
#   * pxr/base/tf/hashset.h includes <ext/hash_set>, whose libstdc++ shim
#     emits a #warning unless __DEPRECATED is off (-Wno-deprecated);
#   * pxr/base/work pulls in tbb/task.h, which prints a #pragma message
#     unless TBB_SUPPRESS_DEPRECATED_MESSAGES is set.
#
# Neither is anything an example can change, so silence both here. Marking the
# USD headers SYSTEM also keeps the compiler from reporting ordinary warnings
# inside them.
add_library(usd_example_warnings INTERFACE)
target_compile_definitions(usd_example_warnings INTERFACE TBB_SUPPRESS_DEPRECATED_MESSAGES=1)
target_compile_options(usd_example_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-deprecated>)

# Everything an example needs when it does not name its own libraries:
# headers and the whole library set, so imaging/Hydra samples just work.
add_library(usd_example_env INTERFACE)
target_include_directories(usd_example_env SYSTEM INTERFACE ${PXR_INCLUDE_DIRS})
target_link_libraries(usd_example_env INTERFACE ${PXR_LIBRARIES} usd_example_warnings)
target_compile_features(usd_example_env INTERFACE cxx_std_20)

# Tests are registered by add_usd_example below; enable them once, in whichever
# directory is the top of this build.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
endif()

# ---------------------------------------------------------------------------
# add_usd_example(<name> <sources...> [LIBS <pxr libs...>])
#
# Turns one source list into a runnable, testable example target.
#
# LIBS names the pxr libraries the lesson actually demonstrates — the tutorials
# use it so the CMakeLists doubles as documentation of what each lesson needs.
# Omit it and the example links the full USD library set.
# ---------------------------------------------------------------------------
function(add_usd_example name)
    cmake_parse_arguments(ARG "" "" "LIBS" ${ARGN})
    set(sources ${ARG_UNPARSED_ARGUMENTS})

    if(NOT sources)
        message(FATAL_ERROR "add_usd_example(${name}): no sources given")
    endif()

    add_executable(${name} ${sources})

    if(ARG_LIBS)
        target_include_directories(${name} SYSTEM PRIVATE ${PXR_INCLUDE_DIRS})
        target_link_libraries(${name} PRIVATE ${ARG_LIBS} usd_example_warnings)
        target_compile_features(${name} PRIVATE cxx_std_20)
    else()
        target_link_libraries(${name} PRIVATE usd_example_env)
    endif()

    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        BUILD_RPATH   "${USD_ROOT}/lib"
        INSTALL_RPATH "${USD_ROOT}/lib")

    # Each example gets its own scratch directory, so examples that write files
    # cannot collide with each other and the source tree is never dirtied.
    #
    # Every example addresses its data as "_assets/..." relative to the working
    # directory, so that directory must exist before the example runs, and any
    # _assets/ checked in beside the source has to be staged into it.
    set(workdir "${CMAKE_BINARY_DIR}/output/${name}")
    file(MAKE_DIRECTORY "${workdir}/_assets")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/_assets")
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/_assets" "${workdir}/_assets"
            COMMENT "Staging _assets for ${name}"
            VERBATIM)
    endif()

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES WORKING_DIRECTORY "${workdir}")

    list(LENGTH sources n)
    message(STATUS "  example: ${name} (${n} source(s))")
endfunction()
