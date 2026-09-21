# Prim hierarchy

C++ port of NVIDIA's [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) lesson — the hierarchy half.

## What it shows

- Building a parent/child scenegraph: `UsdGeomScope` at `/Geometry`, then a
  `UsdGeomXform` beneath it, then a `UsdGeomCube` beneath that.
- `SdfPath::AppendChild(TfToken(...))` to derive each child path from its parent's,
  rather than writing full path strings.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/02_prim_hierarchy # run from this directory: asset paths are relative
```

Writes `_assets/cube_prim.usda` — the stage `../traverse_hierarchy` reads back.
