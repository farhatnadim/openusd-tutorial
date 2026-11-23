#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/clipsAPI.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/vt/array.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// An empty layer for _assets/clips/<name>: the layer that is open or on disk,
// else a new one, then cleared. Thus a second run starts as the first did.
static SdfLayerRefPtr FreshLayer(const std::string& name)
{
  const std::string path = "_assets/clips/" + name;
  SdfLayerRefPtr layer = SdfLayer::FindOrOpen(path);
  if (!layer) {
    layer = SdfLayer::CreateNew(path);
  }
  layer->Clear();
  return layer;
}

// The value of a double attribute at each time, or that it has none.
static void Sample(const UsdAttribute& attr, const std::vector<double>& times)
{
  for (double time : times) {
    double value = 0.0;
    if (attr.Get(&value, UsdTimeCode(time))) {
      std::cout << "  " << attr.GetName() << " at " << time << ": " << value << "\n";
    } else {
      std::cout << "  " << attr.GetName() << " at " << time << ": no value\n";
    }
  }
}

// The times at which the attribute has samples, clips included.
static void PrintTimeSamples(const UsdAttribute& attr)
{
  std::vector<double> times;
  attr.GetTimeSamples(&times);
  std::cout << "  time samples:";
  if (times.empty()) {
    std::cout << " none";
  }
  for (double time : times) {
    std::cout << " " << time;
  }
  std::cout << "\n";
}

// A clip layer: /Clip with a double attribute size and its time samples.
static void MakeClip(const std::string& name,
                     const std::vector<std::pair<double, double>>& samples)
{
  SdfLayerRefPtr layer = FreshLayer(name);
  UsdStageRefPtr stage = UsdStage::Open(layer);
  UsdPrim clip = stage->DefinePrim(SdfPath("/Clip"));
  UsdAttribute size = clip.CreateAttribute(TfToken("size"), SdfValueTypeNames->Double);
  for (const auto& [time, value] : samples) {
    size.Set(value, UsdTimeCode(time));
  }
  layer->Save();
}

// The asset paths and the prim path of the clips of Example 1.
static void UseClipsAB(UsdClipsAPI& clips)
{
  clips.SetClipAssetPaths(VtArray<SdfAssetPath>{SdfAssetPath("./clip_a.usda"),
                                                SdfAssetPath("./clip_b.usda")});
  clips.SetClipPrimPath("/Clip");
}

// Example 1: a working clip set. Two clip layers hold time samples for
// /Clip.size; UsdClipsAPI on /World/Thing plays them one after the other.
static void WorkingClipSet()
{
  std::cout << "Example 1: a working clip set\n";
  MakeClip("clip_a.usda", {{0.0, 0.0}, {1.0, 1.0}});
  MakeClip("clip_b.usda", {{0.0, 10.0}, {1.0, 11.0}});

  SdfLayerRefPtr layer = FreshLayer("root.usda");
  UsdStageRefPtr stage = UsdStage::Open(layer);
  UsdPrim thing = UsdGeomXform::Define(stage, SdfPath("/World/Thing")).GetPrim();
  // Declared with no value: the clips give its values.
  UsdAttribute size = thing.CreateAttribute(TfToken("size"), SdfValueTypeNames->Double);

  UsdClipsAPI clips(thing);
  UseClipsAB(clips);
  // From stage time 0 clip 0 (clip_a) is active, from stage time 2 clip 1 (clip_b).
  clips.SetClipActive(VtVec2dArray{GfVec2d(0, 0), GfVec2d(2, 1)});
  // Stage time to clip time: stage times 2 and 3 read clip_b at 0 and 1.
  clips.SetClipTimes(VtVec2dArray{GfVec2d(0, 0), GfVec2d(1, 1), GfVec2d(2, 0), GfVec2d(3, 1)});
  layer->Save();

  PrintTimeSamples(size);
  Sample(size, {0, 0.5, 1, 2, 2.5, 3});
  PrintStage(stage);
}

// A cube under /World that the slide clip moves. Its own translate op has no
// value: it declares xformOp:translate, the attribute that the clip drives.
// A second translate op puts the cube on its row. Returns the driven attribute.
static UsdAttribute ClippedCube(const UsdStageRefPtr& stage, const std::string& name,
                                double row, const VtVec2dArray& times)
{
  UsdGeomCube cube = UsdGeomCube::Define(stage, SdfPath("/World").AppendChild(TfToken(name)));
  cube.GetSizeAttr().Set(1.0);
  UsdGeomXformOp slide = cube.AddTranslateOp();
  cube.AddTranslateOp(UsdGeomXformOp::PrecisionDouble, TfToken("row")).Set(GfVec3d(0, row, 0));

  UsdClipsAPI clips(cube.GetPrim());
  clips.SetClipAssetPaths(VtArray<SdfAssetPath>{SdfAssetPath("./slide.usda")});
  clips.SetClipPrimPath("/Clip");
  clips.SetClipActive(VtVec2dArray{GfVec2d(0, 0)});
  clips.SetClipTimes(times);
  return slide.GetAttr();
}

// Example 2: retiming a shared clip. One slide clip drives three cubes at full
// speed, half speed and delayed, through different clip times.
static void RetimingSharedClip()
{
  std::cout << "Example 2: retiming a shared clip\n";
  SdfLayerRefPtr slide_layer = FreshLayer("slide.usda");
  UsdStageRefPtr slide_stage = UsdStage::Open(slide_layer);
  UsdGeomXformOp slide = UsdGeomXform::Define(slide_stage, SdfPath("/Clip")).AddTranslateOp();
  slide.Set(GfVec3d(0, 0, 0), UsdTimeCode(0));
  slide.Set(GfVec3d(6, 0, 0), UsdTimeCode(24));
  slide_layer->Save();

  SdfLayerRefPtr layer = FreshLayer("offsets.usda");
  UsdStageRefPtr stage = UsdStage::Open(layer);
  stage->SetStartTimeCode(0);
  stage->SetEndTimeCode(48);
  UsdGeomXform world = UsdGeomXform::Define(stage, SdfPath("/World"));
  stage->SetDefaultPrim(world.GetPrim());

  std::vector<std::pair<std::string, UsdAttribute>> cubes = {
      {"FullSpeed", ClippedCube(stage, "FullSpeed", 0, VtVec2dArray{GfVec2d(0, 0), GfVec2d(24, 24)})},
      {"HalfSpeed", ClippedCube(stage, "HalfSpeed", 2, VtVec2dArray{GfVec2d(0, 0), GfVec2d(48, 24)})},
      {"Delayed", ClippedCube(stage, "Delayed", 4,
                              VtVec2dArray{GfVec2d(0, 0), GfVec2d(12, 0), GfVec2d(36, 24)})}};
  layer->Save();

  // The x position of each cube, frame by frame.
  std::ostringstream table;
  table << std::fixed << std::setprecision(2) << std::setw(5) << "frame";
  for (const auto& [name, attr] : cubes) {
    table << std::setw(11) << name;
  }
  table << "\n";
  for (int frame : {0, 12, 24, 36, 48}) {
    table << std::setw(5) << frame;
    for (const auto& [name, attr] : cubes) {
      GfVec3d translate(0.0);
      attr.Get(&translate, UsdTimeCode(frame));
      table << std::setw(11) << translate[0];
    }
    table << "\n";
  }
  std::cout << table.str() << "\n";
}

// root_<tag>.usda: /World/Thing with the clip set of Example 1 and no times,
// and the manifest when one is named. Returns the stage, because an attribute
// is valid only while its stage is alive.
static UsdStageRefPtr Build(const std::string& manifest, const std::string& tag)
{
  SdfLayerRefPtr layer = FreshLayer("root_" + tag + ".usda");
  UsdStageRefPtr stage = UsdStage::Open(layer);
  UsdPrim thing = UsdGeomXform::Define(stage, SdfPath("/World/Thing")).GetPrim();
  thing.CreateAttribute(TfToken("size"), SdfValueTypeNames->Double);

  UsdClipsAPI clips(thing);
  UseClipsAB(clips);
  clips.SetClipActive(VtVec2dArray{GfVec2d(0, 0), GfVec2d(2, 1)});
  if (!manifest.empty()) {
    clips.SetClipManifestAssetPath(SdfAssetPath("./" + manifest));
  }
  layer->Save();
  return stage;
}

// Example 3: selecting attributes with a manifest: none, one that declares
// size, and an empty one.
static void ManifestSelection()
{
  std::cout << "Example 3: selecting attributes with a manifest\n";
  SdfLayerRefPtr good = FreshLayer("manifest_good.usda");
  UsdStage::Open(good)->OverridePrim(SdfPath("/Clip"))
      .CreateAttribute(TfToken("size"), SdfValueTypeNames->Double);
  good->Save();
  FreshLayer("manifest_empty.usda")->Save();

  const std::vector<std::pair<std::string, std::string>> runs = {
      {"", "none"}, {"manifest_good.usda", "good"}, {"manifest_empty.usda", "empty"}};
  for (const auto& [manifest, tag] : runs) {
    UsdStageRefPtr stage = Build(manifest, tag);
    UsdAttribute size = stage->GetAttributeAtPath(SdfPath("/World/Thing.size"));
    std::cout << "manifest " << tag << ":\n";
    PrintTimeSamples(size);
    Sample(size, {0});
  }
  std::cout << "\n";
}

// root_omit_<missing>.usda: the clip set of Example 3 without the field that
// missing names ("nothing" omits no field).
static UsdStageRefPtr BuildOmitting(const std::string& missing)
{
  SdfLayerRefPtr layer = FreshLayer("root_omit_" + missing + ".usda");
  UsdStageRefPtr stage = UsdStage::Open(layer);
  UsdPrim thing = UsdGeomXform::Define(stage, SdfPath("/World/Thing")).GetPrim();
  thing.CreateAttribute(TfToken("size"), SdfValueTypeNames->Double);

  UsdClipsAPI clips(thing);
  if (missing != "assetPaths") {
    clips.SetClipAssetPaths(VtArray<SdfAssetPath>{SdfAssetPath("./clip_a.usda"),
                                                  SdfAssetPath("./clip_b.usda")});
  }
  if (missing != "primPath") {
    clips.SetClipPrimPath("/Clip");
  }
  if (missing != "active") {
    clips.SetClipActive(VtVec2dArray{GfVec2d(0, 0), GfVec2d(2, 1)});
  }
  layer->Save();
  return stage;
}

// Example 4: checking required fields: assetPaths, primPath and active, each
// omitted in turn.
static void RequiredFields()
{
  std::cout << "Example 4: the required fields\n";
  UsdStageRefPtr without_active;
  for (const std::string missing : {"nothing", "assetPaths", "primPath", "active"}) {
    UsdStageRefPtr stage = BuildOmitting(missing);
    UsdAttribute size = stage->GetAttributeAtPath(SdfPath("/World/Thing.size"));
    std::cout << "omitted " << missing << ":\n";
    PrintTimeSamples(size);
    Sample(size, {0});
    if (missing == "active") {
      without_active = stage;
    }
  }

  // With no clip value, a schema attribute gives its fallback: 2 for a Cube.
  UsdPrim thing = without_active->GetPrimAtPath(SdfPath("/World/Thing"));
  thing.SetTypeName(TfToken("Cube"));
  double size = 0.0;
  UsdGeomCube(thing).GetSizeAttr().Get(&size, UsdTimeCode(0));
  std::cout << "omitted active, typed Cube: size at 0: " << size << "\n";
}

int main()
{
  WorkingClipSet();
  RetimingSharedClip();
  ManifestSelection();
  RequiredFields();
  return 0;
}
