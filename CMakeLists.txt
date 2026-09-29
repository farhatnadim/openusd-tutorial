cmake_minimum_required(VERSION 3.20)
project(open_usd_tutorial LANGUAGES CXX)

# ---------------------------------------------------------------------------
# One build for every example in the repository.
#
#   cmake --preset default
#   cmake --build build
#   ctest --preset default          # builds nothing, runs everything
#
# The lessons live in nvidia_tutorials/: C++ ports of NVIDIA's Learn OpenUSD
# curriculum, each listed explicitly so the build mirrors the curriculum.
#
# How an example is actually compiled lives in cmake/OpenUSDExample.cmake, which
# each tutorial also includes when built on its own.
# ---------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(OpenUSDExample)

# --- nvidia_tutorials/ ----------------------------------------------------
add_subdirectory(nvidia_tutorials)
