#!/usr/bin/env bash
# Build-only entry point used by the Neovim Codex agent. Never cleans or runs the app.
set -o pipefail

if [[ $# -ne 5 ]]; then
    printf 'Usage: bash %s PROJECT_ROOT BUILD_DIR ENV_ACTIVATE USD_PREFIX RUN_DIR\n' "$0" >&2
    exit 2
fi

# Establish a safe output location before opening logs or writing status files.
project_root=$(realpath -e -- "$1") || exit 2
build_dir=$(realpath -e -- "$2") || exit 2
env_activate=$3
usd_prefix=$4
run_dir=$(realpath -e -- "$5") || exit 2
if [[ ! -d "$project_root" || ! -d "$build_dir" ||
      "$build_dir" != "$project_root/build/codex-nvim" ]]; then
    printf 'Refusing build directory: expected PROJECT_ROOT/build/codex-nvim.\n' >&2
    exit 2
fi
run_name=${run_dir#"$build_dir/.codex-runs/"}
if [[ ! -d "$run_dir" || "$run_name" == "$run_dir" ||
      -z "$run_name" || "$run_name" == */* ]]; then
    printf 'Refusing run directory: expected a direct child of BUILD_DIR/.codex-runs.\n' >&2
    exit 2
fi

stage=environment
write_status() {
    local result=$1 status_tmp
    status_tmp=$(mktemp "$run_dir/.build-status.XXXXXX") || return 1
    if ! printf '{"exit_code":%d,"stage":"%s"}\n' "$result" "$stage" > "$status_tmp"; then
        rm -f -- "$status_tmp"
        return 1
    fi
    if ! mv -f -- "$status_tmp" "$run_dir/build-status.json"; then
        rm -f -- "$status_tmp"
        return 1
    fi
}

if [[ -L "$run_dir/build.log" || ( -e "$run_dir/build.log" && ! -f "$run_dir/build.log" ) ]]; then
    printf 'Refusing non-regular build log.\n' >&2
    write_status 2
    exit 2
fi

# Wait for the pipeline to finish its status trap when the whole job is stopped.
trap 'exit 130' INT
trap 'exit 143' TERM

# The group preserves the activated environment for both CMake commands. Its EXIT
# trap records the failing command's status, independently of tee's exit status.
(
    trap 'result=$?; write_status "$result" || exit 1' EXIT
    # tee may exit before the status trap during a process-group interruption.
    trap '' PIPE
    trap 'exit 130' INT
    trap 'exit 143' TERM

    fail() { printf '%s\n' "$1" >&2; exit 2; }
    [[ -r "$project_root/CMakeLists.txt" && -f "$project_root/CMakeLists.txt" ]] ||
        fail 'Project root has no readable CMakeLists.txt.'
    [[ -r "$env_activate" && -f "$env_activate" ]] ||
        fail "Environment activation file is missing or unreadable: $env_activate"
    [[ -r "$usd_prefix/pxrConfig.cmake" && -f "$usd_prefix/pxrConfig.cmake" ]] ||
        fail "OpenUSD prefix has no readable pxrConfig.cmake: $usd_prefix"
    # shellcheck disable=SC1090
    source "$env_activate"
    result=$?
    [[ $result -eq 0 ]] || exit "$result"
    command -v cmake >/dev/null 2>&1 || fail 'cmake is not available in the activated environment.'
    command -v make >/dev/null 2>&1 || fail 'make is not available in the activated environment.'

    stage=configure
    cache_file="$build_dir/CMakeCache.txt"
    [[ ! -L "$cache_file" ]] || fail 'Refusing a symlinked CMake cache.'
    if [[ -e "$cache_file" ]]; then
        [[ -f "$cache_file" && -r "$cache_file" ]] || fail 'CMake cache is not a readable regular file.'
        cache_generator= cache_source= cache_directory=
        while IFS= read -r cache_line; do
            case "$cache_line" in
                CMAKE_GENERATOR:INTERNAL=*) cache_generator=${cache_line#*=} ;;
                CMAKE_HOME_DIRECTORY:INTERNAL=*) cache_source=${cache_line#*=} ;;
                CMAKE_CACHEFILE_DIR:INTERNAL=*) cache_directory=${cache_line#*=} ;;
            esac
        done < "$cache_file"
        [[ "$cache_generator" == 'Unix Makefiles' ]] ||
            fail 'Existing CMake cache does not use Unix Makefiles; select a fresh build directory.'
        [[ -n "$cache_source" && "$(realpath -m -- "$cache_source")" == "$project_root" ]] ||
            fail 'Existing CMake cache belongs to a different source directory.'
        [[ -n "$cache_directory" && "$(realpath -m -- "$cache_directory")" == "$build_dir" ]] ||
            fail 'Existing CMake cache was created in a different build directory.'
    fi

    cmake -S "$project_root" -B "$build_dir" -G 'Unix Makefiles' \
        -DCMAKE_PREFIX_PATH="$usd_prefix"
    result=$?
    [[ $result -eq 0 ]] || exit "$result"

    stage=build
    cmake --build "$build_dir" --parallel 2
    exit "$?"
) 2>&1 | tee "$run_dir/build.log"
pipeline_status=("${PIPESTATUS[@]}")
if [[ ${pipeline_status[0]} -ne 0 ]]; then
    exit "${pipeline_status[0]}"
fi
if [[ ${pipeline_status[1]} -ne 0 ]]; then
    # Compilation succeeded, but the incomplete log makes this run unusable.
    stage=build
    write_status "${pipeline_status[1]}"
    exit "${pipeline_status[1]}"
fi
exit 0
