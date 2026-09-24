#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3f.h>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace pxr;
int main()
{
  std::string file_path("_assets/lights_props.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
  UsdGeomXform world = UsdGeomXform::Define(stage, SdfPath("/World"));
  UsdGeomScope scope = UsdGeomScope::Define(stage, world.GetPath().AppendChild(TfToken("Geometry")));

  // A cube under the Geometry scope.
  UsdGeomCube cube = UsdGeomCube::Define(stage, scope.GetPath().AppendChild(TfToken("Box")));

  // A sibling scope holding both lights.
  UsdGeomScope lights_scope = UsdGeomScope::Define(stage, world.GetPath().AppendChild(TfToken("Lights")));
  UsdLuxSphereLight sphere_light =
      UsdLuxSphereLight::Define(stage, lights_scope.GetPath().AppendChild(TfToken("SphereLight")));
  UsdLuxDistantLight distant_light =
      UsdLuxDistantLight::Define(stage, lights_scope.GetPath().AppendChild(TfToken("SunLight")));
  
  distant_light.GetColorAttr().Set(GfVec3f(1.0f, 0.0f, 0.0f));
  distant_light.GetIntensityAttr().Set(120.0f);
  
  UsdGeomXformCommonAPI distant_light_xform_api(distant_light);
  if(!distant_light_xform_api)
    throw std::runtime_error("Prim not compatible with XformCommonAPI");
  distant_light_xform_api.SetRotate(GfVec3f(45.0,0.0,0.0));

  
  UsdGeomXformCommonAPI sphere_light_xform_api(sphere_light);
  sphere_light.GetColorAttr().Set(GfVec3f(0.0,0.0,1.0));
  sphere_light.GetIntensityAttr().Set(50000.0f);
  
  if(!sphere_light_xform_api) 
      throw std::runtime_error("Prim not compatibale with XformCommonAPI");
  sphere_light_xform_api.SetTranslate(GfVec3f(5.0,10.0,0.0));
  
  

  stage->Save();

  std::string result;
  stage->ExportToString(&result, /*addSourceFileComment=*/false);
  std::cout << result << "\n";



}
