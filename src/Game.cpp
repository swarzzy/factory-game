#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include "Player.h"
#include "Container.h"
#include "Pipe.h"
#include "Pickup.h"
#include "Belt.h"
#include "Extractor.h"
#include "EntityTraits.h"


#include "Globals.h"

#include <stdlib.h>

void RegisterBuiltInEntities(Context* context) {
    auto entityInfo = &context->entityInfo;

    { // Traits
        auto belt = EntityInfoRegisterTrait(entityInfo);
        assert(belt->id == (u32)Trait::Belt);
        belt->name = "Belt";

        auto itemExchange = EntityInfoRegisterTrait(entityInfo);
        assert(itemExchange->id == (u32)Trait::ItemExchange);
        itemExchange->name = "Item exchange";
    }

    { // Entities
        auto container = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(container->typeID == (u32)EntityType::Container);
        container->Create = CreateContainerEntity;
        container->name = "Container";
        container->DropPickup = ContainerDropPickup;
        container->Behavior = ContainerUpdateAndRender;
        container->UpdateAndRenderUI = ContainerUpdateAndRenderUI;
        container->hasUI = true;
        REGISTER_ENTITY_TRAIT(container, Container, itemExchangeTrait, Trait::ItemExchange);

        auto pipe = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(pipe->typeID == (u32)EntityType::Pipe);
        pipe->Create = CreatePipeEntity;
        pipe->name = "Pipe";
        pipe->DropPickup = PipeDropPickup;
        pipe->Behavior = PipeUpdateAndRender;
        pipe->UpdateAndRenderUI = PipeUpdateAndRenderUI;

        auto belt = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(belt->typeID == (u32)EntityType::Belt);
        belt->Create = CreateBelt;
        belt->name = "Belt";
        belt->DropPickup = BeltDropPickup;
        belt->Behavior = BeltBehavior;
        REGISTER_ENTITY_TRAIT(belt, Belt, beltTrait, Trait::Belt);

        auto extractor = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(extractor->typeID == (u32)EntityType::Extractor);
        extractor->Create = CreateExtractor;
        extractor->name = "Extractor";
        extractor->DropPickup = ExtractorDropPickup;
        extractor->Behavior = ExtractorBehavior;
        extractor->UpdateAndRenderUI = ExtractorUpdateAndRenderUI;
        REGISTER_ENTITY_TRAIT(extractor, Container, itemExchangeTrait, Trait::ItemExchange);
        //REGISTER_ENTITY_TRAIT(extractor, Extractor, testTrait, Trait::Test);

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
        pickup->Behavior = PickupUpdateAndRender;

        auto player = EntityInfoRegisterEntity(entityInfo, EntityKind::Spatial);
        assert(player->typeID == (u32)EntityType::Player);
        player->Create = CreatePlayerEntity;
        player->name = "Player";
        player->ProcessOverlap = PlayerProcessOverlap;
        player->Behavior = PlayerUpdateAndRender;
        player->UpdateAndRenderUI = PlayerUpdateAndRenderUI;
        player->hasUI = true;

        assert(entityInfo->entityTable.count == ((u32)EntityType::_Count - 1));
    }
    { // Items
        auto container = EntityInfoRegisterItem(entityInfo);
        assert(container->id == (u32)Item::Container);
        container->name = "Container";
        container->convertsToBlock = false;
        container->associatedEntityTypeID = (u32)EntityType::Container;
        container->mesh = context->containerMesh;
        container->material = &context->containerMaterial;
        container->icon = &context->containerIcon;

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
        coalOre->mesh = context->coalOreMesh;
        coalOre->material = &context->coalOreMaterial;
        coalOre->icon = &context->coalIcon;

        auto pipe = EntityInfoRegisterItem(entityInfo);
        assert(pipe->id == (u32)Item::Pipe);
        pipe->name = "Pipe";
        pipe->convertsToBlock = false;
        pipe->associatedEntityTypeID = (u32)EntityType::Pipe;
        pipe->mesh = context->pipeStraightMesh;
        pipe->material = &context->pipeMaterial;

        auto belt = EntityInfoRegisterItem(entityInfo);
        assert(belt->id == (u32)Item::Belt);
        belt->name = "Belt";
        belt->convertsToBlock = false;
        belt->associatedEntityTypeID = (u32)EntityType::Belt;
        belt->mesh = context->beltStraightMesh;
        belt->material = &context->beltMaterial;
        belt->icon = &context->beltIcon;

        auto extractor = EntityInfoRegisterItem(entityInfo);
        assert(extractor->id == (u32)Item::Extractor);
        extractor->name = "Extractor";
        extractor->convertsToBlock = false;
        extractor->associatedEntityTypeID = (u32)EntityType::Extractor;
        extractor->mesh = context->extractorMesh;
        extractor->material = &context->extractorMaterial;
        extractor->icon = &context->extractorIcon;

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

    context->coalIcon = LoadTextureFromFile("../res/coal_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->coalIcon.base);
    UploadToGPU(&context->coalIcon);

    context->containerIcon = LoadTextureFromFile("../res/chest_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->containerIcon.base);
    UploadToGPU(&context->containerIcon);

    context->beltIcon = LoadTextureFromFile("../res/belt_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->beltIcon.base);
    UploadToGPU(&context->beltIcon);

    context->extractorIcon = LoadTextureFromFile("../res/extractor_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->extractorIcon.base);
    UploadToGPU(&context->extractorIcon);

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

    context->beltStraightMesh = LoadMeshFlux("../res/belt/belt_straight.mesh");
    assert(context->beltStraightMesh);
    UploadToGPU(context->beltStraightMesh);

    context->extractorMesh = LoadMeshFlux("../res/extractor/extractor.mesh");
    assert(context->extractorMesh);
    UploadToGPU(context->extractorMesh);

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

    context->beltDiffuse = LoadTextureFromFile("../res/belt/diffuse.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->beltDiffuse.base);
    UploadToGPU(&context->beltDiffuse);
    context->beltMaterial.workflow = Material::Workflow::PBR;
    context->beltMaterial.pbr.useAlbedoMap = true;
    context->beltMaterial.pbr.albedoMap = &context->beltDiffuse;
    context->beltMaterial.pbr.roughnessValue = 1.0f;
    context->beltMaterial.pbr.metallicValue = 0.0f;

    context->extractorDiffuse = LoadTextureFromFile("../res/extractor/diffuse.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->extractorDiffuse.base);
    UploadToGPU(&context->extractorDiffuse);
    context->extractorMaterial.workflow = Material::Workflow::PBR;
    context->extractorMaterial.pbr.useAlbedoMap = true;
    context->extractorMaterial.pbr.albedoMap = &context->extractorDiffuse;
    context->extractorMaterial.pbr.roughnessValue = 1.0f;
    context->extractorMaterial.pbr.metallicValue = 0.0f;

    EntityInfoInit(&context->entityInfo);
    RegisterBuiltInEntities(context);

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
    //Entity* barrel = CreateBarrel(context, gameWorld, IV3(4, 16, 0));

    UIInit(&context->ui);

    context->camera.mode = CameraMode::Gameplay;
    GlobalPlatform.inputMode = InputMode::FreeCursor;
    context->camera.inputMode = GameInputMode::Game;

    EntityInventoryPushItem(player->toolbelt, Item::Pipe, 128);
    EntityInventoryPushItem(player->toolbelt, Item::Belt, 128);
    EntityInventoryPushItem(player->toolbelt, Item::CoalOre, 128);
    EntityInventoryPushItem(player->toolbelt, Item::Container, 128);
    EntityInventoryPushItem(player->toolbelt, Item::Stone, 128);
    EntityInventoryPushItem(player->toolbelt, Item::Extractor, 128);
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

    auto ui = &context->ui;

    if (KeyPressed(Key::E)) {
        if (UIHasOpen(ui)) {
            UICloseAll(ui);
        } else {
            UIOpenForEntity(ui, context->gameWorld.playerID);
        }
    }

    if (KeyPressed(Key::Escape)) {
        UICloseAll(ui);
    }

    UIUpdateAndRender(ui);

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

        if (hitEntity != 0) {
            if (KeyPressed(Key::R)) {
                auto entity = GetEntity(&context->playerRegion, hitEntity);
                if (entity) {
                    auto info = GetEntityInfo(entity->type);
                    EntityRotateData data {};
                    data.direction = EntityRotateData::Direction::CW;
                    info->Behavior(entity, EntityBehaviorInvoke::Rotate, &data);
                }
            }
            Entity* entity = GetEntity(&context->playerRegion, hitEntity); {
                if (entity) {
                    UIDrawEntityInfo(&context->ui, entity);
                }
            }
        } else if (hitVoxel.x != GameWorld::InvalidCoord) {
            auto voxel = GetVoxel(&context->gameWorld, hitVoxel);
            if (voxel) {
                UIDrawBlockInfo(&context->ui, voxel);
            }
        }

        if (hitVoxel.x != GameWorld::InvalidCoord) {
            if (MouseButtonPressed(MouseButton::Left)) {
                ConvertBlockToPickup(&context->gameWorld, hitVoxel);
            }
            if (MouseButtonPressed(MouseButton::Right)) {
                bool buildBlock = true;
                if (hitEntity != 0) {
                    auto entity = GetEntity(&context->playerRegion, hitEntity);
                    if (entity) {
                        auto info = GetEntityInfo(entity->type);
                        if (info->hasUI) {
                            if (UIOpenForEntity(&context->ui, hitEntity)) {
                                UIOpenForEntity(ui, context->gameWorld.playerID);
                                buildBlock = false;
                            }
                        }
                    }
                }
                if (buildBlock) {
                    auto blockToBuild = player->toolbelt->slots[player->toolbeltSelectIndex].item;
                    if (blockToBuild != Item::None) {
                        // TODO: Check is item exist in inventory before build?
                        auto result = BuildBlock(context, &context->gameWorld, hitVoxel + hitNormalInt, blockToBuild);
                        if (!CreativeModeEnabled) {
                            if (result) {
                                EntityInventoryPopItem(player->toolbelt, player->toolbeltSelectIndex);
                            }
                        }
                        //assert(result);
                    }
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

    foreach(context->gameWorld.entitiesToDelete) {
        assert(it);
        auto entity = *it;
        assert(entity->deleted);
        DeleteEntity(&context->gameWorld, entity);
    }

    BucketArrayClear(&context->gameWorld.entitiesToDelete);

    Begin(renderer, group);
    ShadowPass(renderer, group);
    MainPass(renderer, group);
    End(renderer);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
