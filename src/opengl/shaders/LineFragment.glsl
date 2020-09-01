#version 450
out vec4 fragColor;

layout(location = 1) in vec3 Color;

void main()
{
    fragColor = vec4(Color, 1.0f);
}