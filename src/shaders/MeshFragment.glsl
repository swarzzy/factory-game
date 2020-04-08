#version 450
#include Common.glh
#include ShadowsCommon.glh

out vec4 Color;

layout (location = 3) in VertOut
{
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    vec3 viewPosition;
    vec4 lightSpacePos[3];
} fragIn;

layout (binding = 0) uniform sampler2D DiffMap;
layout (binding = 1) uniform sampler2D SpecMap;
layout (binding = 2) uniform sampler2DArrayShadow ShadowMap;

void main()
{
    vec3 normal = normalize(fragIn.normal);

    vec4 diffSample;
    if (MeshData.phongUseDiffuseMap == 1)
    {
        diffSample = texture(DiffMap, fragIn.uv);
    }
    else
    {
        diffSample = vec4(MeshData.customPhongDiffuse, 1.0f);
    }

    vec4 specSample;
    if (MeshData.phongUseSpecularMap == 1)
    {
        specSample = texture(SpecMap, fragIn.uv);
    }
    else
    {
        specSample = vec4(MeshData.customPhongSpecular, 1.0f);
    }

    specSample.a = 1.0f;
    vec3 lightDir = normalize(-FrameData.dirLight.dir);
    float kDiff = max(dot(normal, lightDir), 0.0f);
    vec3 viewDir = normalize(FrameData.viewPos - fragIn.fragPos);
    vec3 rFromLight = reflect(-lightDir, normal);
    float kSpec = pow(max(dot(viewDir, rFromLight), 0.0f), 32.0f);

    vec4 kShadow = vec4(CalcShadow(fragIn.viewPosition, FrameData.shadowCascadeSplits, fragIn.lightSpacePos, ShadowMap, FrameData.shadowFilterSampleScale, FrameData.showShadowCascadeBoundaries), 1.0f);

    vec4 ambient = diffSample * vec4(FrameData.dirLight.ambient, 1.0f);
    vec4 diffuse = diffSample * kDiff * vec4(FrameData.dirLight.diffuse, 1.0f) * kShadow;
    vec4 specular = specSample * kSpec * vec4(FrameData.dirLight.specular, 1.0f) * kShadow;
    Color = ambient + diffuse + specular;
}