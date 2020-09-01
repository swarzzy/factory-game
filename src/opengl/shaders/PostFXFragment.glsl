#version 450
#include Common.glh
#include Tonemapping.glh

layout (location = 0) in vec2 UV;

out vec4 fragColorResult;

layout (binding = 0) uniform sampler2D ColorSourceLinear;

float D3DX_FLOAT_to_SRGB(float val)
{
    if (val < 0.0031308f)
    {
        val *= 12.92f;
    }
    else
    {
        val = 1.055f * pow(val, 1.0f / FrameData.gamma) - 0.055f;
    }
    return val;
}

vec3 D3DX_RGB_to_SRGB(vec3 rgb)
{
    rgb.r = D3DX_FLOAT_to_SRGB(rgb.r);
    rgb.g = D3DX_FLOAT_to_SRGB(rgb.g);
    rgb.b = D3DX_FLOAT_to_SRGB(rgb.b);
    return rgb;
}

void main()
{
    vec3 hdrSample = texture(ColorSourceLinear, UV).xyz;

    vec3 ldrSample = ACESFilmApproxTonemap(hdrSample * FrameData.exposure);
    //vec3 ldrSample = ACESFilmStephenHillTonemap(hdrSample * FrameData.exposure);
    //vec3 ldrSample = Uncharted2Tonemap(hdrSample * FrameData.exposure);
    //vec3 ldrSample = ReinhardTonemap(hdrSample * FrameData.exposure);
    //vec3 ldrSample = Uncharted2TonemapLuminance(hdrSample * FrameData.exposure);
    //vec3 ldrSample = ACESFilmStephenHillTonemapLuminance(hdrSample * FrameData.exposure);

    vec3 resultSample = D3DX_RGB_to_SRGB(ldrSample * FrameData.exposure);
    fragColorResult = vec4(resultSample, 1.0f);
}