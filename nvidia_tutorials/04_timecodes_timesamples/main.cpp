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
#include <string>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>

int main()
{
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/timecode_sample.usda");
  stage->SetStartTimeCode(1);
  stage->SetEndTimeCode(60);
  stage->Export("_assets/timecode_ex1.usda",false);

  pxr::UsdGeomXform world = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
  pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere::Define(stage,world.GetPath().AppendChild(pxr::TfToken("Sphere")));
  pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(stage,world.GetPath().AppendChild(pxr::TfToken("Cube")));
  
  pxr::VtArray<pxr::GfVec3f> vecColor{pxr::GfVec3f(0.0, 0.0f, 1.0f)};
  cube.GetDisplayColorAttr().Set(vecColor);
  auto cube_xform_api = pxr::UsdGeomXformCommonAPI(cube);
  cube_xform_api.SetScale(pxr::GfVec3f(5,5,0.1));
  cube_xform_api.SetTranslate(pxr::GfVec3d(0,0,-2));

  if ( auto translate_attr = sphere.GetTranslateOp().GetAttr())
      translate_attr.Clear();
  
  
  auto sphere_xform_api = pxr::UsdGeomXformCommonAPI(sphere);
    
  sphere_xform_api.SetTranslate(pxr::GfVec3d(0,5.50,0),1);
  sphere_xform_api.SetTranslate(pxr::GfVec3d(0,-4.50,0),30);
  sphere_xform_api.SetTranslate(pxr::GfVec3d(0,-5.00,0),45);
  sphere_xform_api.SetTranslate(pxr::GfVec3d(0,-3.25,0),50);
  sphere_xform_api.SetTranslate(pxr::GfVec3d(0,5.50,0),60);

  stage->GetRootLayer()->Save();

}
