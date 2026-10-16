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

# --- repl/TUTORIAL.md -------------------------------------------------------
# A lesson explored at the prompt. cpp-repl's checker runs its transcripts
# again through repl/usd-repl and compares them; it comes with cpp-repl, so a
# pin from before it gives no test rather than a failing one.
if(USD_REPL_CLANG_REPL AND EXISTS "${cpp_repl_SOURCE_DIR}/scripts/check_tutorial.py")
    add_test(NAME tutorial_repl
             COMMAND python3 "${cpp_repl_SOURCE_DIR}/scripts/check_tutorial.py"
                     --root "${CMAKE_SOURCE_DIR}"
                     --cpp-repl "${USD_REPL_SCRIPT} --build-dir ${CMAKE_BINARY_DIR}"
                     --clang-repl "${USD_REPL_CLANG_REPL}"
                     "${CMAKE_SOURCE_DIR}/repl/TUTORIAL.md")
    # The lesson writes build/output/08_xform/_assets, as its own tests do;
    # 77 is the checker's "recorded with another clang-repl".
    set_tests_properties(tutorial_repl PROPERTIES LABELS repl RESOURCE_LOCK 08_xform SKIP_RETURN_CODE 77)
endif()
