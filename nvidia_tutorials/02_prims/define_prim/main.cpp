#include <iostream>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/sdf/path.h>

int main()
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/sphere_prim.usda");
  pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere::Define(stage,pxr::SdfPath("/hello"));
  sphere.CreateRadiusAttr().Set(2.);
  stage->GetRootLayer()->Save();
}

