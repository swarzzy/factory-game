#version 450
#include Common.glh

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

layout (location = 0) uniform int CascadeIndex;

void main()
{
    mat4 viewProj = FrameData.lightSpaceMatrices[CascadeIndex];
    vec3 normal = normalize(Normal);
    float NdotL = dot(normal, FrameData.dirLight.pos);
    gl_Position = viewProj * vec4(Position, 1.0f);
}