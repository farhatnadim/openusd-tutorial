# NVIDIA Learn OpenUSD — in C++

NVIDIA's [Learn OpenUSD](https://docs.nvidia.com/learn-openusd/latest/) curriculum is taught entirely in **Python**. These are C++
ports of those lessons, for anyone working against the OpenUSD C++ API instead of the
Python bindings.

One directory per NVIDIA lesson page, numbered in the curriculum's own reading order.
Where a lesson has more than one example, each gets a subdirectory named for the concept
it demonstrates. Every example is a self-contained CMake project.

## Lessons

**[Setting the Stage](https://docs.nvidia.com/learn-openusd/latest/stage-setting/index.html)**

| Directory | NVIDIA lesson | Concept |
| --- | --- | --- |
| [`01_stage`](01_stage) | [Stage](https://docs.nvidia.com/learn-openusd/latest/stage-setting/stage.html) | `UsdStage::CreateNew`, root layer, `SdfLayer` sublayers |
| [`02_prims/define_prim`](02_prims/define_prim) | [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) | `UsdGeomSphere::Define`, schema attributes |
| [`02_prims/prim_hierarchy`](02_prims/prim_hierarchy) | [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) | `Scope`/`Xform`/`Cube` via `SdfPath::AppendChild` |
| [`02_prims/traverse_hierarchy`](02_prims/traverse_hierarchy) | [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) | `UsdStage::Open`, `GetPrimAtPath`, `UsdPrim::GetChild` |
| [`03_properties/attributes`](03_properties/attributes) | [Attributes](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/attributes.html) | `UsdAttribute` get/set, `VtArray<GfVec3f>` |
| [`03_properties/relationships`](03_properties/relationships) | [Relationships](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/relationships.html) | `CreateRelationship`, `SetTargets`, `GetTargets` |
| [`03_properties/material_binding`](03_properties/material_binding) | [Relationships](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/relationships.html) | `UsdShadeMaterial`, `UsdShadeMaterialBindingAPI` |
| [`04_timecodes_timesamples`](04_timecodes_timesamples) | [Time Codes and Time Samples](https://docs.nvidia.com/learn-openusd/latest/stage-setting/timecodes-timesamples.html) | start/end time codes, `SetTranslate(v, timeCode)` |
| [`05_prim_property_paths`](05_prim_property_paths) | [Prim and Property Paths](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prim-property-paths.html) | `SdfPath::AppendProperty`, `GetPrimPath`, `GetNameToken` |

Not ported from this module: OpenUSD File Formats, OpenUSD Modules, Metadata.

**[Scene Description Blueprints](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/index.html)**

| Directory | NVIDIA lesson | Concept |
| --- | --- | --- |
| [`06_schemas`](06_schemas) | [Schemas](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/schemas.html) | not yet ported: a placeholder README |
| [`07_scopes`](07_scopes) | [Scope](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/scope.html) | `UsdGeomScope::Define`, prims grouped under scopes, `UsdPrim::SetActive` |
| [`08_xform`](08_xform) | [Xform](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/xform.html) | `UsdGeomXform::Define`, `UsdStage::ExportToString` |
| [`09_xform_common_api`](09_xform_common_api) | [XformCommonAPI](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/xformcommonapi.html) | `UsdGeomXformCommonAPI` translate, rotate and scale; one local translation under two parents |
| [`10_lights/distant_light`](10_lights/distant_light) | [Lights](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/lights.html) | `UsdLuxDistantLight::Define` under a `Lights` scope |
| [`10_lights/sphere_light`](10_lights/sphere_light) | [Lights](https://docs.nvidia.com/learn-openusd/latest/scene-description-blueprints/lights.html) | `UsdLuxSphereLight`, colour and intensity attributes, lights placed through `XformCommonAPI` |

**[Composition Basics](https://docs.nvidia.com/learn-openusd/latest/composition-basics/index.html)**

| Directory | NVIDIA lesson | Concept |
| --- | --- | --- |
| [`11_specifiers`](11_specifiers) | [Specifiers](https://docs.nvidia.com/learn-openusd/latest/composition-basics/specifiers.html) | `def`, `over` and `class`: `CreateClassPrim`, `OverridePrim`, `AddInherit`, a sublayer, `GetPrimStack` |
| [`12_references`](12_references) | [Referencing Basics](https://docs.nvidia.com/learn-openusd/latest/composition-basics/references.html) | `UsdReferences::AddReference` to a layer with a default prim, and to an external asset (two examples) |
| [`13_default_prim`](13_default_prim) | [Default Prim](https://docs.nvidia.com/learn-openusd/latest/composition-basics/default-prim.html) | `UsdStage::SetDefaultPrim` (in progress) |

Not ported from this module: Layers, Composition Arcs and Strength Ordering, Variant
Sets Basics.

**[Beyond the Basics](https://docs.nvidia.com/learn-openusd/latest/beyond-basics/index.html)**

| Directory | NVIDIA lesson | Concept |
| --- | --- | --- |
| [`14_primvars`](14_primvars) | [Primvars](https://docs.nvidia.com/learn-openusd/latest/beyond-basics/primvars.html) | `UsdGeomPrimvar` interpolation (constant, uniform, vertex) on a `UsdGeomMesh`; `UsdGeomPrimvarsAPI::CreatePrimvar` for rest-state and deformation primvars (two examples) |

Where a lesson's Python original is checked in, it sits beside the C++ as `main.py`.
Every lesson also runs in the REPL, which takes the `.cpp` as it is and leaves you at a
prompt with the lesson's variables in scope: `repl/usd-repl nvidia_tutorials/08_xform/main.cpp`
from the repository root (see [the REPL section](../README.md#repl)).

## Building

Every lesson here is built by the repository root build:

```sh
cd ..
cmake --preset default
cmake --build build
ctest --preset default        # runs every lesson, fails on a non-zero exit
```

Binaries land in `build/examples/`, one per lesson, named for the target in that
lesson's `CMakeLists.txt` (`01_stage`, `02_define_prim`, …).

### Building one lesson on its own

Each directory is also a complete CMake project, so a lesson can be copied out and
built by itself:

```sh
export USD_ROOT=/path/to/OpenUSD          # the directory holding pxrConfig.cmake

cd 03_properties/attributes
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Both paths share one definition of how an example is compiled,
[`../cmake/OpenUSDExample.cmake`](../cmake/OpenUSDExample.cmake); a lesson's own
`CMakeLists.txt` includes it only when that lesson is the top-level project.

### Assets

Every example addresses its data as `_assets/...`, relative to the working directory.
The build gives each example a private working directory at `build/output/<target>/`,
creates `_assets/` inside it, and copies in any `_assets/` checked in beside the source.
Examples therefore write their output into the build tree and never dirty the source.

Inspect what a lesson authored with `usdcat build/output/<target>/_assets/<file>.usda`,
or open it in `usdview`.

## Editor tooling

The repository root's `.nvim.lua` builds and runs the example you save in Neovim: see
[the root README](../README.md#editing-in-neovim).
