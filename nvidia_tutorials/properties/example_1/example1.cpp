#include <iostream>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3f.h>       // Gf.Vec3f
#include <pxr/base/gf/vec3d.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>// Gf.Vec3d
#include <pxr/base/vt/array.h>
int main()
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/sphere_prim.usda");
  pxr::UsdGeomXform world_Xform = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
  pxr::SdfPath spherePath  = world_Xform.GetPath().AppendChild(pxr::TfToken("Sphere"));
  pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere::Define(stage,spherePath);
  sphere.CreateRadiusAttr().Set(2.);
  pxr::SdfPath cubePath  = world_Xform.GetPath().AppendChild(pxr::TfToken("Cube"));
  pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(stage,cubePath);
  pxr::UsdGeomXformCommonAPI(cube).SetTranslate(pxr::GfVec3d(5,0,0));

  double size {0.0};
  pxr::UsdAttribute sizeAttr = cube.GetSizeAttr();
  if (sizeAttr.Get(&size)) 
  {
    sizeAttr.Set(size*2.0);
  }
  
  pxr::UsdAttribute displayColor = cube.GetDisplayColorAttr();
  displayColor.Set(pxr::VtArray<pxr::GfVec3f>{pxr::GfVec3f(0,1.0,0)});
  pxr::UsdAttribute extentAttr = cube.GetExtentAttr();
  auto extent_size {0.0};
  extentAttr.Get(&extent_size);
  extentAttr.Set(extent_size);

  stage->GetRootLayer()->Save();
}

