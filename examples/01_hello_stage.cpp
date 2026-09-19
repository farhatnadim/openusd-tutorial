// Minimal in-memory stage: define a couple of prims and print the result.
// Nothing touches disk, so this is the quickest check that USD is wired up.

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>

#include <iostream>
#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

int main() {
    UsdStageRefPtr stage = UsdStage::CreateInMemory();
    UsdGeomSetStageUpAxis(stage, UsdGeomTokens->y);

    UsdGeomXform::Define(stage, SdfPath("/World"));
    stage->SetDefaultPrim(stage->GetPrimAtPath(SdfPath("/World")));

    UsdGeomCube cube = UsdGeomCube::Define(stage, SdfPath("/World/Cube"));
    cube.CreateSizeAttr().Set(2.0);

    std::string usda;
    stage->ExportToString(&usda);
    std::cout << usda;

    return cube ? 0 : 1;
}
