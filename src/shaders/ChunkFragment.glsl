#version 450

#include Common.glh

layout (location = 4) in VertOut {
    flat uint voxelValue;
    vec3 normal;
    vec3 tangent;
    vec2 uv;
    vec3 position;
} vertIn;

layout (binding = 0) uniform sampler2DArray TerrainAtlas;

out vec4 Color;

vec3 CalcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffSample)
{
    vec3 lightDir = normalize(-light.dir);
    vec3 lightDirReflected = reflect(-lightDir, normal);

    float Kd = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * diffSample;
    vec3 diffuse = Kd * light.diffuse * diffSample;
    return ambient + diffuse;
}

void main()
{
    vec3 normal = normalize(vertIn.normal);
    vec3 viewDir = normalize(FrameData.viewPos - vertIn.position.xyz);

    uint textureIndex = vertIn.voxelValue;

    vec3 diffSample = vec3(0.3f);

    float alpha;
    diffSample = texture(TerrainAtlas, vec3(vertIn.uv.x, vertIn.uv.y, textureIndex)).rgb;
    alpha = 1.0f;

    vec3 directional = CalcDirectionalLight(FrameData.dirLight, normal, viewDir, diffSample);

    Color = vec4(directional, 1.0f);
}