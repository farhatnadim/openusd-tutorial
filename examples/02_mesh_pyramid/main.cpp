// A multi-file example: geometry generation lives in geometry.cpp.
// Everything in examples/02_mesh_pyramid/ compiles into one target.

#include "geometry.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>

#include <iostream>

int main() {
    const std::string path = "pyramid.usda";

    UsdStageRefPtr stage = UsdStage::CreateNew(path);
    if (!stage) {
        std::cerr << "failed to create " << path << "\n";
        return 1;
    }

    UsdGeomSetStageUpAxis(stage, UsdGeomTokens->y);
    UsdGeomSetStageMetersPerUnit(stage, 0.01);

    UsdGeomXform::Define(stage, SdfPath("/World"));
    stage->SetDefaultPrim(stage->GetPrimAtPath(SdfPath("/World")));

    const PyramidMesh data = MakePyramid(1.0f, 1.5f);

    UsdGeomMesh mesh = UsdGeomMesh::Define(stage, SdfPath("/World/Pyramid"));
    mesh.CreatePointsAttr().Set(data.points);
    mesh.CreateFaceVertexCountsAttr().Set(data.faceVertexCounts);
    mesh.CreateFaceVertexIndicesAttr().Set(data.faceVertexIndices);
    mesh.CreateExtentAttr().Set(data.extent);
    mesh.CreateSubdivisionSchemeAttr().Set(UsdGeomTokens->none);

    if (!stage->GetRootLayer()->Save()) {
        std::cerr << "failed to save " << path << "\n";
        return 1;
    }

    std::cout << "wrote " << path << " ("
              << data.points.size() << " points, "
              << data.faceVertexCounts.size() << " faces)\n";
    return 0;
}
