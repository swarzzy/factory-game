#version 450
#include Common.glh
layout (location = 0) in vec2 UV;

out vec4 fragColorResult;

layout (binding = 0)uniform sampler2D ColorSourcePerceptual;

// TODO: Are these coefficients correct?
float Luma(vec3 rgb)
{
    float result = dot(rgb, vec3(0.299f, 0.587f, 0.114f));
    return result;
}

#define EDGE_MIN_THRESHOLD 0.0625f  //0.0312f
#define EDGE_MAX_THRESHOLD 0.0625f  //0.125f
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75f

float STEPS[6] = float[](1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 8.0f);
#define QUALITY(i) (STEPS[min(0, max(5, i))])

void main()
{
    vec2 invScreenSize = vec2(1.0f) / FrameData.screenSize;
    // STUDY: Dependent texture reads
    vec3 sampleCenter = texture(ColorSourcePerceptual, UV).xyz;

    float lumaCenter = Luma(sampleCenter);
    float lumaDown = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(0, -1)).xyz);
    float lumaUp = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(0, 1)).xyz);
    float lumaLeft = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(-1, 0)).xyz);
    float lumaRight = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(1, 0)).xyz);

    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    float lumaRange = lumaMax - lumaMin;

    if (lumaRange >= max(EDGE_MIN_THRESHOLD, lumaMax * EDGE_MAX_THRESHOLD))
    {
        float lumaDownLeft = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(-1, -1)).xyz);
        float lumaUpRight = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(1, 1)).xyz);
        float lumaUpLeft = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(-1, 1)).xyz);
        float lumaDownRight = Luma(textureOffset(ColorSourcePerceptual, UV, ivec2(1, -1)).xyz);

        float lumaDownUp = lumaDown + lumaUp;
        float lumaLeftRight = lumaLeft + lumaRight;
        float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
        float lumaDownCorners = lumaDownLeft + lumaDownRight;
        float lumaRightCorners = lumaDownRight + lumaUpRight;
        float lumaUpCorners = lumaUpRight + lumaUpLeft;

        float gradH = abs(-2.0f * lumaLeft + lumaLeftCorners) + abs(-2.0f * lumaCenter + lumaDownUp) * 2.0f + abs(-2.0 * lumaRight + lumaRightCorners);
        float gradV = abs(-2.0f * lumaUp + lumaUpCorners) + abs(-2.0f * lumaCenter + lumaLeftRight) * 2.0f + abs(-2.0f * lumaDown + lumaDownCorners);
        bool isHorizontal = (gradH >= gradV);

        float luma1 = isHorizontal ? lumaDown : lumaLeft;
        float luma2 = isHorizontal ? lumaUp : lumaRight;
        float grad1 = abs(luma1 - lumaCenter);
        float grad2 = abs(luma2 - lumaCenter);
        bool is1Steepest = grad1 >= grad2;
        float gradScaled = 0.25f * max(grad1, grad2);

        // TODO: dFdx() dFdy() ?
        float stepLength = isHorizontal ? invScreenSize.y : invScreenSize.x;
        float lumaLocalAvg = 0.0f;
        if (is1Steepest)
        {
            stepLength = -stepLength;
            lumaLocalAvg = 0.5f * (luma1 + lumaCenter);
        }
        else
        {
            lumaLocalAvg = 0.5f * (luma2 + lumaCenter);
        }

        vec2 currUV = UV;
        isHorizontal ? (currUV.y = currUV.y + stepLength * 0.5f) : (currUV.x = currUV.x + stepLength * 0.5f);

        vec2 offset = isHorizontal ? vec2(invScreenSize.x, 0.0f) : vec2(0.0f, invScreenSize.y);
        vec2 uv1 = currUV - offset;
        vec2 uv2 = currUV + offset;

        float lumaEnd1 = Luma(texture(ColorSourcePerceptual, uv1).xyz);
        float lumaEnd2 = Luma(texture(ColorSourcePerceptual, uv2).xyz);
        lumaEnd1 -= lumaLocalAvg;
        lumaEnd2 -= lumaLocalAvg;
        bool reached1 = abs(lumaEnd1) >= gradScaled;
        bool reached2 = abs(lumaEnd2) >= gradScaled;
        bool reachedBoth = reached1 && reached2;
        if (!reached1) uv1 -= offset;
        if (!reached2) uv2 += offset;

        if (!reachedBoth)
        {
            for (int i = 1; i < ITERATIONS; i++)
            {
                if (!reached1)
                {
                    lumaEnd1 = Luma(texture(ColorSourcePerceptual, uv1).xyz);
                    lumaEnd1 -= lumaLocalAvg;
                }
                if (!reached2)
                {
                    lumaEnd2 = Luma(texture(ColorSourcePerceptual, uv2).xyz);
                    lumaEnd2 -= lumaLocalAvg;
                }
                reached1 = abs(lumaEnd1) >= gradScaled;
                reached2 = abs(lumaEnd2) >= gradScaled;
                reachedBoth = reached1 && reached2;
                if (!reached1) uv1 -= offset * QUALITY(i);
                if (!reached2) uv2 += offset * QUALITY(i);
                if (reachedBoth) break;
            }
        }

        float dist1 = isHorizontal ? (UV.x - uv1.x) : (UV.y - uv1.y);
        float dist2 = isHorizontal ? (uv2.x - UV.x) : (uv2.y - UV.y);

        bool isDir1 = (dist1 < dist2);
        float minDist = min(dist1, dist2);
        float edgeLen = (dist1 + dist2);

        float pixelOffset = -minDist / edgeLen + 0.5f;

        bool isLumaCenterSmaller = lumaCenter < lumaLocalAvg;
        bool correctVariation = ((isDir1 ? lumaEnd1 : lumaEnd2) < 0.0f) != isLumaCenterSmaller;
        pixelOffset = correctVariation ? pixelOffset : 0.0f;

        vec2 resultUV = UV;
        isHorizontal ? (resultUV.y = resultUV.y + pixelOffset * stepLength) : (resultUV.x = resultUV.x + pixelOffset * stepLength);

        float lumaAvg = (1.0f / 12.0f) * (2.0f * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
        float subPixelOffset1 = clamp(abs(lumaAvg - lumaCenter) / lumaRange, 0.0f, 1.0f);
        float subPixelOffset2 = (-2.0f * subPixelOffset1 + 3.0f) + subPixelOffset1 * subPixelOffset1;
        float subPixelOffsetResult = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
        pixelOffset = max(pixelOffset, subPixelOffsetResult);

        fragColorResult = vec4(texture(ColorSourcePerceptual, resultUV).xyz, 1.0f);
    }
    else
    {
        fragColorResult = vec4(sampleCenter, 1.0f);
    }
    //fragColorResult = vec4(sampleCenter, 1.0f);
}