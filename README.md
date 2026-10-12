# open_usd_tutorial

A C++/CMake environment for running OpenUSD tutorial lessons against a local
OpenUSD install. The lessons live in
[`nvidia_tutorials/`](nvidia_tutorials): C++ ports of NVIDIA's Learn OpenUSD
curriculum, listed explicitly in curriculum order and indexed in
[`nvidia_tutorials/README.md`](nvidia_tutorials/README.md).

## Build

```sh
cmake --preset default        # configure (USD_ROOT comes from CMakePresets.json)
cmake --build --preset default
ctest --preset default        # run every example, compiled and in the REPL
```

Use `--preset debug` for an unoptimised build in `build-debug/`.

The OpenUSD install is located automatically: `-DUSD_ROOT=...` wins, then the
`USD_ROOT` environment variable, then conventional locations
(`/media/nadim/Data/OpenUSD`, `~/OpenUSDBuild`, `~/USD`, `/usr/local/USD`,
`/opt/USD`). An install is recognised by its `pxrConfig.cmake`.

## Adding a lesson

Lessons are listed rather than globbed, so `nvidia_tutorials/` reads as the
curriculum index: give the lesson a `CMakeLists.txt` calling
`add_usd_example`, then add one `add_subdirectory` line to
[`nvidia_tutorials/CMakeLists.txt`](nvidia_tutorials/CMakeLists.txt).

How an example is compiled is defined once, in
[`cmake/OpenUSDExample.cmake`](cmake/OpenUSDExample.cmake).

Build and run one lesson:

```sh
cmake --build --preset default --target 12_add_reference
(cd build/output/12_add_reference && ../../examples/12_add_reference)
```

## What you get per example

- The full OpenUSD library set by default (`PXR_LIBRARIES` — 80+ modules
  including Hydra and imaging). A lesson that declares exactly what it needs
  passes `LIBS` instead — the lessons in `nvidia_tutorials/` do, so each
  `CMakeLists.txt` documents the modules that lesson demonstrates.
- An rpath into the install's `lib`, so binaries run with no
  `DYLD_LIBRARY_PATH`.
- A private working directory at `build/output/<name>/` when run via `ctest`,
  so examples that write files don't overwrite each other.
- A CTest registration, so `ctest` is a regression check over everything.
- A second test, `repl_<name>`, that runs the same sources through the
  [REPL](#repl) when clang-repl is installed (label `repl`;
  `ctest --preset default -LE repl` skips them).
- `compile_commands.json` in `build/` for clangd / IDE completion.

## Editing in Neovim

`.nvim.lua` in the project root wires up build-and-run on save. It loads
automatically when you start `nvim` in this directory, provided your config has
`vim.o.exrc = true`; nvim asks once whether to trust the file (`:trust`).

Saving a `.cpp`/`.h` builds **only the target that file belongs to** and runs it
on success. The target comes from the nearest `CMakeLists.txt` that calls
`add_usd_example`. Program output goes to a `usd://output` split; the cursor
stays in your source. A compile error opens the quickfix list instead and skips the run.
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

## REPL

`repl/usd-repl` is a C++ REPL for OpenUSD built on clang-repl. Given a lesson,
it runs the lesson's source and leaves you at a prompt with its variables in
scope, so the next API call is one line away instead of one build away.

```sh
repl/usd-repl                                             # an empty session
repl/usd-repl nvidia_tutorials/08_xform/main.cpp          # run the lesson, keep the prompt
repl/usd-repl --batch nvidia_tutorials/08_xform/main.cpp  # run it and exit
repl/usd-repl --emit nvidia_tutorials/08_xform/main.cpp   # print the inputs it becomes
```

It needs a configured build. The compile flags come from
`build/compile_commands.json` and `USD_ROOT` from `build/CMakeCache.txt`, so the
REPL compiles exactly what the lessons compile. A lesson runs in
`build/output/<target>/` with its `_assets/` staged, as under ctest. The
backend is `clang-repl-18` (Ubuntu package `clang-tools-18`); set
`USD_REPL_CLANG_REPL` to use another.

A lesson source becomes REPL inputs like this:
- includes and file-scope declarations and functions go in as written
- `main()`'s body follows one statement at a time
- a final `return` is dropped

Every session starts from [`repl/usd_repl.h`](repl/usd_repl.h): the common USD
headers, `using namespace pxr;`, `show(x)` and `tree(stage)`.

```
usd> auto stage = UsdStage::CreateInMemory();
usd> auto sphere = UsdGeomSphere::Define(stage, SdfPath("/World/Ball"));
usd> sphere.GetRadiusAttr()
/World/Ball.radius (double) = 1
usd> for (auto p : stage->Traverse()) {
...>   std::cout << p.GetPath() << std::endl;
...> }
/World
/World/Ball
usd> tree(stage)
def "World"
  def Sphere "Ball"
```

- **Bare expressions print.** An expression typed without a semicolon prints its
  value:
  - a stage prints its usda
  - a prim prints its specifier, type, authored properties and children
  - an attribute prints its value and time samples

  `show(x)` does the same explicitly. A pasted block that ends in a bare
  expression prints it, as a notebook cell does.
- **When an input runs.** Enter runs an input once its brackets balance and
  its statement is finished, and otherwise starts a new line. A multi-line `if`
  or `try` runs after an empty line, so an `else` or `catch` can still follow.
  Esc then Enter runs a block as it stands. A pasted block can mix `#include`
  lines and code.
- **Crash recovery.** Ctrl-C stops a running input; `->` on a null stage, or any
  other crash, ends the backend. Either way the session is rebuilt from a journal
  of the inputs that worked, without the one that failed. A warm spare backend
  makes that take about a second.
- **Saving.**
  - `%save FILE` writes the journal as a `.repl` file. `repl/usd-repl FILE`,
    `%load` or `--batch` runs it again.
  - `%save-cpp FILE` writes it as a C++ program:
    - the prelude is included (by a relative path inside the repository)
    - directives and definitions sit at file scope
    - everything else goes in `main()`, and bare expressions become
      `show(...)` calls

    It builds like a lesson, and the REPL runs it again.
- **Commands.** `%help` lists them:
  - `%undo`
  - `%restart`
  - `%reset`
  - `%journal`
  - `%save FILE`
  - `%save-cpp FILE`
  - `%load FILE`
  - `%lib PATH`
  - `%quit`

### Editing and colour

With prompt_toolkit installed (`sudo apt install python3-prompt-toolkit`),
input works as in IPython:
- C++ is highlighted as you type.
- A multi-line block is one buffer. The arrow keys move between its lines,
  Enter after `{` indents, and `}` dedents.
- Tab opens a menu of USD class and method names and the session's own names.
- Ctrl-R searches the history, and → takes the grey suggestion it offers.

Without prompt_toolkit, input goes through readline one line at a time, with the
same completion words.

Output is coloured when it goes to a terminal:
- usda through Pygments' USD lexer
- clang's diagnostics in clang's colours
- USD's errors and warnings by severity

`--no-color` or `NO_COLOR=1` turns colour off.

### Why a front end

clang-repl runs the code. `repl/usd-repl` sits in front because typing straight
into clang-repl 18 runs into the following:

| Typed into clang-repl 18 | What happens | What usd-repl does |
| --- | --- | --- |
| A line ending in an open `{` | An endless error loop; Ctrl-C ends the session ([llvm#127569](https://github.com/llvm/llvm-project/pull/127569)) | Waits for the block to close |
| A top-level range-for | The loop variable keeps its first value, and a second loop fails to link ([llvm#89506](https://github.com/llvm/llvm-project/issues/89506)) | Runs top-level `for`, `while`, `do`, `if`, `switch` and `try` inside `[]{ ... }();` |
| A top-level `try`/`catch` | Link errors that break later inputs | The same lambda |
| Tab after `stage->` | clang-repl segfaults | Completes USD class and method names itself |
| `stage` | Prints nothing | Prints the value |
| A crash | Everything typed is lost, and Ubuntu files a crash report | Rebuilds the session, and no report is written |

Both upstream issues are still open. Since LLVM 22, clang-repl prints values of
built-in types only.

### Limits

- **Each input is compiled on its own, and a name can't be defined twice.**
  `auto stage = ...` a second time is an error. Assign instead
  (`stage = ...`), pick a new name, or `%undo`.
- **Variables typed at the prompt are globals.** A name can therefore clash with
  a C library function: `int index = 3;` fails with "redefinition of 'index' as
  different kind of symbol". Lessons avoid this, because a helper function keeps
  its locals local.
- **Rebuilding replays the journal, file writes included.**
- **Typed code can't read stdin**, because the front end uses it to talk to the
  backend.
- **Raw string literals are rewritten.** Each input is sent as a single line, so
  a multi-line raw string becomes an ordinary escaped literal.
- **Startup takes about 4 s** while the USD headers parse. After that a line
  takes 10–50 ms.

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
