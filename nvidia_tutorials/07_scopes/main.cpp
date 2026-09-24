#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/scope.h>






int main()

{
    pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/scope.usda");
    pxr::UsdGeomXform xform = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World"));
    pxr::UsdGeomSphere sphere;
    pxr::UsdGeomCube cube;

    auto num_a_prims = 2; 
    auto num_b_prims = 2;
    
    pxr::UsdGeomScope a_scope = pxr::UsdGeomScope::Define(stage, xform.GetPath().AppendChild(pxr::TfToken("A_Scope")));
    pxr::UsdGeomScope b_scope = pxr::UsdGeomScope::Define(stage, xform.GetPath().AppendChild(pxr::TfToken("B_Scope")));
    
    for ( unsigned scope_index = 0 ; scope_index < num_a_prims; ++ scope_index)
    {
       sphere = pxr::UsdGeomSphere::Define(stage,a_scope.GetPath().AppendChild(pxr::TfToken(std::string("A_sphere") + std::to_string(scope_index))));
       cube = pxr::UsdGeomCube::Define(stage,b_scope.GetPath().AppendChild(pxr::TfToken(std::string("B_cube") + std::to_string(scope_index))));
       pxr::UsdGeomXformCommonAPI(sphere).SetTranslate(pxr::GfVec3d(scope_index*2.5,0,0));
       pxr::UsdGeomXformCommonAPI(cube).SetTranslate(pxr::GfVec3d(scope_index*2.5,-2.5,0));

    }
    a_scope.GetPrim().SetActive(false);

    stage->Save();

} // end of main
