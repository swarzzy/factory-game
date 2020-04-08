#version 450
#include Common.glh

out vec4 color;

void main()
{
    gl_FragDepth = gl_FragCoord.z + FrameData.constShadowBias;
    color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
}
