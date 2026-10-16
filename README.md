# open_usd_tutorial

A C++/CMake environment for running OpenUSD tutorial lessons against a local
OpenUSD install. The lessons live in
[`nvidia_tutorials/`](nvidia_tutorials): C++ ports of NVIDIA's Learn OpenUSD
curriculum, listed explicitly in curriculum order and indexed in
[`nvidia_tutorials/README.md`](nvidia_tutorials/README.md).

## Build

```sh
cmake --preset default        # configure; finds the OpenUSD install (below)
cmake --build --preset default
ctest --preset default        # run every example, compiled and in the REPL
make check                    # before a pull request: the three above, after the tree checks CI runs
```

Use `--preset debug` for an unoptimised build in `build-debug/`. Changes land
by pull request from an issue on the project board;
[CONTRIBUTING.md](CONTRIBUTING.md) has the steps.

The OpenUSD install is located automatically: `-DUSD_ROOT=...` wins, then the
`USD_ROOT` environment variable, then the locations listed in
[`cmake/OpenUSDExample.cmake`](cmake/OpenUSDExample.cmake)
(`/media/nadim/Data/OpenUSD`, `~/OpenUSDBuild`, `~/USD`, `/usr/local/USD`,
`/opt/USD`, among others). An install is recognised by its `pxrConfig.cmake`.

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
`exrc` looks in the current directory only. A few lines in `init.lua` that walk
up from the opened file with `vim.fs.find` and load the file through
`vim.secure.read` make it work from inside a lesson directory as well.

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
[`repl/TUTORIAL.md`](repl/TUTORIAL.md) walks through lesson 08 that way, and
`ctest -L repl` checks its transcripts against a run.

```sh
repl/usd-repl                                             # an empty session
repl/usd-repl nvidia_tutorials/08_xform/main.cpp          # run the lesson, keep the prompt
repl/usd-repl --batch nvidia_tutorials/08_xform/main.cpp  # run it and exit
repl/usd-repl --emit nvidia_tutorials/08_xform/main.cpp   # print the inputs it becomes
repl/usd-repl --serve repl.sock --cwd build/output/08_xform &   # a session that stays
repl/usd-repl --connect repl.sock FILE                    # run FILE, or stdin, in it
```

The REPL is [cpp-repl](https://github.com/farhatnadim/cpp_repl) with its
`usd` profile. `cmake --preset default` fetches cpp-repl at a pinned commit
into `build/_deps/cpp_repl-src/` when a clang-repl is installed (the pin is in
[`cmake/OpenUSDExample.cmake`](cmake/OpenUSDExample.cmake)), and
`repl/usd-repl` runs it with this repository's defaults:

- `-p usd`: OpenUSD loaded, `using namespace pxr;`, `show(x)` and `tree(stage)`
- `--build-dir build`: the compile flags come from
  `build/compile_commands.json` and `USD_ROOT` from `build/CMakeCache.txt`, so
  the REPL compiles exactly what the lessons compile
- `--cwd build/output/<target>`, with the lesson's `_assets/` staged as under
  ctest, when the first file is a lesson's source

Everything else on the command line goes to cpp-repl as it is: its options,
and compiler flags such as `-I DIR` or `-O2` (`repl/usd-repl --help`). It
needs the configured build, Python 3.11 or newer and a clang-repl. The lessons
are tested with `clang-repl-23` (`clang-23` and `clang-tools-23` from apt.llvm.org),
and cpp-repl needs 23 or newer;
`USD_REPL_CLANG_REPL` or `CPP_REPL_CLANG_REPL` names another. To run a
checkout of cpp-repl instead of the pinned commit, configure with
`-DFETCHCONTENT_SOURCE_DIR_CPP_REPL=/path/to/cpp_repl`. Plain `cpp-repl` works
in the repository too: [`cpp-repl.toml`](cpp-repl.toml) names the profile and
the build.

A lesson source becomes REPL inputs like this:
- includes and file-scope declarations and functions go in as written
- an `#if ... #endif` region goes in whole
- `main()`'s body follows one statement at a time, and a `#pragma` stays with
  the statement after it
- a `return` ends the program with its status, as it does compiled, and
  `return run();` still runs `run()`. At the prompt the session goes on; a
  `--batch` run stops there, and names the files after it that it leaves
  unrun

Every session starts from cpp-repl's prelude and
[`cpp_repl_usd.h`](https://github.com/farhatnadim/cpp_repl/blob/05feb4485b8e727144e8b77f9a335dce940f99f3/src/cpp_repl/profiles/usd/cpp_repl_usd.h):
the common USD headers, `using namespace pxr;`, `show(x)` and `tree(stage)`.

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
  or `try`, and a function head such as `int f()`, run after an empty line, so
  an `else`, a `catch` or the body can still follow. With prompt_toolkit, Esc
  then Enter runs a block as it stands. A pasted block can mix `#include`
  lines and code.
- **Crash recovery.** Ctrl-C stops a running input; `->` on a null stage, or any
  other crash, ends the backend. Either way the session is rebuilt from a journal
  of the inputs that worked, without the one that failed. A warm spare backend
  makes that take about a second. An exception nothing catches is named at the
  prompt, and the session goes on.
- **A name can be declared again.** `auto stage = ...` a second time replaces
  the first: the session is rebuilt without the old declaration, the inputs
  after it are replayed, and the prompt says so. An input that needs the old
  one keeps it.
- **Saving.**
  - `%save FILE` writes the journal as a `.repl` file. `repl/usd-repl FILE`,
    `%load` or `--batch` runs it again.
  - `%save-cpp FILE` writes it as a C++ program:
    - the prelude headers are included by name
    - directives and definitions sit at file scope
    - everything else goes in `main()`, and bare expressions become
      `show(...)` calls

    It builds like a lesson (`add_usd_example` adds the prelude's
    directories), or with
    `clang++ $(repl/usd-repl --cflags) FILE $(repl/usd-repl --libs)`, and the
    REPL runs it again.
- **Commands.** `%help` lists them: `%undo`, `%restart`, `%reset`,
  `%journal`, `%save FILE`, `%save-cpp FILE`, `%load FILE`, `%lib PATH`,
  `%time INPUT`, `%timeit INPUT`, `%flags` and `%quit`.

The history, and the inputs that go through a file, live under
`~/.local/state/cpp-repl/`.

### Editing and colour

With prompt_toolkit installed (`sudo apt install python3-prompt-toolkit`, or
`pip install prompt_toolkit`), input works as in IPython:
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

clang-repl runs the code. cpp-repl sits in front because typing straight into
clang-repl runs into the following. The table is clang-repl 18's, where this
began; cpp-repl's README has the same for 23 and 24, which cpp-repl runs:

| Typed into clang-repl 18 | What happens | What cpp-repl does |
| --- | --- | --- |
| A line ending in an open `{` | An endless error loop; Ctrl-C ends the session ([llvm#127569](https://github.com/llvm/llvm-project/pull/127569)) | Waits for the block to close |
| A top-level range-for | The loop variable keeps its first value, and a second loop fails to link ([llvm#89506](https://github.com/llvm/llvm-project/issues/89506)) | Runs top-level `for`, `while`, `do`, `if`, `switch` and `try` inside a function |
| A top-level `try`/`catch` | Link errors that break later inputs | The same function |
| Tab after `stage->` | clang-repl segfaults | Completes USD class and method names itself |
| `stage` | Prints nothing | Prints the value |
| A crash | Everything typed is lost, and Ubuntu files a crash report | Rebuilds the session, and no report is written |

On clang-repl 23 and 24, the open `{` still hangs and the range-for works.
Since LLVM 22, clang-repl prints values of built-in types only. cpp-repl's
README has the table for 23 and 24, and what C++ you can type at its prompt.

### Limits

- **Variables typed at the prompt are globals.** A name can therefore clash with
  a C library function: `int index = 3;` fails with "redefinition of 'index' as
  different kind of symbol". Lessons avoid this, because a helper function keeps
  its locals local.
- **Rebuilding replays the journal, file writes included;** so does declaring
  a name again, for the inputs after its old declaration.
- **An `#if` region, or a preprocessor line inside a block, needs its own
  lines,** so those inputs go through a file under `~/.local/state/cpp-repl/`.
- **The session compiles as C++20.** OpenUSD's headers do not build as C++23
  or later. clang 23 rejects an enumeration constant in the TBB 2020 headers
  this OpenUSD carries, so cpp-repl reads a patched copy of them.
- **The first start precompiles the OpenUSD headers,** about 6 s; every
  start after that takes about 1 s. After that a line takes 10–50 ms.

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
