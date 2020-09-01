#version 450

#include Common.glh
#include Pbr.glh
#include ShadowsCommon.glh

out vec4 resultColor;

layout (location = 5) in VertOut
{
    vec3 fragPos;
    vec3 normal;
    vec2 uv;
    mat3 tbn;
    vec3 viewPosition;
    vec4 lightSpacePos[3];
} fragIn;

layout (binding = 0) uniform samplerCube IrradanceMap;
layout (binding = 1) uniform samplerCube EnviromentMap;
layout (binding = 2) uniform sampler2D BRDFLut;

layout (binding = 3) uniform sampler2D AlbedoMap;
layout (binding = 4) uniform sampler2D NormalMap;

layout (binding = 5) uniform sampler2D RoughnessMap;
layout (binding = 6) uniform sampler2D MetallicMap;

layout (binding = 7) uniform sampler2D SpecularMap;
layout (binding = 8) uniform sampler2D GlossMap;
layout (binding = 10) uniform sampler2D AOMap;
layout (binding = 11) uniform sampler2D EmissionMap;

layout (binding = 9) uniform sampler2DArrayShadow ShadowMap;
//uniform sampler2D uAOMap;

void main()
{
    vec3 V = normalize(FrameData.viewPos - fragIn.fragPos);

    PBR context;

    vec3 N;
    if (MeshData.pbrUseNormalMap == 1)
    {
        vec3 n = texture(NormalMap, fragIn.uv).xyz * 2.0f - 1.0f;
        if (MeshData.normalFormat == 0)
        {
            // OpenGL format
        }
        else
        {
            // NOTE: Flipping y because engine uses LH normal maps (UE4) but OpenGL does it's job in RH space
            n.y = -n.y;
        }

        N = normalize(n);
#if 0
        float roughness = texture(RoughnessMap, fragIn.uv).x;
        N = mix(N, vec3(0.0, 0.0, 1.0), pow(roughness, 0.5)); // smooth normal based on roughness (to reduce specular aliasing)
        N.x *= 2.0;
        N.y *= 2.0;
#endif
        N = normalize(N);

        N = normalize(fragIn.tbn * N);
    }
    else
    {
        N = normalize(fragIn.normal);
    }

    vec3 albedo;
    if (MeshData.pbrUseAlbedoMap == 1)
    {
        albedo = texture(AlbedoMap, fragIn.uv).xyz;
    }
    else
    {
        albedo = MeshData.pbrAlbedoValue;
    }

    float AO;
    if (MeshData.pbrUseAOMap == 1)
    {
        AO = texture(AOMap, fragIn.uv).x;
    }
    else
    {
        AO = 1.0f;
    }

    vec3 emissionColor = vec3(0.0f);
    if (MeshData.emitsLight == 1)
    {
        if (MeshData.pbrUseEmissionMap == 1)
        {
            emissionColor = texture(EmissionMap, fragIn.uv).xyz;
        }
        else
        {
            emissionColor = MeshData.pbrEmissionValue;
        }
    }

    if (MeshData.metallicWorkflow == 1)
    {
        float roughness;
        if (MeshData.pbrUseRoughnessMap == 1)
        {
            roughness = texture(RoughnessMap, fragIn.uv).x;
        }
        else
        {
            roughness = MeshData.pbrRoughnessValue;
        }

        float metallic;
        if (MeshData.pbrUseMetallicMap == 1)
        {
            metallic = texture(MetallicMap, fragIn.uv).x;
        }
        else
        {
            metallic = MeshData.pbrMetallicValue;
        }

        context = InitPBRMetallic(V, N, albedo, metallic, roughness, AO);
    }
    else // Specular workflow
    {
        vec3 specular;
        if (MeshData.pbrUseSpecularMap == 1)
        {
            specular = texture(SpecularMap, fragIn.uv).xyz;
        }
        else
        {
            specular = MeshData.pbrSpecularValue;
        }

        float gloss;
        if (MeshData.pbrUseGlossMap == 1)
        {
            gloss = texture(GlossMap, fragIn.uv).x;
        }
        else
        {
            gloss = MeshData.pbrGlossValue;
        }
         context = InitPBRSpecular(V, N, albedo, specular, gloss, AO);
    }

    vec3 L = normalize(-FrameData.dirLight.dir);
    vec3 H = normalize(V + L);

    vec3 dirRadiance = FrameData.dirLight.diffuse;
    dirRadiance = Unreal4DirectionalLight(context, L) * dirRadiance;

    vec3 envRadiance = Unreal4EnviromentLight(context, IrradanceMap, EnviromentMap, BRDFLut);

    vec3 kShadow = CalcShadow(fragIn.viewPosition, FrameData.shadowCascadeSplits, fragIn.lightSpacePos, ShadowMap, FrameData.shadowFilterSampleScale, FrameData.showShadowCascadeBoundaries);

    resultColor = vec4((envRadiance +  dirRadiance * kShadow + emissionColor), 1.0f);
}
