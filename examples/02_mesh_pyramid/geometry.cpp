#include "geometry.h"

#include <pxr/base/gf/range3f.h>

PyramidMesh MakePyramid(float b, float h) {
    PyramidMesh m;

    m.points = {
        GfVec3f(-b, 0.0f, -b),  // 0
        GfVec3f( b, 0.0f, -b),  // 1
        GfVec3f( b, 0.0f,  b),  // 2
        GfVec3f(-b, 0.0f,  b),  // 3
        GfVec3f(0.0f, h, 0.0f)  // 4 apex
    };

    // Four triangular sides, then the quad base. Counter-clockwise winding so
    // the default (rightHanded) orientation points normals outward.
    m.faceVertexCounts = {3, 3, 3, 3, 4};
    m.faceVertexIndices = {
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4,
        3, 2, 1, 0
    };

    GfRange3f bounds;
    for (const GfVec3f& p : m.points) {
        bounds.UnionWith(p);
    }
    m.extent = {bounds.GetMin(), bounds.GetMax()};

    return m;
}
