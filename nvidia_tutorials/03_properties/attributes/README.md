# Attributes

C++ port of NVIDIA's [Attributes](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/attributes.html) lesson.

## What it shows

- Schema-provided attribute accessors: `UsdGeomSphere::CreateRadiusAttr`,
  `UsdGeomCube::GetSizeAttr`, `GetDisplayColorAttr`, `GetExtentAttr`.
- The generic `UsdAttribute` round-trip — `Get(&value)`, modify, `Set(value)` — which
  is how you read and write any attribute without knowing its schema.
- Array-valued attributes via `VtArray<GfVec3f>` for `displayColor`.
- `UsdGeomXformCommonAPI::SetTranslate` for placement.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/03_attributes     # run from this directory: asset paths are relative
```

Writes `_assets/sphere_prim.usda`.
