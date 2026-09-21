#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/sdf/path.h>
int main()
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/cube_prim.usda");
  pxr::UsdGeomScope scope = pxr::UsdGeomScope::Define(stage,pxr::SdfPath("/Geometry"));
  pxr::UsdGeomXform xform = pxr::UsdGeomXform::Define(stage,scope.GetPath().AppendChild(pxr::TfToken("GroupTransform")));
  pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(stage,xform.GetPath().AppendChild(pxr::TfToken("Box"))); 
  stage->GetRootLayer()->Save();
}

