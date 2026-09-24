#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/sdf/path.h>

#include <iostream>
#include <string>

int main()
{
    std::string file_path = "_assets/xform_prim.usda";
    pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew(file_path);

    pxr::UsdGeomXform world = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/World"));

    stage->Save();

    std::string result;
    stage->ExportToString(&result, /*addSourceFileComment=*/false);
    std::cout << result << "\n";
}
