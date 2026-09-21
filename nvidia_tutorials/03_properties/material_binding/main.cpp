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
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/relationships_ex3.usda");
  pxr::UsdGeomXform world_Xform = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
  std::vector<pxr::SdfPath> cubePaths;
  std::vector<pxr::UsdGeomCube> cubes;
  constexpr unsigned NUMBER_OF_PATH {3};
  for (unsigned  path_index {0} ; path_index < NUMBER_OF_PATH; ++path_index)
  {

    cubePaths.push_back(world_Xform.GetPath().AppendChild(pxr::TfToken(std::string("Cube_") + std::to_string(path_index))));
    cubes.push_back(pxr::UsdGeomCube::Define(stage,cubePaths[path_index]));
  }
  pxr::UsdGeomXformCommonAPI(cubes[1]).SetTranslate(pxr::GfVec3d(5,0,0));
  pxr::UsdGeomXformCommonAPI(cubes[2]).SetTranslate(pxr::GfVec3d(10,0,0));
  
  auto looks = stage->DefinePrim(pxr::SdfPath("/World/Looks"));
  // green
  pxr::UsdShadeMaterial green = pxr::UsdShadeMaterial::Define(stage,looks.GetPath().AppendChild(pxr::TfToken("GreenMat")));
  pxr::UsdShadeShader green_ps =  pxr::UsdShadeShader::Define(stage,green.GetPath().AppendChild(pxr::TfToken("PreviewSurface")));
  green_ps.CreateIdAttr(pxr::VtValue("UsdPreviewSurface"));
  green_ps.CreateInput(pxr::TfToken("diffuseColor"),pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(0.0,1.0,0.0));
  green.CreateSurfaceOutput().ConnectToSource(green_ps.ConnectableAPI(),pxr::TfToken("surface"));
  // red
  pxr::UsdShadeMaterial red = pxr::UsdShadeMaterial::Define(stage,looks.GetPath().AppendChild(pxr::TfToken("RedMat")));
  pxr::UsdShadeShader red_ps =  pxr::UsdShadeShader::Define(stage,red.GetPath().AppendChild(pxr::TfToken("PreviewSurface")));
  red_ps.CreateIdAttr(pxr::VtValue("UsdPreviewSurface"));
  red_ps.CreateInput(pxr::TfToken("diffuseColor"),pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(1.0,0.0,0.0));
  red.CreateSurfaceOutput().ConnectToSource(red_ps.ConnectableAPI(),pxr::TfToken("surface"));
  for ( auto it = cubes.begin(); it != cubes.end() ; ++it )
  {
    pxr::UsdShadeMaterialBindingAPI::Apply(it->GetPrim()).Bind(green);

  }
  pxr::UsdShadeMaterialBindingAPI::Apply(cubes[2].GetPrim()).Bind(red);

  for ( const auto & prim : cubes)
  { 
    auto mat = pxr::UsdShadeMaterialBindingAPI(prim).GetDirectBinding().GetMaterial();
    std::cout << mat.GetPath() << "->" << (!mat.GetPath().IsEmpty() ? mat.GetPath().GetString() : "None ") << "\n";
  }
  stage->GetRootLayer()->Save();

}
