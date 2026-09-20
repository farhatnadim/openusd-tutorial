# Codex builds on Neovim save

Saving a C++ source or header in `nvidia_tutorials` runs the build script
directly in the background, without invoking an agent. Use `:BuildAgent` to
request a Codex build with **gpt-6-astra, medium reasoning**, and `$build-cpp`,
including precise fix suggestions grounded in compiler errors and inspected code. Agent instructions prohibit changing
source or CMake files. Neither mode runs the application.

## Install

```bash
python3 tools/nvim-codex-build/install.py
```

The installer preserves your settings, backs up `init.lua`, and links the Lua
module into `~/.config/nvim/lua`. Keep this bundle in place: the module resolves
its link to locate the build runner and response schema. Restart Neovim, or run
`:luafile ~/.config/nvim/init.lua`. Requires Neovim 0.9 or newer, Bash, CMake, and GNU Make. Only
`:BuildAgent` requires an authenticated Codex CLI on Neovim's PATH.

## Use

- Save (`:w`) for a direct build of a `.cpp`, `.cc`, `.cxx`, `.c++`, `.C`, `.h`, `.hpp`, `.hh`, or `.hxx` file.
- A bottom panel opens automatically without taking focus from your code. It
  shows elapsed time and refreshes compiler progress every 500 ms. The final
  **Build success**, **Build failed**, or **Agent failed** result stays visible.
  On completion the panel scrolls to the top, showing the result and any agent
  fix suggestion before compiler output. Direct failures show the first compiler
  error at the top. Long explanations wrap.
- Compiler errors open quickfix while your editing window retains focus. Use
  `:cnext`, `:cprevious`, or Enter in quickfix to visit errors.
- `:BuildLog` opens the live compiler output and the agent's explanation.
  You can close the panel with `:q`; it reopens for the next build.
- `:BuildAgent` builds the current saved application using Codex and suggests
  minimal fixes with file:line, a brief cause, and exact replacement code. It
  omits speculative advice and cascading errors, and never applies the fixes.
  Suggestions appear first in quickfix, including replacement code; press Enter
  to jump to the source location. Multiline replacements occupy consecutive
  entries pointing to the same location. Raw compiler diagnostics follow. It does not save unsaved edits and works with automatic builds disabled.
- `:CodexBuild` directly builds the current file's application from disk, even when
  automatic builds are disabled. It does not save unsaved edits.
- `:CodexBuildToggle` toggles automatic builds for this Neovim session. An active
  build may finish; pending builds are discarded when disabled.

Only this tutorial tree is enabled. Generated build files and files outside it
are ignored. Saves within 400 ms are grouped; only one build runs at a time in a
Neovim instance. Saves during a build queue another build of the affected
project. Results superseded by newer saves are not presented as current errors.
A save during an agent build queues a direct build. An explicit agent request
queued for a project is retained if further saves arrive before it starts.
Use one Neovim instance per project while automatic builds are enabled.

## Build and output

The runner activates `~/envs/general/bin/activate`, then executes:

```bash
cmake -S PROJECT -B PROJECT/build/codex-nvim \
  -G 'Unix Makefiles' -DCMAKE_PREFIX_PATH=/media/nadim/Data/OpenUSD
cmake --build PROJECT/build/codex-nvim --parallel 2
```

This generator makes `cmake --build` use GNU Make. Each tutorial currently has
one application, so building its default target builds the containing app.
The separate build directory avoids existing stale caches. The OpenUSD drive
must be mounted. No clean rebuild is performed on save.

Each run keeps `build.log`, `build-status.json`, and `report.txt` under
`PROJECT/build/codex-nvim/.codex-runs/`. Logs are retained for inspection; remove
old run directories when needed. Agent runs also keep `response.json`,
`agent.jsonl`, and `agent.stderr`. Compiler output is preserved in `build.log`;
OpenUSD deprecation warnings are excluded from quickfix.

Direct builds require a successful runner exit and build status. For agent builds,
the runner's exit status determines build failure; Codex exiting successfully
is not sufficient. Authentication, missing tools, incomplete responses, and
sandbox failures are reported through `:BuildLog`. The CLI uses
`--approve-for-me`: workspace-write sandboxing with automatic approval review.
This machine currently cannot start the sandbox (a bwrap loopback error), so the
agent can request reviewed execution of reads and the fixed build runner.
Rejected approvals are reported in the log; no sandbox bypass flag is used.
Build-only source handling is an agent instruction; workspace-write itself
allows writing the project directory.

## Configuration and removal

The installation adds `require('codex_build').setup()` to `init.lua`. Optional
setup fields are `root`, `codex`, `skill`, `activate`, `usd_prefix`, and
`debounce_ms`. Defaults match this machine's OpenUSD setup. Astra and medium
reasoning are pinned in the invocation.

To remove the hook, remove its `require` line and comment from `init.lua`, then
remove the `lua/codex_build.lua` symlink. Existing source files are unaffected.

## Verify

Run `bash tools/nvim-codex-build/tests/run.sh` from the tutorial directory.
Tests compile temporary CMake projects directly and use a fake Codex process for
explicit agent commands, avoiding model calls.

Codex CLI interfaces: [non-interactive mode](https://learn.chatgpt.com/docs/non-interactive-mode).

After updating the Lua module, restart Neovim to load the changes.
