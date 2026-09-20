#!/usr/bin/env bash
set -euo pipefail
test_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
fixture_dir=$(mktemp -d /tmp/codex-nvim-tests.XXXXXX)
trap 'rm -rf -- "$fixture_dir"' EXIT
export CODEX_BUILD_TEST_DIR="$fixture_dir"
export CODEX_BUILD_TEST_TRACE="$fixture_dir/trace.jsonl"
export CODEX_BUILD_TEST_SOURCE="$test_dir"
nvim --headless -u NONE -i NONE -n "+lua dofile(vim.env.CODEX_BUILD_TEST_SOURCE .. '/test_hook.lua')"
