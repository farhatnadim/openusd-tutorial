#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// Example 2: the same two-quad mesh three times, with displayColor authored at
// constant, uniform and vertex interpolation.
static void PrimvarDeformation()
{
  unsigned start_tc {1}, end_tc{90} , time_code_per_second {30};
  std::string file_path("_assets/primvars_mesh_deformation.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
  stage->SetStartTimeCode(start_tc);
  stage->SetEndTimeCode(end_tc);
  stage->SetTimeCodesPerSecond(time_code_per_second);
  

  UsdGeomXform world{UsdGeomXform::Define(stage,SdfPath("/World"))};
  stage->SetDefaultPrim(world.GetPrim());
  VtArray<GfVec3f> mesh_vertex_locs =  { GfVec3f(0,0,0), GfVec3f(1,0,0), GfVec3f(1,1,0),  GfVec3f(0,1,0) };
  VtArray<int> face_vertex_counts {4};
  VtArray<int> face_vertex_indices {0,1,2,3};

  VtArray<GfVec3f> per_prim_color = {GfVec3f(0.1, 0.8, 0.1)};
  VtArray<GfVec3f> per_face_colors = {GfVec3f(0.0, 0.0, 1.0), GfVec3f(1.0, 0.0, 0.0)};
  VtArray<GfVec3f> per_vertex_colors = {
    GfVec3f(0.0, 0.0, 1.0), GfVec3f(0.5, 0.0, 0.5), GfVec3f(0.5, 0.0, 0.5),
    GfVec3f(0.0, 0.0, 1.0), GfVec3f(1.0, 0.0, 0.0), GfVec3f(1.0, 0.0, 0.0)};
  
  struct  example_mesh{
    SdfPath mesh_name;
    TfToken interpolation;
    VtArray<GfVec3f> colors;
  };
   
  std::vector<example_mesh> example_meshes = { 
    example_mesh(SdfPath("Plane"),TfToken(UsdGeomTokens->constant),per_prim_color)};
  UsdGeomMesh mesh_prim;
  UsdGeomPrimvar mesh_disp_color_primvar;
  unsigned index{0};
  for ( auto mesh : example_meshes)
  {
    mesh_prim = UsdGeomMesh::Define(
                                stage,world.GetPath().AppendPath(mesh.mesh_name));
    mesh_prim.GetPointsAttr().Set(mesh_vertex_locs);
    mesh_prim.GetFaceVertexCountsAttr().Set(face_vertex_counts);
    mesh_prim.GetFaceVertexIndicesAttr().Set(face_vertex_indices);
    UsdGeomXformCommonAPI(mesh_prim).SetTranslate(GfVec3d(2,0,0));
    mesh_disp_color_primvar = mesh_prim.GetDisplayColorPrimvar();
    mesh_disp_color_primvar.SetInterpolation(mesh.interpolation);
    mesh_disp_color_primvar.Set(mesh.colors);
    index++;
  }
  UsdGeomPrimvarsAPI plane_primvar_api{mesh_prim};
  plane_primvar_api.CreatePrimvar(TfToken("rest_state"), SdfValueTypeNames->Float3Array,UsdGeomTokens->vertex).Set(mesh_vertex_locs);
  VtArray<GfVec3f>  deformation {
    GfVec3f(0.0, 0.0, 0.0),
    GfVec3f(-0.3, 0.4, 0.0),
    GfVec3f(-0.3, 0.4, 0.0),
    GfVec3f(0.0, 0.0, 0.0),
  };
  
  plane_primvar_api = UsdGeomPrimvarsAPI(mesh_prim);
  plane_primvar_api.CreatePrimvar(TfToken("deformation"), SdfValueTypeNames->Float3Array,UsdGeomTokens->vertex).Set(deformation);
  
  VtArray<GfVec3f> new_points;
  new_points = mesh_vertex_locs + deformation;
  mesh_prim.GetPointsAttr().Set(mesh_vertex_locs, UsdTimeCode(start_tc));
  mesh_prim.GetPointsAttr().Set(new_points, UsdTimeCode(end_tc));

  stage->Save();
  PrintStage(stage);
}

int main()
{
  PrimvarDeformation();
  return 0;
}
