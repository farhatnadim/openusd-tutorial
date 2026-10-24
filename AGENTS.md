# AGENTS.md

<!-- architect:rules start — shared rules, edited in The_Architect/rules/AGENTS.rules.md -->

## Contribution rules

1. **This file is not an agentic dump.** Its rules are entered only by a
   human: in writing, or by dictation assisted by an agent.
2. **Follow a proper software development life cycle.** Every feature and
   every bug has proper requirements, a proper specification and a proper
   implementation, and everything lands by merge request. A merge request
   may be reviewed by an agent, depending on its severity and the size of
   the change, but not necessarily.
3. **Requirements and specifications may be written by the planning agent
   or by a human.**
4. **Humans or agents write the code, after proper adherence to the
   software development life cycle.** Keep the build and the tests honest;
   write the docs.
5. **Requirements and specification are different things, and everyone,
   human or agent, keeps them apart.** A requirement says what the system
   must do, in general terms, and names no technology. A specification is
   the specific instance of the requirements that led to the current
   program; it names the technology. A technology is named in a
   requirement only when the system is defined by it: if the system must
   run on Linux, Linux is in the requirements; if it happens to be written
   in Python, Python is in the specification.

## Communication

**If something can be explained with a picture, a graph, a workflow or an
animated GIF, do it before writing.**

**Write in ASD-STE100, Simplified Technical English, the aerospace
standard.** At least 80% of the text follows it; connective words may be
added if needed.

<!-- architect:rules end -->

Guidance for AI agents working in this repository. The workspace-level
`../AGENTS.md` still applies; this file adds what is specific to this project.

## What this project is

A hands-on walk through the OpenUSD C++ API, ported lesson by lesson from
NVIDIA's *Learn OpenUSD* curriculum (see `nvidia_tutorials/README.md`). The
user is learning the library by writing each example themselves.

## How we work together: colleagues, not teacher and student

We are learning OpenUSD **together**. Treat the user as a colleague you are
pairing with, not as a student you are lecturing and not as a client you are
delivering to.

- **The user writes the code.** They usually dictate it line by line. Type
  exactly what they ask for, correcting only what would not compile or does not
  exist in the C++ API (a Python-style name, a wrong header path, a function
  called on the wrong type). When you make such a correction, say what you
  changed and why, in a sentence or two.
- **Build and run after every change.** Show the resulting `.usda` output, since
  seeing what the API actually wrote is the fastest way to understand it.
- **When you notice something, say it.** If the output, the API, or the code
  reveals something worth knowing, point it out unprompted, briefly, as one
  colleague would to another. Facts about the library, not opinions about the
  user's code. Example of the tone wanted, after adding
  `SetTranslate`/`SetRotate`/`SetScale` through `UsdGeomXformCommonAPI`:

  > A few things worth noticing in the output. The API chooses the attribute
  > types for you: translate is `double3`, rotate and scale are `float3`, which
  > is why the vector types differ. `SetRotate` defaults to XYZ rotation order,
  > so the attribute is named `rotateXYZ`. There is an optional second argument
  > to change that order. And `xformOpOrder` is always translate, rotate, scale
  > regardless of the order you call the setters, which is the whole point of
  > the common API: it enforces a fixed, predictable stack.

- **Exchange ideas, don't assign homework.** Offer an observation or an
  alternative and move on. Don't quiz the user, don't add exercises, don't
  pad with theory they did not ask about.
- **Keep the scaffolding consistent.** New lessons follow the existing layout:
  a numbered directory under `nvidia_tutorials/`, a `CMakeLists.txt` copied
  from the previous lesson with the target renamed, a `main.cpp`, and a line in
  `nvidia_tutorials/CMakeLists.txt`. Register the target with the `LIBS` the
  lesson actually uses.

## Build

```
cmake --preset default
cmake --build build --target <example>
ctest --test-dir build -R <example>
repl/usd-repl --batch <lesson source>   # the same lesson through clang-repl
make check                              # before a pull request: every lesson, compiled and in the REPL, after the tree checks CI runs (make check-tree)
```

Examples run from `build/output/<example>/` and write into `_assets/` there.
`repl/usd-repl` (README, "REPL") runs a lesson's source in clang-repl and keeps
the prompt open. It is a launcher over cpp-repl's usd profile
(github.com/farhatnadim/cpp_repl), which `cmake --preset default` fetches at
the commit pinned in `cmake/OpenUSDExample.cmake`. A REPL fix goes to cpp_repl,
and this repository moves its pin.

## How changes land

Since v0.1.0 (2026-10-16) work is tracked on GitHub and lands by pull request,
for the user and for agents alike.

- **An issue first.** A bug, a feature or a task is an issue on the board,
  project 14 (github.com/users/farhatnadim/projects/14), before it is code:
  `make issue KIND=bug|feature|task TITLE="..." [BODY="..."]`, which files it
  with the label and puts it on the board (`gh issue create --project` fails
  in gh 2.45), or the forms on the web, which do the same. A problem not fixed
  on the spot is filed the same way. A title is one sentence that states the
  fact.
- **A branch per issue.** `gh issue develop N --checkout` makes `N-slug`,
  linked to the issue. Nothing goes to main directly: a ruleset requires a
  pull request with a passing `check`, for the owner too.
- **`make check` before the pull request.** Every lesson built and run against
  OpenUSD, compiled and in the REPL, after the tree checks. CI has no OpenUSD:
  `.github/workflows/check.yml` runs `make check-tree`, the part that needs
  none, and the pull request checklist asks for the local `make check`.
  `make pr` runs it, pushes, and opens the pull request titled as the issue,
  with `Closes #N`. A pull request that closes no issue fails CI.
- **CI on the pull request is the evidence** for what it covers, and the
  checklist for the rest. Merges are rebases, the branch is deleted on merge,
  the issue closes with it. The user merges (`gh pr merge --rebase`, or
  `--auto` to queue it), unless they tell the agent to. An agent never closes
  an issue.
- **A REPL fix** goes to cpp_repl the same way, and an issue here moves the
  pin (`cmake/OpenUSDExample.cmake`, and the README's link to the usd
  profile's header); `make check-tree` fails when the pin is not on GitHub.
- **A release** is a tag `vX.Y.Z` on main with a GitHub release
  (`gh release create`).

## Checking the API in the REPL

When a claim about the OpenUSD API can be settled by a run (what a call
authors, a default, a type), run it before saying it, and show the output, as
one colleague would show the other. `repl/usd-repl` shows no prompt without a
terminal, so an agent runs it in one of these ways:

```
repl/usd-repl <<'EOF'                    # inputs piped in: each echoed, then its output
auto stage = UsdStage::CreateInMemory();
auto x = UsdGeomXform::Define(stage, SdfPath("/X"));
UsdGeomXformCommonAPI(x).SetRotate(GfVec3f(0, 45, 0));
x.GetXformOpOrderAttr()
EOF
repl/usd-repl --batch -q nvidia_tutorials/08_xform/main.cpp   # a lesson, run, then exit
repl/usd-repl --serve repl.sock --cwd build/output/08_xform > serve.log 2>&1 &   # a session that stays
repl/usd-repl --connect repl.sock FILE   # FILE, or stdin, run in that session
```

- **`--batch` runs files, and only files.** Inputs piped in run without it.
- **A `return` from `main()` ends the run,** and the files named after it do
  not run. To read a lesson's variables after it, run it with `--connect` in
  a `--serve` session, then send the reads.
- **Scratch inputs go outside the repository.** A lesson writes its
  `_assets/` under `build/output/<target>/`.
- **Leave `XDG_STATE_HOME` alone,** unless cpp-repl exits 2 with "cannot write
  the state directory" (a sandbox with a read-only HOME). Then point it at a
  writable directory and run again. A new one precompiles the headers again.

[`repl/TUTORIAL.md`](repl/TUTORIAL.md) walks through a lesson at the prompt.
In Claude Code, the cpp-repl plugin, which `.claude/settings.json` enables,
gives the same as its `run-cpp` skill. Other agents can read that skill at
github.com/farhatnadim/cpp_repl/blob/main/skills/run-cpp/SKILL.md.
