# open_usd_tutorial

A drop-in C++/CMake environment for running OpenUSD tutorial examples against a
local OpenUSD install. Add a `.cpp`, build, run. No per-example compile scripts
and no CMake edits.

## Build

```sh
cmake --preset default        # configure (USD_ROOT comes from CMakePresets.json)
cmake --build --preset default
ctest --preset default        # run every example, fail on non-zero exit
```

Use `--preset debug` for an unoptimised build in `build-debug/`.

The OpenUSD install is located automatically: `-DUSD_ROOT=...` wins, then the
`USD_ROOT` environment variable, then conventional locations
(`~/OpenUSDBuild`, `~/USD`, `/usr/local/USD`, `/opt/USD`). An install is
recognised by its `pxrConfig.cmake`.

## Adding an example

Two layouts, both discovered automatically:

```
examples/my_example.cpp        ->  target "my_example"     (single file)
examples/my_example/*.cpp      ->  target "my_example"     (multi file)
```

The glob uses `CONFIGURE_DEPENDS`, so `cmake --build --preset default` after
dropping in a file re-configures and builds it. You never edit CMakeLists.txt.

Build and run one example:

```sh
cmake --build --preset default --target my_example
./build/examples/my_example
```

## What you get per example

- The full OpenUSD library set (`PXR_LIBRARIES` — 80+ modules including Hydra
  and imaging), so tutorial code links regardless of which module it uses.
- An rpath into the install's `lib`, so binaries run with no
  `DYLD_LIBRARY_PATH`.
- A private working directory at `build/output/<name>/` when run via `ctest`,
  so examples that write files don't overwrite each other.
- A CTest registration, so `ctest` is a regression check over everything.
- `compile_commands.json` in `build/` for clangd / IDE completion.

## Included examples

| Example | What it shows |
| --- | --- |
| `00_sphere_smoke_test` | Install smoke test: author a sphere, save, reopen, verify |
| `01_hello_stage` | In-memory stage, `ExportToString` to stdout, no disk I/O |
| `02_mesh_pyramid` | Multi-file target; builds a `UsdGeomMesh` from generated data |

## Editing in Neovim

`.nvim.lua` in the project root wires up build-and-run on save. It loads
automatically when you start `nvim` in this directory, provided your config has
`vim.o.exrc = true`; nvim asks once whether to trust the file (`:trust`).

Saving a `.cpp`/`.h` builds **only the target that file belongs to** and runs it
on success. Program output goes to a `usd://output` split; the cursor stays in
your source. A compile error opens the quickfix list instead and skips the run.
Saving `CMakeLists.txt` or `CMakePresets.json` rebuilds everything and runs
nothing.

| Command | Effect |
| --- | --- |
| `:Build` | Build the current example now |
| `:Run` | Build and run the current example now |
| `:BuildAll` | Build every example |
| `:Target foo` | Pin to target `foo` (no argument = automatic) |
| `:RunArgs a b` | Arguments passed to the example (no args = clear) |
| `:View [file]` | Open the example's newest stage in usdview |
| `:ViewClose` | Close the usdview window we opened |
| `:Check` | Run `usdchecker` on the example's newest stage |
| `:BuildOnSave` | Toggle build-on-save |
| `:RunOnSave` | Toggle run-after-build |
| `:UsdCat` | Toggle the usdcat dump |

Each run uses `build/output/<target>/` as its working directory, matching the
ctest setup.

After a successful run, any `.usda`/`.usdc`/`.usd`/`.usdz` the example wrote is
piped through `usdcat` into the same split, so binary crate files show up as
readable text. The dump is capped at 200 lines per file; `:UsdCat` turns it off.

`:View` is deliberately manual rather than on-save — usdview has no remote
reload, so refreshing means replacing the window and losing your camera. It
does replace the previous viewer instead of stacking windows, and closes when
nvim exits (it runs as a child job).

## Notes

- `find_package(OpenGL)` is called before `find_package(pxr)`. The imaging
  targets in `pxrTargets.cmake` reference `OpenGL::GL`, but `pxrConfig.cmake`
  never finds it, so configuring fails at generate time without this.
- `compile_commands.json` lands in `build/`, which clangd finds on its own via
  its `build/` subdirectory heuristic.
- usdview needs the Python bindings on `PYTHONPATH`; `.nvim.lua` sets that up
  by reading `USD_ROOT` back out of `build/CMakeCache.txt` and globbing
  `lib/python*/site-packages`, so nothing is hardcoded.
- Stage metadata matters: author `upAxis` and `metersPerUnit` or `usdchecker`
  reports the result as invalid even though it opens fine.
