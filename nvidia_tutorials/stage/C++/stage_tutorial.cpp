#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/layer.h>
#include <filesystem>
#include <iostream>


using namespace pxr;
namespace fs = std::filesystem;

int main()
{
  UsdStageRefPtr stage = UsdStage::CreateNew("_assets/root_layer_example_cpp.usda");
  SdfLayerHandle rootLayer = stage->GetRootLayer();
  std::cout << "Root Layer Identifier :" 
            << fs::relative(rootLayer->GetIdentifier()) << std::endl;
  
  stage->DefinePrim(SdfPath("/World"), TfToken("Xform"));
  SdfLayerRefPtr extraLayer = SdfLayer::CreateNew("_assets/extra_layer_cpp.usdc");
  const std::string relativePath = "./" + fs::path(extraLayer->GetIdentifier()).filename().generic_string();
  rootLayer->GetSubLayerPaths().push_back(relativePath);
  stage->Save();
  extraLayer->Save();

  std::string rootLayerContents;
  rootLayer->ExportToString(&rootLayerContents);
  std::cout << "Root Layer Contents :\n" << rootLayerContents;
  return 0;
}
