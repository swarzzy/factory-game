#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include <stdlib.h>

BlockEntity* CreateContainer(Context* context, GameWorld* world, iv3 p) {
    auto container = AddBlockEntity(world, p);
    if (container) {
        container->type = BlockEntityType::Container;
        container->flags |= BlockEntityFlag_Collides;
        container->inventory = AllocateEntityInventory(64, 128);
        container->mesh = context->containerMesh;
        container->material = &context->containerMaterial;
    }
    return container;
}


// This proc is crazy mess for now. We need some smart algorithm for orienting pipes
void OrientPipe(Context* context, GameWorld* world, BlockEntity* pipe) {
    iv3 p = pipe->p;

    // TODO: Be carefull with pointers in blocks
    BlockEntity* westNeighbour = GetVoxel(world, p + IV3(-1, 0, 0))->entity;
    BlockEntity* eastNeighbour = GetVoxel(world, p + IV3(1, 0, 0))->entity;
    BlockEntity* northNeighbour = GetVoxel(world, p + IV3(0, 0, -1))->entity;
    BlockEntity* southNeighbour = GetVoxel(world, p + IV3(0, 0, 1))->entity;
    BlockEntity* upNeighbour = GetVoxel(world, p + IV3(0, 1, 0))->entity;
    BlockEntity* downNeighbour = GetVoxel(world, p + IV3(0, -1, 0))->entity;

    bool px = 0;
    bool nx = 0;
    bool py = 0;
    bool ny = 0;
    bool pz = 0;
    bool nz = 0;
    if (westNeighbour && westNeighbour->type == BlockEntityType::Pipe) { nx = true; }
    if (eastNeighbour && eastNeighbour->type == BlockEntityType::Pipe) { px = true; }
    if (northNeighbour && northNeighbour->type == BlockEntityType::Pipe) { nz = true; }
    if (southNeighbour && southNeighbour->type == BlockEntityType::Pipe) { pz = true; }
    if (upNeighbour && upNeighbour->type == BlockEntityType::Pipe) { py = true; }
    if (downNeighbour && downNeighbour->type == BlockEntityType::Pipe) { ny = true; }

    bool xConnected = px && nx;
    bool yConnected = py && ny;
    bool zConnected = pz && nz;

    // Check for crossing
    if (xConnected && zConnected) {
        pipe->meshRotation = V3(0.0f, 0.0f, 0.0f);
        pipe->mesh = context->pipeCrossMesh;
    } else if (xConnected && yConnected) {
        pipe->mesh = context->pipeCrossMesh;
        pipe->meshRotation = V3(90.0f, 0.0f, 0.0f);
    } else if (yConnected && zConnected) {
        pipe->mesh = context->pipeCrossMesh;
        pipe->meshRotation = V3(0.0f, 0.0f, 90.0f);
    } else {
        // Not a crossing
        if (xConnected) {
            // checking for tee
            // TODO: Check whether side pipe as a turn. If it is then place straight pipe instead of a tee
            if (ny) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(180.0f, 0.0f, 0.0f);
            } else if (py) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 0.0f);
            } else if (nz) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(90.0f, 0.0f, 0.0f);
            } else if (pz) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(-90.0f, 0.0f, 0.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 0.0f);
            }
        } else if (yConnected) {
            // checking for tee
            if (nx) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, -90.0f);
            } else if (px) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 90.0f);
            } else if (nz) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(90.0f, 0.0f, 90.0f);
            } else if (pz) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(-90.0f, 0.0f, 90.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 90.0f);
            }
        } else if (zConnected) {
            // checking for tee
            if (nx) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 90.0f, -90.0f);
            } else if (px) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 90.0f, 90.0f);
            } else if (ny) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 90.0f, 180.0f);
            } else if (py) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->meshRotation = V3(0.0f, 90.0f, 0.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->meshRotation = V3(0.0f, 90.0f, 0.0f);
            }
        } else { // Turn or staight end
            // checking for turn
            if (nx && nz) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, -270.0f, 0.0f);
            } else if (px && nz) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 0.0f);
            } else if (px && pz) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, -90.0f, 0.0f);
            } else if (nx && pz) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, -180.0f, 0.0f);
            } else if (nx && py) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(-90.0f, 180.0f, 0.0f);
            } else if (nz && py) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, -90.0f);
            } else if (px && py) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(-90.0f, 0.0f, 0.0f);
            } else if (pz && py) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(-90.0f, -90.0f, 0.0f);
            } else if (nx && ny) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(90.0f, 180.0f, 0.0f);
            } else if (nz && ny) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(0.0f, 0.0f, 90.0f);
            } else if (px && ny) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(90.0f, 0.0f, 0.0f);
            } else if (pz && ny) {
                pipe->mesh = context->pipeTurnMesh;
                pipe->meshRotation = V3(90.0f, -90.0f, 0.0f);
            } else { // straight end
                if (px && nx) {
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->meshRotation = V3(0.0f, 0.0f, 0.0f);
                } else if (py || ny) {
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->meshRotation = V3(0.0f, 0.0f, 90.0f);
                } else if (pz || nz) {
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->meshRotation = V3(0.0f, 90.0f, 0.0f);
                }
            }
        }
    }
}

BlockEntity* CreatePipe(Context* context, GameWorld* world, iv3 p) {
    auto pipe = AddBlockEntity(world, p);
    if (pipe) {
        pipe->type = BlockEntityType::Pipe;
        pipe->flags |= BlockEntityFlag_Collides;
        pipe->mesh = context->pipeStraightMesh;
        pipe->material = &context->pipeMaterial;
        //OrientPipe(context, world, pipe);
    }


    return pipe;
}

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
    InitWorld(&context->gameWorld, context, &context->chunkMesher, 234234);

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Stone, stone->bits);
    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Grass, grass->bits);
    auto coalOre = ResourceLoaderLoadImage("../res/tile_coal_ore.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::CoalOre, coalOre->bits);

    context->playerMesh = LoadMeshFlux("../res/cube.mesh");
    assert(context->playerMesh);
    UploadToGPU(context->playerMesh);

    context->coalOreMesh = LoadMeshFlux("../res/coal_ore.mesh");
    assert(context->coalOreMesh);
    UploadToGPU(context->coalOreMesh);

    context->containerMesh = LoadMeshFlux("../res/container/container.mesh");
    assert(context->containerMesh);
    UploadToGPU(context->containerMesh);

    context->pipeStraightMesh = LoadMeshFlux("../res/pipesss/pipe_straight.mesh");
    assert(context->pipeStraightMesh);
    UploadToGPU(context->pipeStraightMesh);

    context->pipeTurnMesh = LoadMeshFlux("../res/pipesss/pipe_turn.mesh");
    assert(context->pipeTurnMesh);
    UploadToGPU(context->pipeTurnMesh);

    context->pipeTeeMesh = LoadMeshFlux("../res/pipesss/pipe_Tee.mesh");
    assert(context->pipeTeeMesh);
    UploadToGPU(context->pipeTeeMesh);

    context->pipeCrossMesh = LoadMeshFlux("../res/pipesss/pipe_cross.mesh");
    assert(context->pipeCrossMesh);
    UploadToGPU(context->pipeCrossMesh);

    context->playerMaterial.workflow = Material::Workflow::PBR;
    context->playerMaterial.pbr.albedoValue = V3(0.8f, 0.0f, 0.0f);
    context->playerMaterial.pbr.roughnessValue = 0.7f;

    context->coalOreMaterial.workflow = Material::Workflow::PBR;
    context->coalOreMaterial.pbr.albedoValue = V3(0.0f, 0.0f, 0.0f);
    context->coalOreMaterial.pbr.roughnessValue = 0.95f;

    context->containerAlbedo = LoadTextureFromFile("../res/container/albedo_1024.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->containerAlbedo.base);
    UploadToGPU(&context->containerAlbedo);
    context->containerMetallic = LoadTextureFromFile("../res/container/metallic_1024.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->containerMetallic.base);
    UploadToGPU(&context->containerMetallic);
    context->containerNormal = LoadTextureFromFile("../res/container/normal_1024.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->containerNormal.base);
    UploadToGPU(&context->containerNormal);
    context->containerAO = LoadTextureFromFile("../res/container/AO_1024.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->containerAO.base);
    UploadToGPU(&context->containerAO);
    context->containerMaterial.workflow = Material::Workflow::PBR;
    context->containerMaterial.pbr.useAlbedoMap = true;
    context->containerMaterial.pbr.useMetallicMap = true;
    context->containerMaterial.pbr.useNormalMap = true;
    context->containerMaterial.pbr.useAOMap = true;
    context->containerMaterial.pbr.normalFormat = NormalFormat::DirectX;
    context->containerMaterial.pbr.albedoMap = &context->containerAlbedo;
    context->containerMaterial.pbr.roughnessValue = 0.35f;
    context->containerMaterial.pbr.metallicMap = &context->containerMetallic;
    context->containerMaterial.pbr.normalMap = &context->containerNormal;
    context->containerMaterial.pbr.AOMap = &context->containerAO;

    context->pipeAlbedo = LoadTextureFromFile("../res/pipesss/textures/albedo.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->pipeAlbedo.base);
    UploadToGPU(&context->pipeAlbedo);
    context->pipeRoughness = LoadTextureFromFile("../res/pipesss/textures/roughness.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->pipeRoughness.base);
    UploadToGPU(&context->pipeRoughness);
    context->pipeMetallic = LoadTextureFromFile("../res/pipesss/textures/metallic.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->pipeMetallic.base);
    UploadToGPU(&context->pipeMetallic);
    context->pipeNormal = LoadTextureFromFile("../res/pipesss/textures/normal.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->pipeNormal.base);
    UploadToGPU(&context->pipeNormal);
    context->pipeAO = LoadTextureFromFile("../res/pipesss/textures/AO.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->pipeAO.base);
    UploadToGPU(&context->pipeAO);
    context->pipeMaterial.workflow = Material::Workflow::PBR;
    context->pipeMaterial.pbr.useAlbedoMap = true;
    context->pipeMaterial.pbr.useMetallicMap = true;
    context->pipeMaterial.pbr.useRoughnessMap = true;
    context->pipeMaterial.pbr.useNormalMap = true;
    context->pipeMaterial.pbr.useAOMap = true;
    context->pipeMaterial.pbr.normalFormat = NormalFormat::DirectX;
    context->pipeMaterial.pbr.albedoMap = &context->pipeAlbedo;
    context->pipeMaterial.pbr.roughnessMap = &context->pipeRoughness;
    context->pipeMaterial.pbr.metallicMap = &context->pipeMetallic;
    context->pipeMaterial.pbr.normalMap = &context->pipeNormal;
    context->pipeMaterial.pbr.AOMap = &context->pipeAO;

    context->camera.targetWorldPosition = MakeWorldPos(IV3(0, 15, 0));

    context->playerRegion.world = &context->gameWorld;

    InitRegion(&context->playerRegion);
    ResizeRegion(&context->playerRegion, GameWorld::ViewDistance, context->gameArena);
    MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(context->camera.targetWorldPosition.voxel).chunk);

    auto player = AddSpatialEntity(gameWorld, IV3(0, 30, 0));
    player->type = SpatialEntityType::Player;
    player->scale = 0.95f;
    player->acceleration = 70.0f;
    player->friction = 10.0f;

    player->inventory = AllocateEntityInventory(16, 128);

    gameWorld->player.entityID = player->id;
    gameWorld->player.region = &context->playerRegion;
    gameWorld->player.height = 1.8f;
    gameWorld->player.selectedVoxel = GameWorld::InvalidPos;
    gameWorld->player.jumpAcceleration = 420.0f;
    gameWorld->player.runAcceleration = 140.0f;

    BlockEntity* container = CreateContainer(context, gameWorld, IV3(0, 7, 0));
    BlockEntity* pipe = CreatePipe(context, gameWorld, IV3(2, 7, 0));

    InitUI(&context->ui, &gameWorld->player, &context->camera);

    context->camera.mode = CameraMode::Gameplay;
    GlobalPlatform.inputMode = InputMode::FreeCursor;
    context->camera.inputMode = GameInputMode::Game;
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto renderer = context->renderer;
    auto camera = &context->camera;

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
        if (context->consoleEnabled) {
            camera->inputMode = GameInputMode::UI;
            context->console.justOpened = true;
        } else {
            // TODO: check if inventory wants to capture input
            camera->inputMode = GameInputMode::Game;
        }
    }

    if (camera->inputMode == GameInputMode::InGameUI || camera->inputMode == GameInputMode::UI) {
        GlobalPlatform.inputMode = InputMode::FreeCursor;
    } else {
        GlobalPlatform.inputMode = InputMode::CaptureCursor;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }

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

    if (KeyPressed(Key::E)) {
        // TODO: If entity inventory open then close else open player inventory
        //CloseEntityInventory(&context->ui);
        if (!context->ui.openPlayerInventory) {
            OpenPlayerInventory(&context->ui);
        } else {
            ClosePlayerInventory(&context->ui);
        }
    }

    if (KeyPressed(Key::Escape)) {
        CloseEntityInventory(&context->ui);
        context->ui.openPlayerInventory = !context->ui.openPlayerInventory;
    }

    TickUI(&context->ui, context);

    Update(&context->camera, &context->gameWorld.player, 1.0f / 60.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawDebugPerformanceCounters();


    auto group = &context->renderGroup;

    group->camera = &context->camera;

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

    auto player = GetSpatialEntity(&context->playerRegion, context->gameWorld.player.entityID);
    auto oldPlayerP = player->p;

    v3 frameAcceleration = {};

    f32 playerAcceleration;
    v3 drag = player->velocity * player->friction;

    auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
    auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
    auto y = V3(0.0f, 1.0f, 0.0f);

    if (camera->inputMode == GameInputMode::Game || camera->inputMode == GameInputMode::InGameUI) {

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
    }

    // TODO: Physically correct friction
    frameAcceleration -= drag;

    if (!context->gameWorld.player.flightMode) {
        if (camera->inputMode == GameInputMode::Game || camera->inputMode == GameInputMode::InGameUI) {
            if (KeyPressed(Key::Space) && player->grounded) {
                frameAcceleration += y * context->gameWorld.player.jumpAcceleration * (1.0f / GlobalGameDeltaTime) / 60.0f;
            }
        }

        frameAcceleration.y += -20.8f;
    }


    v3 movementDelta = 0.5f * frameAcceleration * GlobalGameDeltaTime * GlobalGameDeltaTime + player->velocity * GlobalGameDeltaTime;

    player->velocity += frameAcceleration * GlobalGameDeltaTime;
    DEBUG_OVERLAY_TRACE(player->velocity);
    bool hitGround = false;
    MoveSpatialEntity(&context->gameWorld, player, movementDelta, camera, group);

    if (ChunkPosFromWorldPos(player->p.voxel).chunk != ChunkPosFromWorldPos(oldPlayerP.voxel).chunk) {
        MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(player->p.voxel).chunk);
    }

    UpdateEntities(&context->playerRegion, group, camera, context);

    auto playerChunk = ChunkPosFromWorldPos(player->p.voxel).chunk;
    DEBUG_OVERLAY_TRACE(playerChunk);

    RegionUpdateChunkStates(&context->playerRegion);

    DrawRegion(&context->playerRegion, group, camera);

    DEBUG_OVERLAY_TRACE(context->playerRegion.chunkCount);
    DEBUG_OVERLAY_TRACE(context->playerRegion.maxChunkCount);

    if (camera->inputMode == GameInputMode::Game) {

        // TODO: Raycast
        v3 ro = camera->position;
        v3 rd = camera->mouseRay;
        f32 dist = 10.0f;

        iv3 roWorld = Offset(camera->targetWorldPosition, ro).voxel;
        iv3 rdWorld = Offset(camera->targetWorldPosition, ro + rd * dist).voxel;

        iv3 min = IV3(Min(roWorld.x, rdWorld.x), Min(roWorld.y, rdWorld.y), Min(roWorld.z, rdWorld.z)) - IV3(1);
        iv3 max = IV3(Max(roWorld.x, rdWorld.x), Max(roWorld.y, rdWorld.y), Max(roWorld.z, rdWorld.z)) + IV3(1);

        f32 tMin = F32::Max;
        iv3 hitVoxel = GameWorld::InvalidPos;
        EntityID hitEntity = EntityID {0};
        v3 hitNormal;
        iv3 hitNormalInt;

        for (i32 z = min.z; z < max.z; z++) {
            for (i32 y = min.y; y < max.y; y++) {
                for (i32 x = min.x; x < max.x; x++) {
                    const Voxel* voxel = GetVoxel(&context->gameWorld, x, y, z);
                    if (voxel && (voxel->value != VoxelValue::Empty || voxel->entity)) {
                        WorldPos voxelWorldP = MakeWorldPos(x, y, z);
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
                            if (voxel->entity) {
                                hitEntity = voxel->entity->id;
                            }
                        }
                    }
                }
            }
        }
        context->gameWorld.player.selectedVoxel = hitVoxel;
        context->gameWorld.player.selectedEntity = hitEntity;

        if (hitVoxel.x != GameWorld::InvalidCoord) {
            if (MouseButtonPressed(MouseButton::Left)) {
                ConvertVoxelToPickup(&context->gameWorld, hitVoxel);
            }
            if (MouseButtonPressed(MouseButton::Right)) {
                bool buildBlock = true;
                if (hitEntity != EntityID {0}) {
                    if (OpenInventoryForEntity(&context->ui, context, hitEntity)) {
                        buildBlock = false;
                    }
                }
                if (buildBlock) {
                    auto result = BuildBlock(context, &context->gameWorld, hitVoxel + hitNormalInt, Item::Pipe);
                    assert(result);
                }
            }

            iv3 selectedVoxelPos = context->gameWorld.player.selectedVoxel;

            v3 minP = RelativePos(camera->targetWorldPosition, MakeWorldPos(selectedVoxelPos));
            v3 maxP = RelativePos(camera->targetWorldPosition, MakeWorldPos(selectedVoxelPos));
            minP -= V3(Voxel::HalfDim);
            maxP += V3(Voxel::HalfDim);
            DrawAlignedBoxOutline(group, minP, maxP, V3(0.0f, 0.0f, 1.0f), 2.0f);
        }

    }

    // Deleting entities
    foreach(context->gameWorld.spatialEntitiesToDelete) {
        assert(it);
        auto entity = *it;
        assert(entity->deleted);
        DeleteSpatialEntity(&context->gameWorld, entity);
    }

    BucketArrayClear(&context->gameWorld.spatialEntitiesToDelete);

    foreach(context->gameWorld.blockEntitiesToDelete) {
        assert(it);
        auto entity = *it;
        assert(entity->deleted);
        DeleteBlockEntity(&context->gameWorld, entity);
    }

    BucketArrayClear(&context->gameWorld.blockEntitiesToDelete);


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
