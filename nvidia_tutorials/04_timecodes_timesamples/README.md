# Time codes and time samples

C++ port of NVIDIA's
[Time Codes and Time Samples](https://docs.nvidia.com/learn-openusd/latest/stage-setting/timecodes-timesamples.html) lesson.

## What it shows

- `stage->SetStartTimeCode(1)` / `SetEndTimeCode(60)` to declare the stage's playback range.
- Authoring **time samples** by passing a time code to the setter:
  `UsdGeomXformCommonAPI::SetTranslate(GfVec3d(...), <timeCode>)` at frames 1, 30, 45, 50
  and 60 — the same call that authors a default value, with one extra argument.
- `UsdGeomXformOp::GetAttr().Clear()` to drop an already-authored op before re-authoring it.
- `UsdStage::Export` alongside `GetRootLayer()->Save()`.

## Build and run

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$USD_ROOT"
cmake --build build
./build/04_timecodes_timesamples   # run from this directory: asset paths are relative
```

Writes `_assets/timecode_sample.usda` and exports `_assets/timecode_ex1.usda`.
