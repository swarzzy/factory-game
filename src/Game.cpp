#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include "Region.h"

#include <stdlib.h>

#if 0
struct Noise {
    static const u32 BitShift = 4;
    static const u32 BitMask = (1 << BitShift) - 1;
    static const u32 Size = 1 << BitShift;
    f32 values[Size];
};

Noise CreateNoise(u32 seed) {
    Noise noise;
    srand(seed);
    for (u32x i = 0; i < Noise::Size; i++) {
        noise.values[i] = rand() / (f32)RAND_MAX;
    }
    return noise;
}

f32 Sample(Noise* noise, float x) {
    i32 xi = (i32)x;
    i32 p = xi & Noise::BitMask;
    i32 pn = (p == (Noise::Size - 1)) ? 0 : p + 1;
    f32 t = x - xi;
    t = SmoothStep(0.0f, 1.0f, t);
    f32 result = Lerp(noise->values[p], noise->values[pn], t);
    return result;
}
#endif

void FluxInit(Context* context) {
    context->skybox = LoadCubemapLDR("../res/skybox/sky_back.png", "../res/skybox/sky_down.png", "../res/skybox/sky_front.png", "../res/skybox/sky_left.png", "../res/skybox/sky_right.png", "../res/skybox/sky_up.png");
    UploadToGPU(&context->skybox);
    context->hdrMap = LoadCubemapHDR("../res/desert_sky/nz.hdr", "../res/desert_sky/ny.hdr", "../res/desert_sky/pz.hdr", "../res/desert_sky/nx.hdr", "../res/desert_sky/px.hdr", "../res/desert_sky/py.hdr");
    UploadToGPU(&context->hdrMap);
    context->irradanceMap = MakeEmptyCubemap(64, 64, TextureFormat::RGB16F, TextureFilter::Bilinear, TextureWrapMode::ClampToEdge, false);
    UploadToGPU(&context->irradanceMap);
    context->enviromentMap = MakeEmptyCubemap(256, 256, TextureFormat::RGB16F, TextureFilter::Trilinear, TextureWrapMode::ClampToEdge, true);
    UploadToGPU(&context->enviromentMap);

    context->renderGroup.drawSkybox = true;
    context->renderGroup.skyboxHandle = context->enviromentMap.gpuHandle;
    context->renderGroup.irradanceMapHandle = context->irradanceMap.gpuHandle;
    context->renderGroup.envMapHandle = context->enviromentMap.gpuHandle;

    GenIrradanceMap(context->renderer, &context->irradanceMap, context->hdrMap.gpuHandle);
    GenEnvPrefiliteredMap(context->renderer, &context->enviromentMap, context->hdrMap.gpuHandle, 6);

    auto gameWorld = &context->gameWorld;
    gameWorld->Init(&context->chunkMesher, 234234);

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc);
    SetVoxelTexture(context->renderer, VoxelValue::Stone, stone->bits);
#if 0
    u32 size = 3 * 256 * 256;
    auto bitmap = (byte*)PlatformAlloc(size);
    auto noise = CreateNoise2D(2342);
    for (u32 y = 0; y < 256; y++) {
        for (u32 x = 0; x < 256; x++) {
            auto ptr = bitmap + (y * 256 + x) * 3;
            byte n = (byte)(Sample(&noise, (f32)(x / 20.0f), (f32)(y / 20.0f)) * 255.0f);
            ptr[0] = n;
            ptr[1] = n;
            ptr[2] = n;
        }
    }
#endif
    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc);
    SetVoxelTexture(context->renderer, VoxelValue::Grass, grass->bits);
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto renderer = context->renderer;

    i32 rendererSampleCount = GetRenderSampleCount(renderer);
    DEBUG_OVERLAY_SLIDER(rendererSampleCount, 0, GetRenderMaxSampleCount(renderer));
    if (rendererSampleCount != GetRenderSampleCount(renderer)) {
        ChangeRenderResolution(renderer, GetRenderResolution(renderer), rendererSampleCount);
    }

    auto renderRes = GetRenderResolution(renderer);
    if (renderRes.x != GlobalPlatform.windowWidth ||
        renderRes.y != GlobalPlatform.windowHeight) {
        ChangeRenderResolution(renderer, UV2(GlobalPlatform.windowWidth, GlobalPlatform.windowHeight), GetRenderSampleCount(renderer));
    }


    Update(&context->camera, 1.0f / 60.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //DrawDebugPerformanceCounters();


    auto group = &context->renderGroup;

    group->camera = &context->camera;
    auto camera = &context->camera;

    DirectionalLight light = {};
    light.dir = Normalize(V3(0.3f, -1.0f, -0.95f));
    light.from = V3(4.0f, 200.0f, 0.0f);
    light.ambient = V3(0.3f);
    light.diffuse = V3(1.0f);
    light.specular = V3(1.0f);
    RenderCommandSetDirLight lightCommand = { light };
    Push(group, &lightCommand);


    DEBUG_OVERLAY_TRACE(context->gameWorld.mesher->freeBlockCount);
    DEBUG_OVERLAY_TRACE(context->gameWorld.mesher->totalBlockCount);

    auto region = BeginRegion(&context->gameWorld, camera->targetWorldPosition.voxel, GameWorld::ViewDistance);
    region.debugShowBoundaries = true;
    region.debugRender = true;
    DEBUG_OVERLAY_TRACE(camera->targetWorldPosition.voxel);
    DEBUG_OVERLAY_TRACE(camera->targetWorldPosition.offset);
    DrawRegion(&region, group, camera);

    Begin(renderer, group);
    ShadowPass(renderer, group);
    MainPass(renderer, group);
    End(renderer);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
