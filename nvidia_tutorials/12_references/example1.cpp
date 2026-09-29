#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/references.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3d.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// Example 1: a cube in its own layer, referenced into a shapes layer.
static void AddReference()
{
  std::string file_path("_assets/cube.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
  UsdGeomCube cube = UsdGeomCube::Define(stage,SdfPath("/Cube"));
  stage->SetDefaultPrim(cube.GetPrim());
  stage->Save();
  PrintStage(stage);

  std::string second_file_path("_assets/shapes.usda");
  stage = UsdStage::CreateNew(second_file_path);
  UsdGeomXform world = UsdGeomXform::Define(stage,SdfPath("/World"));
  UsdGeomSphere sphere = UsdGeomSphere::Define(stage, world.GetPath().AppendPath(SdfPath("Sphere")));

  UsdPrim reference_prim = stage->DefinePrim(world.GetPath().AppendPath(SdfPath(("Cube_ref"))));

  reference_prim.GetReferences().AddReference(("./cube.usda"));
  UsdGeomXformCommonAPI(reference_prim).SetTranslate(GfVec3d(5,0,0));


  stage->Save();
  PrintStage(stage);
}

int main()
{
  AddReference();
}
