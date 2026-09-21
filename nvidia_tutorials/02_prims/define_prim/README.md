# Define a prim

C++ port of NVIDIA's [Prims](https://docs.nvidia.com/learn-openusd/latest/stage-setting/prims.html) lesson.

## What it shows

- `UsdGeomSphere::Define(stage, SdfPath("/hello"))` — defining a prim through its
  schema class, the usual alternative to `UsdStage::DefinePrim`.
- `sphere.CreateRadiusAttr().Set(2.)` to author the schema's own attribute.
- `stage->GetRootLayer()->Save()`.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/02_define_prim    # run from this directory: asset paths are relative
```

Writes `_assets/sphere_prim.usda`.

## Python original

`main.py` is the NVIDIA lesson this port was made from.
