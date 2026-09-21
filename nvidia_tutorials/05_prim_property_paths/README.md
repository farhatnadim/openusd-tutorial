# Prim and property paths

C++ port of NVIDIA's
[Prim and Property Paths](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prim-property-paths.html) lesson.

## What it shows

- A property path is a prim path plus a property name:
  `SdfPath::AppendProperty(TfToken("userProperties:tag"))`.
- Taking one apart again — `SdfPath::GetPrimPath()` to recover the owning prim (then
  `stage->GetPrimAtPath`), and `SdfPath::GetNameToken()` to recover the property name.
- `UsdPrim::GetAttribute(name).IsDefined()` as the existence check, then
  `UsdPrim::CreateAttribute(name, SdfValueTypeNames->String)` to author it.
- The same round-trip for a relationship: `UsdPrim::CreateRelationship` + `AddTarget`.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/05_prim_property_paths   # run from this directory: asset paths are relative
```

Writes `_assets/paths_property_authoring.usda`.

## Python original

`main.py` is the NVIDIA lesson this port was made from. It additionally uses the
stage-level lookups `GetPropertyAtPath`, `GetAttributeAtPath` and
`GetRelationshipAtPath`, which the C++ port does not.
