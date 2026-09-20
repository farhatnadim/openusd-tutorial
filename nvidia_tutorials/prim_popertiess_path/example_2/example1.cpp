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
   pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew("_assets/paths_property_authoring.usda");
   pxr::UsdGeomSphere sphere = pxr::UsdGeomSphere::Define(stage,pxr::SdfPath("/World/Geom/Sphere"));
   // create a property path 
   auto attr_property_path = sphere.GetPath().AppendProperty(pxr::TfToken("userProperties:tag"));
   // working with the property path for the attrubute
   auto owner_prim = stage->GetPrimAtPath(attr_property_path.GetPrimPath());
   auto attr_name = attr_property_path.GetNameToken();
   std::cout << "Attribute Property " << attr_name << " has been defined on " << owner_prim.GetPath() << " After Append Attribute " << owner_prim.GetAttribute(attr_name).IsDefined()
             << "\n";
   auto attr = owner_prim.CreateAttribute(attr_name,pxr::SdfValueTypeNames->String);
   attr.Set("surveyed");
   auto marker = pxr::UsdGeomXform::Define(stage,pxr::SdfPath("/World/Markers/MarkerA"));
   auto rel_property_path = sphere.GetPath().AppendProperty(pxr::TfToken("my:ref"));
   auto owner_prim_ = stage->GetPrimAtPath(rel_property_path.GetPrimPath());
   auto rel_name = rel_property_path.GetNameToken();
   auto rel = owner_prim_.CreateRelationship(rel_name);

   rel.AddTarget(marker.GetPath());
   

   stage->Save();
}
