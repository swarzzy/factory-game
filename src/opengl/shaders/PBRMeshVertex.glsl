#version 450
#include Common.glh
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bitangent;

layout (location = 5) out VertOut
{
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    mat3 tbn;
    vec3 viewPosition;
    vec4 lightSpacePos[3];
} vertOut;

void main()
{
    vec3 n = normalize(MeshData.normalMatrix * Normal);
    vec3 t = normalize(MeshData.normalMatrix * Tangent);
    t = normalize(t - dot(t, n) * n);
    vec3 b;
    if (MeshData.hasBitangents == 1)
    {
        b = normalize(MeshData.normalMatrix * Bitangent);
    }
    else
    {
        b = normalize(cross(n, t));
    }
    mat3 tbn = mat3(t, b, n);

    gl_Position = FrameData.viewProjMatrix * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.fragPos = (MeshData.modelMatrix * vec4(Pos, 1.0f)).xyz;
    vertOut.uv = UV;
    vertOut.normal = n;
    vertOut.tbn = tbn;
    vertOut.viewPosition = (FrameData.viewMatrix * MeshData.modelMatrix * vec4(Pos, 1.0f)).xyz;
    vertOut.lightSpacePos[0] = FrameData.lightSpaceMatrices[0] * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.lightSpacePos[1] = FrameData.lightSpaceMatrices[1] * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.lightSpacePos[2] = FrameData.lightSpaceMatrices[2] * MeshData.modelMatrix * vec4(Pos, 1.0f);
}