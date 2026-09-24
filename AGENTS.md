# AGENTS.md

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
```

Examples run from `build/output/<example>/` and write into `_assets/` there.
