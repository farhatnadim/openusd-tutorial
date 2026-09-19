// Smoke test for an OpenUSD install: author a sphere, save it, read it back.
//
// Exercises the pieces most likely to be broken by a bad install: linking
// against the core libraries, plugin discovery (the .usda file format and the
// UsdGeomSphere schema are both plugins), and round-tripping through disk.

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

int failures = 0;

void Check(bool condition, const std::string& what) {
    std::cout << (condition ? "  ok   : " : "  FAIL : ") << what << "\n";
    if (!condition) {
        ++failures;
    }
}

constexpr double kRadius = 2.5;
const SdfPath kSpherePath("/World/Sphere");

// Author a tiny scene and write it to disk.
bool Write(const std::string& path) {
    UsdStageRefPtr stage = UsdStage::CreateNew(path);
    Check(bool(stage), "UsdStage::CreateNew(" + path + ")");
    if (!stage) {
        return false;
    }

    // Stage metadata: usdchecker treats a missing upAxis or metersPerUnit as
    // an error, since without them a consumer can't orient or scale the scene.
    UsdGeomSetStageUpAxis(stage, UsdGeomTokens->y);
    UsdGeomSetStageMetersPerUnit(stage, 0.01);

    UsdGeomXform::Define(stage, SdfPath("/World"));
    stage->SetDefaultPrim(stage->GetPrimAtPath(SdfPath("/World")));

    UsdGeomSphere sphere = UsdGeomSphere::Define(stage, kSpherePath);
    Check(bool(sphere), "UsdGeomSphere::Define(/World/Sphere)");
    if (!sphere) {
        return false;
    }

    sphere.CreateRadiusAttr().Set(kRadius);

    // A sphere's extent is not implied by its radius; author it explicitly so
    // the file passes usdchecker and bounds correctly in a renderer.
    VtVec3fArray extent;
    UsdGeomSphere::ComputeExtent(kRadius, &extent);
    sphere.CreateExtentAttr().Set(extent);

    UsdGeomXformCommonAPI(sphere).SetTranslate(GfVec3d(0.0, kRadius, 0.0));

    Check(stage->GetRootLayer()->Save(), "save layer to disk");
    return true;
}

// Reopen the file from scratch and confirm what we wrote survived.
void ReadBack(const std::string& path) {
    UsdStageRefPtr stage = UsdStage::Open(path);
    Check(bool(stage), "UsdStage::Open(" + path + ")");
    if (!stage) {
        return;
    }

    UsdPrim prim = stage->GetPrimAtPath(kSpherePath);
    Check(prim.IsValid(), "/World/Sphere exists");
    Check(prim.IsA<UsdGeomSphere>(), "/World/Sphere is a UsdGeomSphere");

    double radius = 0.0;
    UsdGeomSphere(prim).GetRadiusAttr().Get(&radius);
    Check(std::abs(radius - kRadius) < 1e-9,
          "radius round-trips as " + std::to_string(kRadius));

    Check(stage->GetDefaultPrim().GetPath() == SdfPath("/World"),
          "default prim is /World");

    Check(UsdGeomGetStageUpAxis(stage) == UsdGeomTokens->y, "upAxis is Y");
    Check(std::abs(UsdGeomGetStageMetersPerUnit(stage) - 0.01) < 1e-9,
          "metersPerUnit is 0.01");
}

}  // namespace

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "sphere.usda";

    std::cout << "OpenUSD install smoke test\n";
    std::cout << "authoring " << path << "\n";

    // CreateNew fails on an existing file; start clean so reruns work.
    std::remove(path.c_str());

    if (Write(path)) {
        std::cout << "reading back " << path << "\n";
        ReadBack(path);
    }

    std::cout << (failures == 0 ? "\nPASS\n" : "\nFAILED\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
