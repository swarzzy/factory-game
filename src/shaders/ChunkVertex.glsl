#version 450
#include Common.glh

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Tangent;
layout (location = 3) in uint VoxelValue;

layout (location = 4) out VertOut {
    flat uint voxelValue;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
    vec3 position;
} vertOut;

layout (location = 0) uniform vec3 Offset;

#define TERRAIN_TEX_ARRAY_NUM_LAYERS 32
#define INDICES_PER_CHUNK_QUAD 6
#define VERTICES_PER_QUAD 4

vec2 UV[] = vec2[](vec2(0.0f, 0.0f),
                   vec2(1.0f, 0.0f),
                   vec2(1.0f, 1.0f),
                   vec2(0.0f, 1.0f));
void main()
{
    int vertIndexInQuad = gl_VertexID % 4;

    vertOut.uv = UV[min(vertIndexInQuad, VERTICES_PER_QUAD - 1)];

    vertOut.voxelValue = VoxelValue;
    vertOut.position = Position + Offset;
    vertOut.normal = Normal;
    gl_Position = FrameData.projectionMatrix * FrameData.viewMatrix * vec4(vertOut.position, 1.0f);
}