#include <iostream>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/imageable.h>

#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3f.h>       // Gf.Vec3f
#include <pxr/base/gf/vec3d.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>// Gf.Vec3d
#include <pxr/base/vt/array.h>
#include <vector>
int main()
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_asSets/relationships_ex3.usda");
  pxr::UsdGeomXform world_Xform = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
  std::vector<pxr::SdfPath> cubePaths;
  constexpr unsigned number_of_path {3};
  pxr::SdfPath cubePath  = world_Xform.GetPath().AppendChild(pxr::TfToken("Cube"));
  pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(stage,cubePath);
  pxr::UsdGeomImageable lowImageable(cube.GetPrim());
  stage->GetRootLayer()->Save();
}
