#version 450
#include Common.glh
layout (location = 0) in vec3 Pos;

layout (location = 1) out vec3 Color;

void main()
{
    gl_Position = FrameData.viewProjMatrix * vec4(Pos, 1.0f);
    Color = MeshData.lineColor;
}