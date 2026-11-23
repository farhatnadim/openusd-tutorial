# The lessons build against an OpenUSD install made with Python 3.14 (issue #11)

```mermaid
flowchart LR
    OS["Ubuntu 26.04<br/>Python 3.14 only"] -. removed .-> DEV["/usr/include/python3.12<br/>libpython3.12.so"]
    OLD["/media/nadim/Data/OpenUSD<br/>made with Python 3.12"] -->|"pxrConfig.cmake and<br/>pxrTargets.cmake name"| DEV
    DEV -->|missing| FAIL["cmake --preset default<br/>stops"]
    UV["uv: CPython 3.14<br/>in the prefix"] --> NEW
    SRC["OpenUSD checkout<br/>6792941c5, version 0.26.11"] -->|"build_usd.py<br/>with Python 3.14"| NEW["/media/nadim/Data/OpenUSD-py314-uv"]
    NEW -->|USD_ROOT| OK["cmake --preset default<br/>make check"]
```

## Problem

The operating system upgrade to Ubuntu 26.04 removed the Python 3.12
development files. The system Python is 3.14.4. `apt` has no
`python3.12-dev`.

The OpenUSD install at `/media/nadim/Data/OpenUSD` is version 0.26.11
(branch `dev`, commit `6792941c5`). It was made on 2026-08-22 with GCC 13.3
and Python 3.12.3. Its CMake package names Python 3.12 in two places:

- `pxrConfig.cmake` calls
  `find_dependency(Python3 "3.12" EXACT COMPONENTS Development)`. The
  configure step stops here.
- `pxrTargets.cmake` puts `/usr/include/python3.12` in the include
  directories of the imported targets.

A test on 2026-11-23 gave the configure step the Python 3.12 development
files from a different location. The find step passed. The generate step
stopped: `Imported target "usd" includes non-existent path
"/usr/include/python3.12"`. Thus a different Python 3.12 does not repair the
install. The install must be made again.

Effects:

- No lesson builds. `make check` and `make pr` fail.
- `usdview` from the install stops with a segmentation fault. Its
  interpreter is now Python 3.14, and its modules are made for Python 3.12.
- The interactive prompt and `make check-tree` operate. The runtime library
  `libpython3.12.so.1.0` is still on the machine.

OpenUSD supports Python 3.14 from release 26.05 (its `CHANGELOG.md`).

## Requirements

- **R1.** The command that configures the build completes with no error on
  the development machine.
- **R2.** Every lesson builds and runs, as a compiled program and at the
  interactive prompt.
- **R3.** The OpenUSD tools that the editor workflow starts (the viewer, the
  checker, the text dump) operate.
- **R4.** The interactive prompt stays in operation while the new install is
  made. The old install stays on the disk until the new install satisfies
  R1, R2 and R3.
- **R5.** The OpenUSD version does not change. Only the Python version
  changes.
- **R6.** The repository records how the OpenUSD install is made.

## Specification

- **S1 (R5).** The source is the checkout `/media/nadim/Data/Source/OpenUSD`
  at commit `6792941c5d5b42778e6875c5ed5a3279e6936981`. This is the commit
  of the old install.
- **S2 (R1, R3).** The Python is a CPython 3.14 that `uv` installs in the
  prefix, at `<prefix>/python` (decision D1). An upgrade of the operating
  system does not replace it, and `uv` does not remove it when it cleans its
  own directories. `uv` 0.9.12 gives CPython 3.14.0, with
  `include/python3.14` and `lib/libpython3.14.so`. A virtual environment at
  `<prefix>/venv` holds PySide6, PyOpenGL and Jinja2:

  ```sh
  PREFIX=/media/nadim/Data/OpenUSD-py314-uv
  uv python install 3.14 --install-dir $PREFIX/python --no-bin
  uv venv --python $PREFIX/python/cpython-3.14.0-linux-x86_64-gnu/bin/python3.14 $PREFIX/venv
  uv pip install --python $PREFIX/venv/bin/python PySide6 PyOpenGL jinja2
  ```

  `libpython3.14.so` is not in a directory of the system loader. OpenUSD
  sets `INSTALL_RPATH_USE_LINK_PATH` on its libraries
  (`cmake/macros/Private.cmake`), thus their run path has
  `<prefix>/python/cpython-3.14.0-linux-x86_64-gnu/lib`.

  `build_usd.py` runs with the interpreter of this environment. Then
  `pxrConfig.cmake` and the scripts in `bin/` name this interpreter.
- **S3 (R4).** The prefix of the new install is
  `/media/nadim/Data/OpenUSD-py314-uv`. The old install stays at
  `/media/nadim/Data/OpenUSD`.
- **S4 (R5).** The build command uses the default components of
  `build_usd.py`, as the old install did: imaging, usdview, MaterialX,
  OpenSubdiv, TBB, the tools, the examples and the tutorials.

  ```sh
  PREFIX=/media/nadim/Data/OpenUSD-py314-uv
  PATH=$PREFIX/venv/bin:$PATH $PREFIX/venv/bin/python \
      /media/nadim/Data/Source/OpenUSD/build_scripts/build_usd.py -j 20 $PREFIX
  ```

- **S5 (R1).** `cmake/OpenUSDExample.cmake` puts
  `/media/nadim/Data/OpenUSD-py314-uv` first in its list of candidate
  locations. The old location stays in the list after it. The root
  `README.md` names the new location in "Build".
- **S6 (R1, R2).** The build directory gets the new location:
  `cmake --preset default -DUSD_ROOT=/media/nadim/Data/OpenUSD-py314-uv`. The
  cache of `build/` holds the old `USD_ROOT`, thus the `-D` is necessary one
  time. Then `make check` passes.
- **S7 (R3).** These commands complete with exit code 0, with
  `PYTHONPATH=$PREFIX/lib/python3.14/site-packages` and `$PREFIX/bin` in
  `PATH`: `usdview --help`, `usdchecker --help`, and `usdcat` on a file
  that a lesson wrote.
- **S8 (R6).** The root `README.md` gets a section "Building OpenUSD" with
  the commands of S2 and S4.
- **S9 (R4).** The human deletes the old install. An agent does not delete
  it.
- **S10 (R1, R3).** `-DUSD_ROOT` and then the environment variable
  `USD_ROOT` come before the candidate list. The shell of the human sets
  `USD_ROOT`, `PATH` and `PYTHONPATH` in `~/.bashrc`, and the three named
  the old install. They name the new install:
  `USD_ROOT=/media/nadim/Data/OpenUSD-py314-uv`, `$PREFIX/bin` in `PATH`,
  and `$PREFIX/lib/python3.14/site-packages` in `PYTHONPATH`.
- **S11 (R1, R2), issue #22.** `find_package(pxr)` takes a cached
  `pxr_DIR` first. Thus a build directory that was configured again with
  `-DUSD_ROOT` kept the headers and the libraries of its first install, and
  printed the new location. `cmake/OpenUSDExample.cmake` sets the cache
  entry `pxr_DIR` to `USD_ROOT`, with `FORCE`, before `find_package(pxr)`.
  `pxrConfig.cmake` is at the root of an install, thus `pxr_DIR` and
  `USD_ROOT` are the same directory.

## Verification

| Requirement | Check |
| --- | --- |
| R1 | `cmake --preset default` prints `OpenUSD 2611 from /media/nadim/Data/OpenUSD-py314-uv` and no error |
| R2 | `make check` passes: each lesson has a compiled test and a `repl_` test |
| R3 | The three commands of S7 |
| R4 | `repl/usd-repl` operates during the build; `/media/nadim/Data/OpenUSD` has no changed file |
| R5 | `PXR_VERSION` in `include/pxr/pxr.h` is 2611 in the two installs |
| R6 | The section "Building OpenUSD" is in `README.md` |

## State on 2026-11-23

- The first build used the system Python, at
  `/media/nadim/Data/OpenUSD-py314`. The human chose the Python of `uv`
  (D1), thus S2 to S4 were done again at
  `/media/nadim/Data/OpenUSD-py314-uv`: `build_usd.py` completed with exit
  code 0 in 14 min 22 s (`-j 20`, GCC 15.2, CMake 4.2), 2.5 GB. No file of
  the old install changed. The first build stays on the disk until the
  human deletes it with the old install (S9).
- S7 is checked on the new install: the three commands complete with exit
  code 0, and `from pxr import Usd` in the venv gives 0.26.11.
- S5 and S8 are in the pull request of issue #11.
- R1 and R2: in a checkout of that branch, with no `USD_ROOT`,
  `cmake --preset default` prints
  `Auto-detected USD_ROOT: /media/nadim/Data/OpenUSD-py314-uv` and
  `OpenUSD 2611 from /media/nadim/Data/OpenUSD-py314-uv`. `make check`
  passes 43 of 43 tests.
- S10: `USD_ROOT` in `~/.bashrc` names the new install. The agent was not
  permitted to change `PATH` and `PYTHONPATH` in `~/.bashrc`; the human
  changes them.
- S6 follows the merge.

## Notes

- The lessons were compiled against Python 3.12 headers. The OpenUSD
  headers include `Python.h`, thus every lesson compiles again after S6.
- The Python modules of the new install are in
  `lib/python3.14/site-packages`. The old install has them in
  `lib/python3.12/site-packages`.
- `uv` 0.9.12 knows CPython 3.14.0 as the newest 3.14. A newer `uv` gives a
  newer patch release in a new directory; the install keeps 3.14.0.

## Decisions

- **D1.** The human chose a CPython 3.14 that `uv` manages (2026-11-23).
  It is in the prefix (S2), thus an upgrade of the operating system does not
  replace it.
- **D2.** S5 changes the candidate list, and S10 changes the environment of
  the human, because the environment comes first.
