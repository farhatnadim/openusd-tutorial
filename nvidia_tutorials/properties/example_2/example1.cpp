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
  pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/relationships_ex1.usda");
  pxr::UsdGeomXform world_Xform = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
  pxr::SdfPath spherePath  = world_Xform.GetPath().AppendChild(pxr::TfToken("Sphere"));
  pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere::Define(stage,spherePath);
  sphere.CreateRadiusAttr().Set(2.);
  pxr::SdfPath cubePath  = world_Xform.GetPath().AppendChild(pxr::TfToken("Cube"));
  pxr::UsdGeomCube cube = pxr::UsdGeomCube::Define(stage,cubePath);
  pxr::UsdGeomXformCommonAPI(cube).SetTranslate(pxr::GfVec3d(5,0,0));
  
  pxr::UsdPrim group = stage->DefinePrim(pxr::SdfPath("/World/Group"));
  std::vector<pxr::SdfPath> geometries {spherePath, cubePath};

  group.CreateRelationship(pxr::TfToken("members"), true).SetTargets(geometries);
  pxr::SdfPathVector targets; 
  pxr::UsdRelationship members_rel = group.GetRelationship(pxr::TfToken("members")); 
  members_rel.GetTargets(&targets);
  for ( const pxr::SdfPath& path : targets)
        std::cout << " " << path.GetAsString();
  stage->GetRootLayer()->Save();
}

