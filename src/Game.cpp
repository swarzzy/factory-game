#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include <stdlib.h>

void FluxInit(Context* context) {
    LogMessage(&context->logger, "Logger test %s", "message\n");
    LogMessage(&context->logger, "Logger prints string");

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

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Stone, stone->bits);
    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Grass, grass->bits);

    context->playerMesh = LoadMeshFlux("../res/cube.mesh");
    assert(context->playerMesh);
    UploadToGPU(context->playerMesh);

    context->playerMaterial.workflow = Material::Workflow::PBR;
    context->playerMaterial.pbr.albedoValue = V3(0.8f, 0.0f, 0.0f);
    context->playerMaterial.pbr.roughnessValue = 0.7f;

    context->camera.targetWorldPosition = WorldPos::Make(IV3(0, 15, 0));

    context->playerRegion.world = &context->gameWorld;
    ResizeRegion(&context->playerRegion, GameWorld::ViewDistance, context->gameArena);
    MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(context->camera.targetWorldPosition.voxel).chunk);
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto renderer = context->renderer;

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
        if (context->consoleEnabled) {
            context->console.justOpened = true;
        }
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }


    context->camera.mode = CameraMode::DebugFollowing;
    GlobalPlatform.inputMode = InputMode::FreeCursor;


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


    Update(&context->camera, &context->gameWorld.player, 1.0f / 60.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawDebugPerformanceCounters();


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

    auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
    auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
    auto y = V3(0.0f, 1.0f, 0.0f);

    v3 frameAcceleration = {};

    if (KeyHeld(Key::W)) {
        frameAcceleration -= z;
    }
    if (KeyHeld(Key::S)) {
        frameAcceleration += z;
    }
    if (KeyHeld(Key::A)) {
        frameAcceleration -= x;
    }
    if (KeyHeld(Key::D)) {
        frameAcceleration += x;
    }

    if ((KeyHeld(Key::Space))) {
        //frameAcceleration += y;
    }

    if (KeyPressed(Key::Y)) {
        context->gameWorld.player.flightMode = !context->gameWorld.player.flightMode;
    }

    auto player = &context->gameWorld.playerEntity;
    auto oldPlayerP = player->p;

    f32 playerAcceleration;
    v3 drag = player->velocity * player->friction;

    if (context->gameWorld.player.flightMode) {
        if (KeyHeld(Key::Space)) {
            frameAcceleration += y;
        }
        if (KeyHeld(Key::Ctrl)) {
            frameAcceleration -= y;
        }
    } else {
        drag.y = 0.0f;
    }

    if ((KeyHeld(Key::Shift))) {
        playerAcceleration = context->gameWorld.player.runAcceleration;
    } else {
        playerAcceleration = player->acceleration;
    }

    frameAcceleration *= playerAcceleration;
    // TODO: Physically correct friction
    frameAcceleration -= drag;

    if (!context->gameWorld.player.flightMode) {
        if (KeyPressed(Key::Space) && player->grounded) {
            frameAcceleration += y * context->gameWorld.player.jumpAcceleration * (1.0f / GlobalGameDeltaTime) / 60.0f;
        }

        frameAcceleration.y += -20.8f;
    }

    v3 movementDelta = 0.5f * frameAcceleration * GlobalGameDeltaTime * GlobalGameDeltaTime + player->velocity * GlobalGameDeltaTime;

    player->velocity += frameAcceleration * GlobalGameDeltaTime;
    DEBUG_OVERLAY_TRACE(player->velocity);
    bool hitGround = false;
    auto newP = DoMovement(&context->gameWorld, player->p, movementDelta, &player->velocity, &hitGround,  camera, group);
    if (hitGround) {
        player->grounded = true;
        player->velocity.y = 0.0f;
    } else {
        player->grounded = false;
    }

    player->p = newP;

    if (ChunkPosFromWorldPos(newP.voxel).chunk != ChunkPosFromWorldPos(oldPlayerP.voxel).chunk) {
        MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(newP.voxel).chunk);
    }

    RegionUpdateChunkStates(&context->playerRegion);

    DrawRegion(&context->playerRegion, group, camera);

    DEBUG_OVERLAY_TRACE(context->playerRegion.chunkCount);
    DEBUG_OVERLAY_TRACE(context->playerRegion.maxChunkCount);

    v3 ro = camera->position;
    v3 rd = camera->mouseRay;
    f32 dist = 10.0f;

    iv3 roWorld = Offset(camera->targetWorldPosition, ro).voxel;
    iv3 rdWorld = Offset(camera->targetWorldPosition, ro + rd * dist).voxel;

    iv3 min = IV3(Min(roWorld.x, rdWorld.x), Min(roWorld.y, rdWorld.y), Min(roWorld.z, rdWorld.z)) - IV3(1);
    iv3 max = IV3(Max(roWorld.x, rdWorld.x), Max(roWorld.y, rdWorld.y), Max(roWorld.z, rdWorld.z)) + IV3(1);

    f32 tMin = F32::Max;
    iv3 hitVoxel = GameWorld::InvalidPos;
    v3 hitNormal;
    iv3 hitNormalInt;

    for (i32 z = min.z; z < max.z; z++) {
        for (i32 y = min.y; y < max.y; y++) {
            for (i32 x = min.x; x < max.x; x++) {
                const Voxel* voxel = GetVoxel(&context->gameWorld, x, y, z);
                if (voxel && voxel->value != VoxelValue::Empty) {
                    WorldPos voxelWorldP = WorldPos::Make(x, y, z);
                    v3 voxelRelP = RelativePos(camera->targetWorldPosition, voxelWorldP);
                    BBoxAligned voxelAABB;
                    voxelAABB.min = voxelRelP - V3(Voxel::HalfDim);
                    voxelAABB.max = voxelRelP + V3(Voxel::HalfDim);

                    //DrawAlignedBoxOutline(group, voxelAABB.min, voxelAABB.max, V3(0.0f, 1.0f, 0.0f), 2.0f);

                    auto intersection = Intersect(voxelAABB, ro, rd, 0.0f, dist); // TODO: Raycast distance
                    if (intersection.hit && intersection.t < tMin) {
                        tMin = intersection.t;
                        hitVoxel = voxelWorldP.voxel;
                        hitNormal = intersection.normal;
                        hitNormalInt = intersection.iNormal;
                    }
                }
            }
        }
    }
    context->gameWorld.player.selectedVoxel = hitVoxel;

    if (hitVoxel.x != GameWorld::InvalidCoord) {
        if (MouseButtonPressed(MouseButton::Left)) {
            auto chunkPos = ChunkPosFromWorldPos(hitVoxel);
            auto chunk = GetChunk(&context->gameWorld, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
            auto voxel = GetVoxelForModification(chunk, chunkPos.voxel.x, chunkPos.voxel.y, chunkPos.voxel.z);
            voxel->value = VoxelValue::Empty;
        }
        if (MouseButtonPressed(MouseButton::Right)) {
            auto chunkPos = ChunkPosFromWorldPos(hitVoxel + hitNormalInt);
            auto chunk = GetChunk(&context->gameWorld, chunkPos.chunk.x, chunkPos.chunk.y, chunkPos.chunk.z);
            auto voxel = GetVoxelForModification(chunk, chunkPos.voxel.x, chunkPos.voxel.y, chunkPos.voxel.z);
            voxel->value = VoxelValue::Stone;
        }

        iv3 selectedVoxelPos = context->gameWorld.player.selectedVoxel;

        v3 minP = RelativePos(camera->targetWorldPosition, WorldPos::Make(selectedVoxelPos));
        v3 maxP = RelativePos(camera->targetWorldPosition, WorldPos::Make(selectedVoxelPos));
        minP -= V3(Voxel::HalfDim);
        maxP += V3(Voxel::HalfDim);
        DrawAlignedBoxOutline(group, minP, maxP, V3(0.0f, 0.0f, 1.0f), 2.0f);
    }



    DEBUG_OVERLAY_TRACE(context->gameWorld.player.selectedVoxel);

    if (camera->mode != CameraMode::Gameplay) {
        RenderCommandDrawMesh command {};
        command.transform = Translate(RelativePos(camera->targetWorldPosition, player->p));
        command.mesh = context->playerMesh;
        command.material = &context->playerMaterial;
        Push(group, &command);
    }

    Begin(renderer, group);
    ShadowPass(renderer, group);
    MainPass(renderer, group);
    End(renderer);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
