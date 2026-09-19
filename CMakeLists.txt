cmake_minimum_required(VERSION 3.20)
project(open_usd_tutorial LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Drop-in example build environment for OpenUSD.
#
# There is nothing to edit below when you add a tutorial example. Either:
#
#   examples/my_example.cpp        -> target "my_example"      (single file)
#   examples/my_example/*.cpp      -> target "my_example"      (multi file)
#
# Then `cmake --build build` picks it up. Every example links the full set of
# USD libraries, so imaging/Hydra samples work without changing anything here.
# ---------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd / IDE completion

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Locate the OpenUSD install: -DUSD_ROOT wins, then $USD_ROOT, then a few
# conventional locations. An install is identified by its pxrConfig.cmake.
set(USD_ROOT "$ENV{USD_ROOT}" CACHE PATH "Root of the OpenUSD install")

if(NOT USD_ROOT)
    foreach(candidate
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
        "  cmake --preset default -DUSD_ROOT=/path/to/OpenUSDBuild\n"
        "or export USD_ROOT in your environment.")
endif()

list(APPEND CMAKE_PREFIX_PATH "${USD_ROOT}")

# The imaging targets in pxrTargets.cmake reference OpenGL::GL but pxrConfig
# never finds it, so the import fails at generate time unless we do it here.
find_package(OpenGL REQUIRED)

find_package(pxr REQUIRED)
message(STATUS "OpenUSD ${PXR_VERSION} from ${USD_ROOT}")

# Everything an example needs: headers, the whole library set, and an rpath so
# the binaries run straight from the build tree with no DYLD_LIBRARY_PATH.
add_library(usd_example_env INTERFACE)
target_include_directories(usd_example_env INTERFACE ${PXR_INCLUDE_DIRS})
target_link_libraries(usd_example_env INTERFACE ${PXR_LIBRARIES})
target_compile_features(usd_example_env INTERFACE cxx_std_17)

enable_testing()

# Turn one source list into a runnable example target.
function(add_usd_example name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE usd_example_env)
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        BUILD_RPATH   "${USD_ROOT}/lib"
        INSTALL_RPATH "${USD_ROOT}/lib")

    # Give each example its own scratch directory so examples that write files
    # can't collide with each other.
    set(workdir "${CMAKE_BINARY_DIR}/output/${name}")
    file(MAKE_DIRECTORY "${workdir}")

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES WORKING_DIRECTORY "${workdir}")

    list(LENGTH ARGN n)
    message(STATUS "  example: ${name} (${n} source(s))")
endfunction()

# --- discovery ------------------------------------------------------------
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

if(NOT TARGET usd_example_env)
    message(FATAL_ERROR "internal: usd_example_env not defined")
endif()
