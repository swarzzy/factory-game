#version 450

layout (location = 0) in vec3 UV;
out vec4 resultColor;

layout (binding = 0) uniform samplerCube uSourceCubemap;

const float PI_32 = 3.14159265358979323846f;

void main()
{
    vec3 normal = normalize(UV);
    vec3 irradance = vec3(0.0f);

    vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(up, normal);
    up = cross(normal, right);

    float sampleDelta = 0.025f;
    int sampleCount = 0;
    for (float phi = 0.0f; phi < (2.0f * PI_32); phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < (0.5f * PI_32); theta += sampleDelta)
        {
            vec3 tgSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleDir = tgSample.x * right + tgSample.y * up + tgSample.z * normal;
            irradance += texture(uSourceCubemap, sampleDir).xyz * cos(theta) * sin(theta);
            sampleCount++;
        }
    }

    irradance = PI_32 * irradance * (1.0f / float(sampleCount));
    resultColor = vec4(irradance, 1.0f);
}