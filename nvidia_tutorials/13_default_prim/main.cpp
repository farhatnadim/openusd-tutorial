#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>

#include <iostream>
#include <string>

using namespace pxr;

static void PrintStage(const UsdStageRefPtr& stage)
{
  std::string result;
  stage->GetRootLayer()->ExportToString(&result);
  std::cout << result << "\n";
}

// Example 1: setting a default prim.
static void SetDefaultPrim()
{
  std::string file_path("_assets/default_prim.usda");
  UsdStageRefPtr stage = UsdStage::CreateNew(file_path);

  // Two prims with no type name: /hello and its child /hello/world.
  stage->DefinePrim(SdfPath("/hello"));
  stage->DefinePrim(SdfPath("/hello/world"));

  // The default prim is written in the metadata of the root layer.
  UsdPrim hello_prim = stage->GetPrimAtPath(SdfPath("/hello"));
  stage->SetDefaultPrim(hello_prim);

  stage->Save();
  PrintStage(stage);
}

int main()
{
  SetDefaultPrim();
  return 0;
}
