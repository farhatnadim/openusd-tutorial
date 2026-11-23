# Lesson 13_default_prim is finished (issue #4)

```mermaid
flowchart LR
    A["UsdStage::CreateNew<br/>_assets/default_prim.usda"] --> B["DefinePrim /hello<br/>DefinePrim /hello/world"]
    B --> C["GetPrimAtPath /hello"]
    C --> D["SetDefaultPrim"]
    D --> E["Save, print the root layer"]
    E --> F["layer metadata:<br/>defaultPrim = #quot;hello#quot;"]
```

The boxes A and E are in `main.cpp` now. The boxes B, C and D are the work.

## Problem

`nvidia_tutorials/13_default_prim/main.cpp` creates the stage, saves it and
prints it. The body of the example is empty. The lesson index marks the
lesson "in progress".

The NVIDIA page
([Default Prim](https://docs.nvidia.com/learn-openusd/latest/composition-basics/default-prim.html))
has one example: "Example 1: Setting a Default Prim".

## Requirements

- **R1.** Lesson 13 has a complete C++ port of Example 1 of the NVIDIA
  Default Prim page.
- **R2.** The output of the program shows the default prim in the metadata
  of the layer.
- **R3.** The lesson runs as a compiled program and at the interactive
  prompt, and the full check includes it.
- **R4.** The lesson index does not mark the lesson "in progress".

## Specification

- **S1 (R1).** The function `SetDefaultPrim()` in `main.cpp` gets three
  steps between `UsdStage::CreateNew` and `stage->Save()`:
  1. It defines the prim `/hello` and the prim `/hello/world` with
     `UsdStage::DefinePrim(SdfPath)`. The prims have no type name.
  2. It gets the prim `/hello` with `UsdStage::GetPrimAtPath(SdfPath)`.
  3. It gives that prim to `UsdStage::SetDefaultPrim(UsdPrim)`.
- **S2 (R2).** The program prints the root layer. The expected output
  (checked at the prompt on 2026-11-23, OpenUSD 0.26.11):

  ```usda
  #usda 1.0
  (
      defaultPrim = "hello"
  )

  def "hello"
  {
      def "world"
      {
      }
  }
  ```

- **S3 (R3).** `CMakeLists.txt` does not change: the target is
  `13_default_prim` with `LIBS usd`. The includes of `main.cpp` are
  sufficient.
- **S4 (R4).** The row of the lesson in `nvidia_tutorials/README.md` becomes:
  `UsdStage::SetDefaultPrim`, the `defaultPrim` layer metadata. The words
  "(in progress)" go.
- **S5.** The untitled snippet under "Working With Python" (an in-memory
  stage, `/World`, and `GetDefaultPrim()`) is not ported. The lesson ports
  the section "Examples", as the other lessons do.

## Who writes what

The human writes the three steps of S1, at the prompt first and then in
`main.cpp`. The agent builds, runs, shows the output, and changes the index
row of S4 on the instruction of the human.

## State on 2026-11-23

The human gave the instruction to finish the work. Thus the agent wrote
`main.cpp` as the specification describes, built it and ran it. The output
is the expected output of the specification. `make check` passed
41 of 41 tests. The lesson landed in `bb08862` (pull request #18).

## Verification

| Requirement | Check |
| --- | --- |
| R1, R2 | `cmake --build build --target 13_default_prim`, then the program prints the output of S2 |
| R3 | `ctest --test-dir build -R 13_default_prim` passes two tests: `13_default_prim` and `repl_13_default_prim` |
| R4 | `make check-tree` passes; the index row has no "(in progress)" |

The compiled checks need issue #11.

## Facts checked at the prompt

- `stage->GetDefaultPrim().GetPath()` gives `/hello`.
  `stage->GetRootLayer()->GetDefaultPrim()` gives the token `hello`.
- The NVIDIA page says that the default prim must be a top-level prim.
  OpenUSD 0.26.11 also accepts a prim that is not top-level:
  `SetDefaultPrim` with `/hello/world` writes
  `defaultPrim = "/hello/world"`, and `GetDefaultPrim()` gives that prim.
