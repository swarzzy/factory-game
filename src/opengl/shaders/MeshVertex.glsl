#version 450
#include Common.glh
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;

layout (location = 3) out VertOut
{
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    vec3 viewPosition;
    vec4 lightSpacePos[3];
} vertOut;

void main()
{
    gl_Position = FrameData.projectionMatrix * FrameData.viewMatrix * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.fragPos = (MeshData.modelMatrix * vec4(Pos, 1.0f)).xyz;
    vertOut.uv = UV;
    vertOut.normal = MeshData.normalMatrix * Normal;
    vertOut.viewPosition = (FrameData.viewMatrix * MeshData.modelMatrix * vec4(Pos, 1.0f)).xyz;
    vertOut.lightSpacePos[0] = FrameData.lightSpaceMatrices[0] * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.lightSpacePos[1] = FrameData.lightSpaceMatrices[1] * MeshData.modelMatrix * vec4(Pos, 1.0f);
    vertOut.lightSpacePos[2] = FrameData.lightSpaceMatrices[2] * MeshData.modelMatrix * vec4(Pos, 1.0f);
}