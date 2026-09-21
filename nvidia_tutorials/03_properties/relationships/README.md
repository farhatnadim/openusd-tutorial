# Relationships

C++ port of NVIDIA's [Relationships](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/relationships.html) lesson.

## What it shows

- The second kind of property: a relationship points at other prims, where an attribute
  holds a value.
- `stage->DefinePrim(SdfPath("/World/Group"))` with no type — a typeless container prim.
- `UsdPrim::CreateRelationship(TfToken("members"), /*custom=*/true)` and
  `UsdRelationship::SetTargets` with a vector of `SdfPath`.
- Reading it back with `UsdPrim::GetRelationship` and `UsdRelationship::GetTargets`.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/03_relationships  # run from this directory: asset paths are relative
```

Writes `_assets/relationships_ex1.usda` and prints each resolved target path.
