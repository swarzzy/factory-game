#version 450
layout (location = 0) in vec3 UV;

out vec4 Color;

layout (binding = 0) uniform samplerCube CubeTexture;

void main()
{
    // NOTE: Temporary using low-res mip of enviroment map
    Color = textureLod(CubeTexture, UV, 5.0f);
}