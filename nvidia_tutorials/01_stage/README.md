# Stage

C++ port of NVIDIA's [Stage](https://docs.nvidia.com/learn-openusd/latest/stage-setting/stage.html) lesson.

## What it shows

- `UsdStage::CreateNew` to author a new stage, and `stage->GetRootLayer()` to reach its
  root `SdfLayerHandle`.
- `stage->DefinePrim(SdfPath("/World"), TfToken("Xform"))` — defining a typed prim by
  type name rather than through a schema class.
- `SdfLayer::CreateNew` for a second layer, registered on the stage by pushing its
  relative path onto `rootLayer->GetSubLayerPaths()`.
- `SdfLayer::ExportToString` to dump the composed layer text to stdout.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/01_stage          # run from this directory: asset paths are relative
```

Writes `_assets/root_layer_example_cpp.usda` and the sublayer `_assets/extra_layer_cpp.usdc`.

## Python original

`main.py` is the NVIDIA lesson this port was made from; it writes
`_assets/root_layer_example.usda` and `_assets/extra_layer.usdc`, both checked in here.
