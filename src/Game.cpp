#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include "Player.h"
#include "Container.h"
#include "Pipe.h"
#include "Pickup.h"

#include <stdlib.h>

Entity* CreateBarrel(Context* context, GameWorld* world, iv3 p) {
    auto barrel = AddBlockEntity<BlockEntity>(world, p);
    if (barrel) {
        barrel->type = EntityType::Barrel;
        barrel->flags |= EntityFlag_Collides;
        barrel->mesh = context->barrelMesh;
        barrel->material = &context->barrelMaterial;
    }


    return barrel;
}

void RegisterBuiltInEntities(Context* context) {
    auto entityInfo = &context->entityInfo;

    { // Entities
        auto container = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(container->typeID == (u32)EntityType::Container);
        container->Create = CreateContainerEntity;
        container->name = "Container";
        container->DropPickup = ContainerDropPickup;
        container->UpdateAndRender = ContainerUpdateAndRender;

        auto pipe = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(pipe->typeID == (u32)EntityType::Pipe);
        pipe->Create = CreatePipeEntity;
        pipe->name = "Pipe";
        pipe->DropPickup = PipeDropPickup;
        pipe->UpdateAndRender = PipeUpdateAndRender;

        auto barrel = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(barrel->typeID == (u32)EntityType::Barrel);
        barrel->name = "Barrel";

        auto tank = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(tank->typeID == (u32)EntityType::Tank);
        tank->name = "Tank";

        auto pickup = EntityInfoRegisterEntity(entityInfo, EntityKind::Spatial);
        assert(pickup->typeID == (u32)EntityType::Pickup);
        pickup->Create = CreatePickupEntity;
        pickup->name = "Pickup";
        pickup->UpdateAndRender = PickupUpdateAndRender;

        auto player = EntityInfoRegisterEntity(entityInfo, EntityKind::Spatial);
        assert(player->typeID == (u32)EntityType::Player);
        player->Create = CreatePlayerEntity;
        player->name = "Player";
        player->ProcessOverlap = PlayerProcessOverlap;
        player->UpdateAndRender = PlayerUpdateAndRender;

        assert(entityInfo->entityTable.count == ((u32)EntityType::_Count - 1));
    }
    { // Items
        auto container = EntityInfoRegisterItem(entityInfo);
        assert(container->id == (u32)Item::Container);
        container->name = "Container";
        container->convertsToBlock = false;
        container->associatedEntityTypeID = (u32)EntityType::Container;

        auto stone = EntityInfoRegisterItem(entityInfo);
        assert(stone->id == (u32)Item::Stone);
        stone->name = "Stone";
        stone->convertsToBlock = true;
        stone->associatedBlock = VoxelValue::Stone;

        auto grass = EntityInfoRegisterItem(entityInfo);
        assert(grass->id == (u32)Item::Grass);
        grass->name = "Grass";
        grass->convertsToBlock = true;
        grass->associatedBlock = VoxelValue::Grass;

        auto coalOre = EntityInfoRegisterItem(entityInfo);
        assert(coalOre->id == (u32)Item::CoalOre);
        coalOre->name = "Coal ore";
        coalOre->convertsToBlock = true;
        coalOre->associatedBlock = VoxelValue::CoalOre;

        auto pipe = EntityInfoRegisterItem(entityInfo);
        assert(pipe->id == (u32)Item::Pipe);
        pipe->name = "Pipe";
        pipe->convertsToBlock = false;
        pipe->associatedEntityTypeID = (u32)EntityType::Pipe;

        auto barrel = EntityInfoRegisterItem(entityInfo);
        assert(barrel->id == (u32)Item::Barrel);
        barrel->name = "Barrel";
        barrel->convertsToBlock = false;
        barrel->associatedEntityTypeID = (u32)EntityType::Barrel;

        auto tank = EntityInfoRegisterItem(entityInfo);
        assert(tank->id == (u32)Item::Tank);
        tank->name = "Tank";
        tank->convertsToBlock = false;
        tank->associatedEntityTypeID = (u32)EntityType::Tank;

        assert(entityInfo->itemTable.count == ((u32)Item::_Count - 1));
    }
    { // Blocks
        auto stone = EntityInfoRegisterBlock(entityInfo);
        assert(stone->id == (u32)VoxelValue::Stone);
        stone->name = "Stone";

        auto grass = EntityInfoRegisterBlock(entityInfo);
        assert(grass->id == (u32)VoxelValue::Grass);
        grass->name = "Grass";

        auto coalOre = EntityInfoRegisterBlock(entityInfo);
        assert(coalOre->id == (u32)VoxelValue::CoalOre);
        coalOre->name = "Coal ore";
        coalOre->DropPickup = CoalOreDropPickup;

        auto water = EntityInfoRegisterBlock(entityInfo);
        assert(water->id == (u32)VoxelValue::Water);
        water->name = "Water";

        assert(entityInfo->blockTable.count == ((u32)VoxelValue::_Count - 1));
    }
}

void FluxInit(Context* context) {
    LogMessage(&context->logger, "Logger test %s", "message\n");
    LogMessage(&context->logger, "Logger prints string");

    EntityInfoInit(&context->entityInfo);
    RegisterBuiltInEntities(context);

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
    InitWorld(&context->gameWorld, context, &context->chunkMesher, 293847);

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Stone, stone->bits);
    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Grass, grass->bits);
    auto coalOre = ResourceLoaderLoadImage("../res/tile_coal_ore.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::CoalOre, coalOre->bits);
    auto water = ResourceLoaderLoadImage("../res/tile_water.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetVoxelTexture(context->renderer, VoxelValue::Water, water->bits);

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

    context->barrelMesh = LoadMeshFlux("../res/barrel/barrel.mesh");
    assert(context->barrelMesh);
    UploadToGPU(context->barrelMesh);

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

    context->barrelAlbedo = LoadTextureFromFile("../res/barrel/albedo.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->barrelAlbedo.base);
    UploadToGPU(&context->barrelAlbedo);
    context->barrelRoughness = LoadTextureFromFile("../res/barrel/roughness.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->barrelRoughness.base);
    UploadToGPU(&context->barrelRoughness);
    context->barrelNormal = LoadTextureFromFile("../res/barrel/normal.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->barrelNormal.base);
    UploadToGPU(&context->barrelNormal);
    context->barrelAO = LoadTextureFromFile("../res/barrel/AO.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->barrelAO.base);
    UploadToGPU(&context->barrelAO);
    context->barrelMaterial.workflow = Material::Workflow::PBR;
    context->barrelMaterial.pbr.useAlbedoMap = true;
    context->barrelMaterial.pbr.useRoughnessMap = true;
    context->barrelMaterial.pbr.useNormalMap = true;
    context->barrelMaterial.pbr.useAOMap = true;
    context->barrelMaterial.pbr.normalFormat = NormalFormat::DirectX;
    context->barrelMaterial.pbr.albedoMap = &context->barrelAlbedo;
    context->barrelMaterial.pbr.roughnessMap = &context->barrelRoughness;
    context->barrelMaterial.pbr.metallicValue = 0.0f;
    context->barrelMaterial.pbr.normalMap = &context->barrelNormal;
    context->barrelMaterial.pbr.AOMap = &context->barrelAO;

    context->camera.targetWorldPosition = WorldPos::Make(IV3(0, 15, 0));

    context->playerRegion.world = &context->gameWorld;

    InitRegion(&context->playerRegion);
    ResizeRegion(&context->playerRegion, GameWorld::ViewDistance, context->gameArena);
    MoveRegion(&context->playerRegion, WorldPos::ToChunk(context->camera.targetWorldPosition.block).chunk);

    auto player = (Player*)CreatePlayerEntity(gameWorld, WorldPos::Make(0, 30, 0));
    player->camera =  &context->camera;
    player->region =  &context->playerRegion;

    gameWorld->playerID = player->id;

    Entity* container = CreateContainerEntity(gameWorld, WorldPos::Make(0, 16, 0));
    Entity* pipe = CreatePipeEntity(gameWorld, WorldPos::Make(2, 16, 0));
    Entity* barrel = CreateBarrel(context, gameWorld, IV3(4, 16, 0));

    InitUI(&context->ui, static_cast<Player*>(player), &context->camera);

    context->camera.mode = CameraMode::Gameplay;
    GlobalPlatform.inputMode = InputMode::FreeCursor;
    context->camera.inputMode = GameInputMode::Game;
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto player = static_cast<Player*>(GetEntity(&context->playerRegion, context->gameWorld.playerID));
    assert(player);

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

    Update(&context->camera, player, 1.0f / 60.0f);
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

    UpdateEntities(&context->playerRegion, group, camera, context);


    RegionUpdateChunkStates(&context->playerRegion);

    DrawRegion(&context->playerRegion, group, camera);

    DEBUG_OVERLAY_TRACE(context->playerRegion.chunkCount);
    DEBUG_OVERLAY_TRACE(context->playerRegion.maxChunkCount);

    if (camera->inputMode == GameInputMode::Game) {

        // TODO: Raycast
        v3 ro = camera->position;
        v3 rd = camera->mouseRay;
        f32 dist = 10.0f;

        iv3 roWorld = WorldPos::Offset(camera->targetWorldPosition, ro).block;
        iv3 rdWorld = WorldPos::Offset(camera->targetWorldPosition, ro + rd * dist).block;

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
                        WorldPos voxelWorldP = WorldPos::Make(x, y, z);
                        v3 voxelRelP = WorldPos::Relative(camera->targetWorldPosition, voxelWorldP);
                        BBoxAligned voxelAABB;
                        voxelAABB.min = voxelRelP - V3(Voxel::HalfDim);
                        voxelAABB.max = voxelRelP + V3(Voxel::HalfDim);

                        //DrawAlignedBoxOutline(group, voxelAABB.min, voxelAABB.max, V3(0.0f, 1.0f, 0.0f), 2.0f);

                        auto intersection = Intersect(voxelAABB, ro, rd, 0.0f, dist); // TODO: Raycast distance
                        if (intersection.hit && intersection.t < tMin) {
                            tMin = intersection.t;
                            hitVoxel = voxelWorldP.block;
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
        player->selectedVoxel = hitVoxel;
        player->selectedEntity = hitEntity;

        if (hitEntity != EntityID {0}) {
            Entity* entity = GetEntity(&context->playerRegion, hitEntity); {
                if (entity) {
                    DrawEntityInfo(&context->ui, entity);
                }
            }
        }

        if (hitVoxel.x != GameWorld::InvalidCoord) {
            if (MouseButtonPressed(MouseButton::Left)) {
                ConvertBlockToPickup(&context->gameWorld, hitVoxel);
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

            iv3 selectedVoxelPos = player->selectedVoxel;

            v3 minP = WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(selectedVoxelPos));
            v3 maxP = WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(selectedVoxelPos));
            minP -= V3(Voxel::HalfDim);
            maxP += V3(Voxel::HalfDim);
            DrawAlignedBoxOutline(group, minP, maxP, V3(0.0f, 0.0f, 1.0f), 2.0f);
        }

    }

    foreach(context->gameWorld.blockEntitiesToDelete) {
        assert(it);
        auto entity = *it;
        assert(entity->deleted);
        if (entity->kind == EntityKind::Spatial) {
            DeleteSpatialEntity(&context->gameWorld, static_cast<SpatialEntity*>(entity));
        } else {
            DeleteBlockEntity(&context->gameWorld, static_cast<BlockEntity*>(entity));
        }
    }

    BucketArrayClear(&context->gameWorld.blockEntitiesToDelete);

    Begin(renderer, group);
    ShadowPass(renderer, group);
    MainPass(renderer, group);
    End(renderer);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
