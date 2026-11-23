#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/schemaRegistry.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdLux/diskLight.h>
#include <pxr/usd/usdPhysics/rigidBodyAPI.h>
#include <pxr/base/gf/vec3f.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// UsdSchemaKind has no TfEnum names, so the kind is printed by its enumerator.
static const char* KindName(UsdSchemaKind kind)
{
  switch (kind) {
    case UsdSchemaKind::AbstractBase: return "AbstractBase";
    case UsdSchemaKind::AbstractTyped: return "AbstractTyped";
    case UsdSchemaKind::ConcreteTyped: return "ConcreteTyped";
    case UsdSchemaKind::NonAppliedAPI: return "NonAppliedAPI";
    case UsdSchemaKind::SingleApplyAPI: return "SingleApplyAPI";
    case UsdSchemaKind::MultipleApplyAPI: return "MultipleApplyAPI";
    case UsdSchemaKind::Invalid: break;
  }
  return "Invalid";
}

// Example 1: the schema registry. The page names FindSchemaInfo and
// GetSchemaTypeName with no arguments; here they answer for one IsA schema
// and one API schema, given as template arguments.
static void SchemaRegistry()
{
  const UsdSchemaRegistry::SchemaInfo* sphere_info =
      UsdSchemaRegistry::FindSchemaInfo<UsdGeomSphere>();
  std::cout << "UsdGeomSphere: identifier " << sphere_info->identifier
            << ", family " << sphere_info->family
            << ", kind " << KindName(sphere_info->kind) << "\n";

  const UsdSchemaRegistry::SchemaInfo* rigid_body_info =
      UsdSchemaRegistry::FindSchemaInfo<UsdPhysicsRigidBodyAPI>();
  std::cout << "UsdPhysicsRigidBodyAPI: kind "
            << KindName(rigid_body_info->kind) << "\n";

  std::cout << "Schema type names: "
            << UsdSchemaRegistry::GetSchemaTypeName<UsdGeomSphere>() << ", "
            << UsdSchemaRegistry::GetSchemaTypeName<UsdPhysicsRigidBodyAPI>()
            << "\n\n";
}

// Example 2: an IsA schema, UsdGeomSphere.
static void SphereSchema()
{
  UsdStageRefPtr stage = UsdStage::CreateNew("_assets/schemas_sphere.usda");
  UsdGeomSphere sphere = UsdGeomSphere::Define(stage, SdfPath("/World/Sphere"));

  // radius is a double. Set(10) gives an int, and returns false.
  sphere.GetRadiusAttr().Set(10.0);

  stage->Save();
  PrintStage(stage);
}

// Example 3: an IsA schema, UsdLuxDiskLight.
static void DiskLightSchema()
{
  UsdStageRefPtr stage = UsdStage::CreateNew("_assets/schemas_disk_light.usda");
  UsdLuxDiskLight disk_light =
      UsdLuxDiskLight::Define(stage, SdfPath("/World/Lights/DiskLight"));

  // A static member function. includeInherited is true by default.
  std::cout << "UsdLuxDiskLight schema attribute names:";
  for (const TfToken& name : UsdLuxDiskLight::GetSchemaAttributeNames()) {
    std::cout << " " << name;
  }
  std::cout << "\n";

  // The two attributes are float.
  disk_light.GetRadiusAttr().Set(0.4f);
  disk_light.GetIntensityAttr().Set(1000.0f);

  stage->Save();
  PrintStage(stage);
}

// Example 4: an API schema, UsdPhysicsRigidBodyAPI, applied to a cube.
static void RigidBodyAPISchema()
{
  UsdStageRefPtr stage = UsdStage::CreateNew("_assets/schemas_rigid_body.usda");
  UsdGeomCube cube = UsdGeomCube::Define(stage, SdfPath("/World/Cube"));

  UsdPhysicsRigidBodyAPI rigid_body = UsdPhysicsRigidBodyAPI::Apply(cube.GetPrim());

  bool kinematic_enabled = true;
  rigid_body.GetKinematicEnabledAttr().Get(&kinematic_enabled);
  std::cout << "physics:kinematicEnabled: "
            << (kinematic_enabled ? "true" : "false") << "\n";

  rigid_body.CreateVelocityAttr(VtValue(GfVec3f(5, 0, 0)));

  stage->Save();
  PrintStage(stage);
}

int main()
{
  SchemaRegistry();
  SphereSchema();
  DiskLightSchema();
  RigidBodyAPISchema();
  return 0;
}
