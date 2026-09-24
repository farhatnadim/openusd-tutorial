#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/tf/token.h>

#include <iostream>
#include <string>

int main()
{
    std::string file_path = "_assets/xform_common_api.usda";
    pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateNew(file_path);

    pxr::UsdGeomXform world = pxr::UsdGeomXform::Define(stage, pxr::SdfPath("/World"));

    // A second Xform, parented under /World.
    pxr::UsdGeomXform Parent_Prim =
        pxr::UsdGeomXform::Define(stage, world.GetPath().AppendChild(pxr::TfToken("Parent_Prim")));

    auto parent_xform_api = pxr::UsdGeomXformCommonAPI(Parent_Prim);
    parent_xform_api.SetTranslate(pxr::GfVec3d(5, 0, 3));
    parent_xform_api.SetRotate(pxr::GfVec3f(90, 0, 0));
    parent_xform_api.SetScale(pxr::GfVec3f(3, 3, 3));

    // A Cone under Parent_Prim, offset along X.
    auto child_translation = pxr::GfVec3d(2, 0, 0);
    pxr::UsdGeomCone child_a =
        pxr::UsdGeomCone::Define(stage, Parent_Prim.GetPath().AppendChild(pxr::TfToken("Child_A")));
    auto child_a_xform_api = pxr::UsdGeomXformCommonAPI(child_a);
    child_a_xform_api.SetTranslate(child_translation);

    // A second parent under /World, this one with no transform of its own.
    auto Alt_Parent =
        pxr::UsdGeomXform::Define(stage, world.GetPath().AppendChild(pxr::TfToken("Alt_Parent")));

    // A Cone under Alt_Parent, with the same local translation as Child_A.
    pxr::UsdGeomCone child_b =
        pxr::UsdGeomCone::Define(stage, Alt_Parent.GetPath().AppendChild(pxr::TfToken("Child_B")));
    auto child_b_xform_api = pxr::UsdGeomXformCommonAPI(child_b);
    child_b_xform_api.SetTranslate(child_translation);

    stage->Save();

    std::string result;
    stage->ExportToString(&result, /*addSourceFileComment=*/false);
    std::cout << result << "\n";
}
