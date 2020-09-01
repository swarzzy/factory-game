#version 450
#include Common.glh
#include Pbr.glh

layout (location = 0) in vec3 UV;

layout (binding = 0) uniform samplerCube EnviromentMapSampler;
layout (location = 0) uniform float Roughness;
layout (location = 1) uniform int Resolution;

out vec4 ResultColor;

// [ Real Shading in Unreal Engine 4 (Brian Karis), Siggraph 2013]
vec3 PrefilterEnvMap(float roughness, vec3 R) {
    vec3 N = R;
    vec3 V = R;

    vec3 prefilteredColor = vec3(0.0f);
    float totalWeight = 0.0f;

    const uint numSamples = 4096u;
    for (uint i = 0u; i < numSamples; i++) {
        vec2 Xi = Hammersley(i, numSamples);
        vec3 H = ImportanceSampleGGX(Xi, roughness, N);
        vec3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = saturate(dot(N, L));
        if (NdotL > 0.0f) {
            // TODO: Check this https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
            // PDF sampling
            // [https://chetanjags.wordpress.com/2015/08/26/image-based-lighting/]
            float alpha = Roughness;
            float alphaG = Roughness * Roughness;
            float NdotH = max(dot(N, H), 0.0f);
            float D = DistributionGGX(NdotH, alphaG * alphaG);
            float HdotV = max(dot(H, V), 0.0f);
            float PDF = D * NdotH / (4.0f * HdotV) + 0.0001;
            float saTexel = 4.0f * PI_32 / (6.0f * Resolution * Resolution);
            float saSample = 1.0f / (float(numSamples) * PDF + 0.0001f);
            float mipLevel = Roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

            prefilteredColor += textureLod(EnviromentMapSampler, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    return prefilteredColor / totalWeight;
}

void main() {
    ResultColor = vec4(PrefilterEnvMap(Roughness, normalize(UV)), 1.0f);
}
