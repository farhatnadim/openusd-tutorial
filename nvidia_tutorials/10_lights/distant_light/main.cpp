#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/token.h>

#include <iostream>
#include <string>

using namespace pxr;
int main()
{
  std::string file_path("_assets/distant_light.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
  UsdGeomXform world = UsdGeomXform::Define(stage, SdfPath("/World"));
  UsdGeomScope scope = UsdGeomScope::Define(stage, world.GetPath().AppendChild(TfToken("Geometry")));

  // A cube under the Geometry scope, so the light has something to fall on.
  UsdGeomCube cube = UsdGeomCube::Define(stage, scope.GetPath().AppendChild(TfToken("Box")));

  // A sibling scope for the lights.
  UsdGeomScope lights_scope = UsdGeomScope::Define(stage, world.GetPath().AppendChild(TfToken("Lights")));
  UsdLuxDistantLight distant_light = UsdLuxDistantLight::Define(stage,lights_scope.GetPath().AppendChild(TfToken("SunLight")));
  
  stage->Save();

  std::string result;
  stage->ExportToString(&result, /*addSourceFileComment=*/false);
  std::cout << result << "\n";
}
