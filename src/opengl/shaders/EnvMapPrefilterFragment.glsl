#version 450

#include Common.glh
#include Pbr.glh

layout (location = 0) in vec3 UV;
out vec4 resultColor;

layout (binding = 0) uniform samplerCube uSourceCubemap;
layout (location = 0) uniform float uRoughness;
layout (location = 1) uniform int uResolution;

const uint SAMPLES = 4096u;

void main()
{
    vec3 N = normalize(UV);
    vec3 R = N;
    vec3 V = R;

    float totalWeight = 0.0f;
    vec3 prefColor = vec3(0.0f);

    for (uint i = 0u; i < SAMPLES; i++)
    {
        vec2 p = Hammersley(i, SAMPLES);
        vec3 H = ImportanceSampleGGX(p, uRoughness, N);
        vec3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        if (NdotL > 0.0f)
        {
            float NdotH = max(dot(N, H), 0.0f);
            float D = DistributionGGX(NdotH, uRoughness * uRoughness * uRoughness * uRoughness);
            float HdotV = max(dot(H, V), 0.0f);
            float PDF = D * NdotH / (4.0f * HdotV) + 0.0001;
            float saTexel = 4.0f * PI_32 / (6.0f * uResolution * uResolution);
            float saSample = 1.0f / (float(SAMPLES) * PDF + 0.0001f);
            float mipLevel = uRoughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);
            prefColor += textureLod(uSourceCubemap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefColor = prefColor / totalWeight;

    resultColor = vec4(prefColor, 1.0f);
}