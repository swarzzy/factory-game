#include "Math.h"
#include "Platform.h"

BBoxAligned BBoxAligned::From(Mesh* mesh) {
    v3 min = V3(F32::Max);
    v3 max = V3(F32::Min);
    while (mesh) {
        for (u32x i = 0; i < mesh->vertexCount; i++) {
            v3 vertex = mesh->vertices[i];
            min.x = Min(min.x, vertex.x);
            min.y = Min(min.y, vertex.y);
            min.z = Min(min.z, vertex.z);

            max.x = Max(max.x, vertex.x);
            max.y = Max(max.y, vertex.y);
            max.z = Max(max.z, vertex.z);
        }
        mesh = mesh->next;
    }
    return { min, max };
}
