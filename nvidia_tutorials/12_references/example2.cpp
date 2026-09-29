#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/references.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// Example 2: referencing an external asset (cubebox_a02 is staged into _assets/).
static void ReferenceExternalAsset()
{
  std::string file_path("_assets/asset_ref.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
  UsdGeomXform world_xform  = UsdGeomXform::Define(stage,SdfPath("/World"));
  UsdGeomXform geometry_xform  = UsdGeomXform::Define(stage,world_xform.GetPath().AppendChild(TfToken("Geometry")));
  UsdGeomXform box_xform  = UsdGeomXform::Define(stage,geometry_xform.GetPath().AppendChild(TfToken("Box")));
  UsdPrim box_prim = box_xform.GetPrim();
  box_prim.GetReferences().AddReference("./cubebox_a02/cubebox_a02.usd");



  stage->Save();
  PrintStage(stage);
}

int main()
{
  ReferenceExternalAsset();
}
