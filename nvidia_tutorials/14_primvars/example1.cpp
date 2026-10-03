#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvar.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
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

// Example 1: the same two-quad mesh three times, with displayColor authored at
// constant, uniform and vertex interpolation.
static void PrimvarInterpolation()
{
  std::string file_path("_assets/primvars_displaycolor.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);
   
  UsdGeomXform world{UsdGeomXform::Define(stage,SdfPath("/World"))};
  stage->SetDefaultPrim(world.GetPrim());
  // two-quad mesh : 6 points, 2 faces
  VtArray<GfVec3f> mesh_vertex_locs =  {GfVec3f(-1,0,0), GfVec3f(0,0,0), GfVec3f(0,1,0), GfVec3f(-1,1,0), GfVec3f(1,0,0), GfVec3f(1,1,0) };
  VtArray<int> face_vertex_counts {4,4};
  VtArray<int> face_vertex_indices { 0,1,2,3, 1,4,5,2};

  VtArray<GfVec3f> per_prim_color = {GfVec3f(0.5, 0.0, 0.5)};
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
    example_mesh(SdfPath("PerPrim"),TfToken(UsdGeomTokens->constant),per_prim_color),
    example_mesh(SdfPath("PerFace"),TfToken(UsdGeomTokens->uniform),per_face_colors),
    example_mesh(SdfPath("PerVertex"),TfToken(UsdGeomTokens->vertex),per_vertex_colors)};
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
    UsdGeomXformCommonAPI(mesh_prim).SetTranslate(GfVec3d(index*2.5,0,0));
    mesh_disp_color_primvar = mesh_prim.GetDisplayColorPrimvar();
    mesh_disp_color_primvar.SetInterpolation(mesh.interpolation);
    mesh_disp_color_primvar.Set(mesh.colors);
    index++;
  }
  stage->Save();
  PrintStage(stage);
}

int main()
{
  PrimvarInterpolation();
  return 0;
}
