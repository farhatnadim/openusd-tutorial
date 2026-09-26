#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/inherits.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/tf/enum.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// Example 1: defs and a class in a base layer.
static void AuthorBase()
{
  std::string base_file_path("_assets/specifiers_base.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(base_file_path);
   
  UsdGeomXform world = UsdGeomXform::Define(stage,SdfPath("/World"));
  stage->SetDefaultPrim(world.GetPrim());
   
  UsdGeomCube box_prim_cube = UsdGeomCube::Define(stage, world.GetPath().AppendChild(TfToken("Box")));
  box_prim_cube.GetSizeAttr().Set(2.0);
  
  UsdGeomCube box_2_prim_cube = UsdGeomCube::Define(stage, world.GetPath().AppendChild(TfToken("Box_2")));
  box_2_prim_cube.GetSizeAttr().Set(2.0);

  UsdGeomXformCommonAPI box_2_api = UsdGeomXformCommonAPI(box_2_prim_cube);
  box_2_api.SetTranslate(GfVec3d(5,0,0));

  UsdPrim class_prim(stage->CreateClassPrim(world.GetPath().AppendPath(SdfPath("_Look/_green"))));
  UsdGeomPrimvarsAPI class_prim_primvar_api(class_prim);
  class_prim_primvar_api.CreatePrimvar(TfToken("displayColor"), SdfValueTypeNames->Color3fArray).Set(VtArray<GfVec3f> {GfVec3f(0.1,0.8,0.2)});
  



  stage->Save();
  PrintStage(stage);
}

// Example 2: an over layer that sublayers the base.
static void AuthorOver()
{
  std::string new_file_path("_assets/specifiers_over_base.usda");
  std::string base_file_path("specifiers_base.usda");   // relative to the new layer
  UsdStageRefPtr stage = UsdStage::CreateNew(new_file_path);
  stage->GetRootLayer()->InsertSubLayerPath(base_file_path);
  std::string prim_path("/World/Box");
  stage->OverridePrim(SdfPath(prim_path));
  

  UsdGeomCube box_prim_cube = UsdGeomCube::Get(stage, SdfPath(prim_path)); 
  box_prim_cube.GetSizeAttr().Set(4.0);
  box_prim_cube.GetPrim().GetInherits().AddInherit(SdfPath("/World/_Look/_green"));
  
  for ( auto const & prim_spec : box_prim_cube.GetPrim().GetPrimStack())
  {
    std::cout << "Specification " << prim_spec->GetLayer()->GetDisplayName() << " " << TfEnum::GetName(prim_spec->GetSpecifier()) << "\n" ;
  }

   

  stage->Save();
  PrintStage(stage);
}

int main()
{
  AuthorBase();
  AuthorOver();
}
