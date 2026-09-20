#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

#include <iostream>

int main()
{
  const pxr::UsdStageRefPtr stage =
      pxr::UsdStage::Open("_assets/cube_prim.usda");

  const pxr::UsdPrim prim =
      stage->GetPrimAtPath(pxr::SdfPath("/Geometry"));

  const pxr::UsdPrim childPrim = prim.GetChild(pxr::TfToken("GroupTransform"));
  if (childPrim) {
    std::cout << "Child prim exists\n";
  } else {
    std::cout << "Child prim DOES NOT exist\n";
  }

  return 0;
}
