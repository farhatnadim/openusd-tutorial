#pragma once

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>

PXR_NAMESPACE_USING_DIRECTIVE

// A square-based pyramid: 5 points, 5 faces (4 triangles + 1 quad base).
struct PyramidMesh {
    VtVec3fArray points;
    VtIntArray faceVertexCounts;
    VtIntArray faceVertexIndices;
    VtVec3fArray extent;
};

PyramidMesh MakePyramid(float baseHalfWidth, float height);
