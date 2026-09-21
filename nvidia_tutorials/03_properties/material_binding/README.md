# Material binding

C++ port of NVIDIA's [Relationships](https://docs.nvidia.com/learn-openusd/latest/stage-setting/properties/relationships.html) lesson —
material binding as the relationship you meet first in practice.

## What it shows

- `UsdShadeMaterial::Define` and `UsdShadeShader::Define`, with
  `CreateIdAttr(VtValue("UsdPreviewSurface"))`.
- `CreateInput(TfToken("diffuseColor"), SdfValueTypeNames->Color3f)` and
  `CreateSurfaceOutput().ConnectToSource(shader.ConnectableAPI(), TfToken("surface"))`
  to wire a shading network.
- `UsdShadeMaterialBindingAPI::Apply(prim).Bind(material)` — an applied API schema
  authoring the binding relationship — then rebinding one prim to a second material.
- Verifying with `UsdShadeMaterialBindingAPI(prim).GetDirectBinding().GetMaterial()`.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/03_material_binding   # run from this directory: asset paths are relative
```

Writes `_assets/relationships_ex3.usda` and prints each cube's resolved material.

## Python original

`main.py` is the NVIDIA lesson this port was made from. It names the cubes
`Cube_1/2/3` where the C++ uses `Cube_0/1/2`, and uses `SdfPath.AppendPath` where the
C++ uses `AppendChild`.
