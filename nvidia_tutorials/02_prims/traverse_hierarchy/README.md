# Traverse a hierarchy

C++ port of NVIDIA's [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) lesson — the read-back half.

> **Run [`../prim_hierarchy`](../prim_hierarchy) first.** This example opens the stage that
> one authors; it does not create its own.

## What it shows

- `UsdStage::Open` on an existing layer instead of `CreateNew`.
- `stage->GetPrimAtPath(SdfPath("/Geometry"))` and `UsdPrim::GetChild(TfToken(...))`
  to walk down the scenegraph.
- `UsdPrim`'s conversion to `bool` as the existence check for a prim that may not
  be there — the idiomatic way to test the result of a lookup.

## Build and run

```sh
cd ../prim_hierarchy && cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT" \
  && cmake --build build && ./build/02_prim_hierarchy && cd -
cp ../prim_hierarchy/_assets/cube_prim.usda _assets/

cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/02_traverse_hierarchy   # run from this directory: asset paths are relative
```

Reads `_assets/cube_prim.usda` and prints whether the child prim exists.
`_assets/prim_hierarchy.usda` is a hand-written equivalent kept for reference.
