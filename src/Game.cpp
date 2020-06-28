#include "Game.h"
#include "Platform.h"
#include "DebugOverlay.h"
#include "Resource.h"

#include "entities/Player.h"
#include "entities/Container.h"
#include "entities/Pipe.h"
#include "entities/Pickup.h"
#include "entities/Belt.h"
#include "entities/Extractor.h"
#include "entities/Projectile.h"
#include "EntityTraits.h"
#include "SaveAndLoad.h"


#include "Globals.h"

#include <stdlib.h>

ItemUseResult GrenadeUse(ItemID id, Entity* user) {
    ItemUseResult result {};
    result.destroyAfterUse = true;
    if (user->type == EntityType::Player) {
        auto player = (Player*)user;
        auto throwDir = player->lookDir;
        auto throwPos = WorldPos::Offset(player->p, V3(0.0f, player->height - 0.2f, 0.0f));

        auto grenade = CreateProjectile(throwPos);
        if (grenade) {
            result.used = true;
            grenade->velocity += throwDir * 100.0f;
        }
    }
    return result;
}

void RegisterBuiltInEntities(Context* context) {
    auto entityInfo = &context->entityInfo;

    { // Traits
        auto belt = EntityInfoRegisterTrait(entityInfo);
        assert(belt->id == (u32)Trait::Belt);
        belt->name = "Belt";

        auto itemExchange = EntityInfoRegisterTrait(entityInfo);
        assert(itemExchange->id == (u32)Trait::ItemExchange);
        itemExchange->name = "Item exchange";

        auto handUsable = EntityInfoRegisterTrait(entityInfo);
        assert(handUsable->id == (u32)Trait::HandUsable);
        handUsable->name = "Hand usable";
    }

    { // Entities
        auto container = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(container->typeID == (u32)EntityType::Container);
        container->Create = CreateContainerEntity;
        container->name = "Container";
        container->DropPickup = ContainerDropPickup;
        container->Behavior = ContainerUpdateAndRender;
        container->UpdateAndRenderUI = ContainerUpdateAndRenderUI;
        container->Delete = DeleteContainer;
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
        REGISTER_ENTITY_TRAIT(belt, Belt, belt, Trait::Belt);

        auto extractor = EntityInfoRegisterEntity(entityInfo, EntityKind::Block);
        assert(extractor->typeID == (u32)EntityType::Extractor);
        extractor->Create = CreateExtractor;
        extractor->name = "Extractor";
        extractor->DropPickup = ExtractorDropPickup;
        extractor->Behavior = ExtractorBehavior;
        extractor->UpdateAndRenderUI = ExtractorUpdateAndRenderUI;
        REGISTER_ENTITY_TRAIT(extractor, Extractor, itemExchangeTrait, Trait::ItemExchange);
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

        auto projectile = EntityInfoRegisterEntity(entityInfo, EntityKind::Spatial);
        assert(projectile->typeID == (u32)EntityType::Projectile);
        projectile->Create = CreateProjectileEntity;
        projectile->name = "Projectile";
        projectile->Behavior = ProjectileUpdateAndRender;
        projectile->CollisionResponse = ProjectileCollisionResponse;

        auto player = EntityInfoRegisterEntity(entityInfo, EntityKind::Spatial);
        assert(player->typeID == (u32)EntityType::Player);
        player->Create = CreatePlayerEntity;
        player->name = "Player";
        player->ProcessOverlap = PlayerProcessOverlap;
        player->Behavior = PlayerUpdateAndRender;
        player->UpdateAndRenderUI = PlayerUpdateAndRenderUI;
        player->hasUI = true;
        player->Delete = DeletePlayer;

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
        stone->associatedBlock = BlockValue::Stone;
        stone->material = &context->stoneMaterial;
        stone->icon = &context->stoneDiffuse;

        auto grass = EntityInfoRegisterItem(entityInfo);
        assert(grass->id == (u32)Item::Grass);
        grass->name = "Grass";
        grass->convertsToBlock = true;
        grass->associatedBlock = BlockValue::Grass;
        grass->material = &context->grassMaterial;
        grass->icon = &context->grassDiffuse;

        auto coalOre = EntityInfoRegisterItem(entityInfo);
        assert(coalOre->id == (u32)Item::CoalOre);
        coalOre->name = "Coal ore";
        coalOre->convertsToBlock = true;
        coalOre->associatedBlock = BlockValue::CoalOre;
        coalOre->mesh = context->coalOreMesh;
        coalOre->material = &context->coalOreMaterial;
        coalOre->icon = &context->coalIcon;
        coalOre->beltAlign = 0.4f;
        coalOre->beltScale = 0.5f;

        auto pipe = EntityInfoRegisterItem(entityInfo);
        assert(pipe->id == (u32)Item::Pipe);
        pipe->name = "Pipe";
        pipe->convertsToBlock = false;
        pipe->associatedEntityTypeID = (u32)EntityType::Pipe;
        pipe->mesh = context->pipeStraightMesh;
        pipe->material = &context->pipeMaterial;
        pipe->beltScale = 0.35f;
        pipe->icon = &context->pipeIcon;

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

        auto water = EntityInfoRegisterItem(entityInfo);
        assert(water->id == (u32)Item::Water);
        water->name = "Water";
        water->convertsToBlock = true;
        water->associatedBlock = BlockValue::Water;
        water->material = &context->waterMaterial;

        auto coalOreBlock = EntityInfoRegisterItem(entityInfo);
        assert(coalOreBlock->id == (u32)Item::CoalOreBlock);
        coalOreBlock->name = "CoalOreBlock";
        coalOreBlock->convertsToBlock = true;
        coalOreBlock->associatedBlock = BlockValue::CoalOre;
        coalOreBlock->material = &context->coalOreBlockMaterial;
        coalOreBlock->icon = &context->coalOreBlockDiffuse;

        auto grenade = EntityInfoRegisterItem(entityInfo);
        assert(grenade->id == (u32)Item::Grenade);
        grenade->name = "Grenade";
        grenade->convertsToBlock = false;
        //grenade->associatedEntityTypeID = (u32)EntityType::Grenade;
        grenade->mesh = context->grenadeMesh;
        grenade->material = &context->grenadeMaterial;
        grenade->icon = &context->grenadeIcon;
        grenade->Use = GrenadeUse;

        assert(entityInfo->itemTable.count == ((u32)Item::_Count - 1));
    }
    { // Blocks
        auto stone = EntityInfoRegisterBlock(entityInfo);
        assert(stone->id == (u32)BlockValue::Stone);
        stone->name = "Stone";
        stone->associatedItem = (ItemID)Item::Stone;

        auto grass = EntityInfoRegisterBlock(entityInfo);
        assert(grass->id == (u32)BlockValue::Grass);
        grass->name = "Grass";
        grass->associatedItem = (ItemID)Item::Grass;

        auto coalOre = EntityInfoRegisterBlock(entityInfo);
        assert(coalOre->id == (u32)BlockValue::CoalOre);
        coalOre->name = "Coal ore";
        coalOre->DropPickup = CoalOreDropPickup;
        coalOre->associatedItem = (ItemID)Item::CoalOre;

        auto water = EntityInfoRegisterBlock(entityInfo);
        assert(water->id == (u32)BlockValue::Water);
        water->name = "Water";
        water->associatedItem = (ItemID)Item::Water;


        assert(entityInfo->blockTable.count == ((u32)BlockValue::_Count - 1));
    }
}

void FluxInit(Context* context) {
    log_print("Chunk size %llu\n", sizeof(Chunk));

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
    InitWorld(&context->gameWorld, context, &context->chunkMesher, 293847, Globals::DebugWorldName);

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetBlockTexture(context->renderer, BlockValue::Stone, stone->bits);
    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetBlockTexture(context->renderer, BlockValue::Grass, grass->bits);
    auto coalOre = ResourceLoaderLoadImage("../res/tile_coal_ore.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetBlockTexture(context->renderer, BlockValue::CoalOre, coalOre->bits);
    auto water = ResourceLoaderLoadImage("../res/tile_water.png", DynamicRange::LDR, true, 3, PlatformAlloc, GlobalLogger, GlobalLoggerData);
    SetBlockTexture(context->renderer, BlockValue::Water, water->bits);

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

    context->pipeIcon = LoadTextureFromFile("../res/pipe_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->pipeIcon.base);
    UploadToGPU(&context->pipeIcon);

    context->grenadeIcon = LoadTextureFromFile("../res/grenade_icon.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::None, DynamicRange::LDR);
    assert(context->grenadeIcon.base);
    UploadToGPU(&context->grenadeIcon);

    context->cubeMesh = LoadMeshFlux("../res/cube.mesh");
    assert(context->cubeMesh);
    UploadToGPU(context->cubeMesh);

    context->coalOreMesh = LoadMeshFlux("../res/coal_ore/coal_ore.mesh");
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

    context->grenadeMesh = LoadMeshFlux("../res/grenade/grenade.mesh");
    assert(context->grenadeMesh);
    UploadToGPU(context->grenadeMesh);

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

    context->stoneDiffuse = LoadTextureFromFile("../res/tile_stone.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->stoneDiffuse.base);
    UploadToGPU(&context->stoneDiffuse);
    context->stoneMaterial.workflow = Material::Workflow::PBR;
    context->stoneMaterial.pbr.useAlbedoMap = true;
    context->stoneMaterial.pbr.albedoMap = &context->stoneDiffuse;
    context->stoneMaterial.pbr.roughnessValue = 1.0f;
    context->stoneMaterial.pbr.metallicValue = 0.0f;

    context->grassDiffuse = LoadTextureFromFile("../res/tile_grass.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grassDiffuse.base);
    UploadToGPU(&context->grassDiffuse);
    context->grassMaterial.workflow = Material::Workflow::PBR;
    context->grassMaterial.pbr.useAlbedoMap = true;
    context->grassMaterial.pbr.albedoMap = &context->grassDiffuse;
    context->grassMaterial.pbr.roughnessValue = 1.0f;
    context->grassMaterial.pbr.metallicValue = 0.0f;

    context->coalOreBlockDiffuse = LoadTextureFromFile("../res/tile_coal_ore.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->coalOreBlockDiffuse.base);
    UploadToGPU(&context->coalOreBlockDiffuse);
    context->coalOreBlockMaterial.workflow = Material::Workflow::PBR;
    context->coalOreBlockMaterial.pbr.useAlbedoMap = true;
    context->coalOreBlockMaterial.pbr.albedoMap = &context->coalOreBlockDiffuse;
    context->coalOreBlockMaterial.pbr.roughnessValue = 1.0f;
    context->coalOreBlockMaterial.pbr.metallicValue = 0.0f;

    context->waterDiffuse = LoadTextureFromFile("../res/tile_water.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->waterDiffuse.base);
    UploadToGPU(&context->waterDiffuse);
    context->waterMaterial.workflow = Material::Workflow::PBR;
    context->waterMaterial.pbr.useAlbedoMap = true;
    context->waterMaterial.pbr.albedoMap = &context->waterDiffuse;
    context->waterMaterial.pbr.roughnessValue = 1.0f;
    context->waterMaterial.pbr.metallicValue = 0.0f;

    context->grenadeAlbedo = LoadTextureFromFile("../res/grenade/textures_256/albedo.png", TextureFormat::SRGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grenadeAlbedo.base);
    UploadToGPU(&context->grenadeAlbedo);
    context->grenadeRoughness = LoadTextureFromFile("../res/grenade/textures_256/roughness.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grenadeRoughness.base);
    UploadToGPU(&context->grenadeRoughness);
    context->grenadeMetallic = LoadTextureFromFile("../res/grenade/textures_256/metallic.png", TextureFormat::R8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grenadeMetallic.base);
    UploadToGPU(&context->grenadeMetallic);
    context->grenadeNormal = LoadTextureFromFile("../res/grenade/textures_256/normal.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grenadeNormal.base);
    UploadToGPU(&context->grenadeNormal);
    context->grenadeAO = LoadTextureFromFile("../res/grenade/textures_256/AO.png", TextureFormat::RGB8, TextureWrapMode::Default, TextureFilter::Default, DynamicRange::LDR);
    assert(context->grenadeAO.base);
    UploadToGPU(&context->grenadeAO);
    context->grenadeMaterial.workflow = Material::Workflow::PBR;
    context->grenadeMaterial.pbr.useAlbedoMap = true;
    context->grenadeMaterial.pbr.useRoughnessMap = true;
    context->grenadeMaterial.pbr.useMetallicMap = true;
    context->grenadeMaterial.pbr.useNormalMap = true;
    context->grenadeMaterial.pbr.useAOMap = true;
    context->grenadeMaterial.pbr.normalFormat = NormalFormat::DirectX;
    context->grenadeMaterial.pbr.albedoMap = &context->grenadeAlbedo;
    context->grenadeMaterial.pbr.roughnessMap = &context->grenadeRoughness;
    context->grenadeMaterial.pbr.metallicMap = &context->grenadeMetallic;
    context->grenadeMaterial.pbr.normalMap = &context->grenadeNormal;
    context->grenadeMaterial.pbr.AOMap = &context->grenadeAO;

    EntityInfoInit(&context->entityInfo);
    RegisterBuiltInEntities(context);

    context->camera.targetWorldPosition = WorldPos::Make(IV3(0, 15, 0));

    MoveRegion(&gameWorld->chunkPool.playerRegion, WorldPos::ToChunk(context->camera.targetWorldPosition.block).chunk);

#if 0
    Entity* container = CreateContainerEntity(gameWorld, WorldPos::Make(0, 16, 0));
    Entity* pipe = CreatePipeEntity(gameWorld, WorldPos::Make(2, 16, 0));
    //Entity* barrel = CreateBarrel(context, gameWorld, IV3(4, 16, 0));
#endif

    UIInit(&context->ui);

    context->camera.mode = CameraMode::Gameplay;
    PlatformSetInputMode(InputMode::FreeCursor);
    context->camera.inputMode = GameInputMode::Game;

    PlatformSetSaveThreadWork(SaveThreadWork, nullptr, 3000);
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto world = &context->gameWorld;
    while (!world->playerID) {
        auto player = (Player*)CreatePlayerEntity(world, WorldPos::Make(0, 30, 0));
        if (player) {
            player->camera =  &context->camera;
            world->playerID = player->id;
            EntityInventoryPushItem(player->toolbelt, Item::Pipe, 128);
            EntityInventoryPushItem(player->toolbelt, Item::Belt, 128);
            EntityInventoryPushItem(player->toolbelt, Item::CoalOre, 128);
            EntityInventoryPushItem(player->toolbelt, Item::Container, 128);
            EntityInventoryPushItem(player->toolbelt, Item::Stone, 128);
            EntityInventoryPushItem(player->toolbelt, Item::Extractor, 128);
            EntityInventoryPushItem(player->toolbelt, Item::Grenade, 128);
        }
        UpdateChunks(&world->chunkPool);
        return;
    }

    auto player = static_cast<Player*>(GetEntity(&context->gameWorld, context->gameWorld.playerID));
    assert(player);

    auto renderer = context->renderer;
    auto camera = &context->camera;

    if (KeyPressed(Key::F5)) {
        auto saved = SaveWorld(world);
        if (saved) {
            log_print("[Game] World %s was saved\n", world->name);
        } else {
            log_print("[Game] Failed to save world %s\n", world->name);
        }
    }

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
        PlatformSetInputMode(InputMode::FreeCursor);
    } else {
        PlatformSetInputMode(InputMode::CaptureCursor);
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
    if (renderRes.x != GetPlatform()->windowWidth ||
        renderRes.y != GetPlatform()->windowHeight) {
        ChangeRenderResolution(renderer, UV2(GetPlatform()->windowWidth, GetPlatform()->windowHeight), GetRenderSampleCount(renderer));
    }

    auto ui = &context->ui;
    auto debugUI = &context->debugUI;

    if (KeyPressed(Key::F1)) {
        Globals::ShowDebugOverlay = !Globals::ShowDebugOverlay;
    }

    if (KeyPressed(Key::F2)) {
        DebugUIToggleChunkTool(debugUI);
    }

    if (KeyPressed(Key::F3)) {
        DebugUITogglePerfCounters(debugUI);
    }


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

    UpdateChunkEntities(&world->chunkPool, group, camera);


    UpdateChunks(&world->chunkPool);

    DrawChunks(&world->chunkPool, group, camera);

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
        iv3 hitBlock = GameWorld::InvalidPos;
        EntityID hitEntity = EntityID {0};
        v3 hitNormal;
        iv3 hitNormalInt;

        for (i32 z = min.z; z < max.z; z++) {
            for (i32 y = min.y; y < max.y; y++) {
                for (i32 x = min.x; x < max.x; x++) {
                    auto block = GetBlock(&context->gameWorld, x, y, z);
                    if ((u32)(block.value) || block.entity) {
                        WorldPos voxelWorldP = WorldPos::Make(x, y, z);
                        v3 voxelRelP = WorldPos::Relative(camera->targetWorldPosition, voxelWorldP);
                        BBoxAligned voxelAABB;
                        voxelAABB.min = voxelRelP - V3(Globals::BlockHalfDim);
                        voxelAABB.max = voxelRelP + V3(Globals::BlockHalfDim);

                        //DrawAlignedBoxOutline(group, voxelAABB.min, voxelAABB.max, V3(0.0f, 1.0f, 0.0f), 2.0f);

                        auto intersection = Intersect(voxelAABB, ro, rd, 0.0f, dist); // TODO: Raycast distance
                        if (intersection.hit && intersection.t < tMin) {
                            tMin = intersection.t;
                            hitBlock = voxelWorldP.block;
                            hitNormal = intersection.normal;
                            hitNormalInt = intersection.iNormal;
                            if (block.entity) {
                                hitEntity = block.entity->id;
                            }
                        }
                    }
                }
            }
        }
        player->selectedBlock = hitBlock;
        player->selectedEntity = hitEntity;

        if (hitEntity != 0) {
            if (KeyPressed(Key::R)) {
                auto entity = GetEntity(&context->gameWorld, hitEntity);
                if (entity) {
                    auto info = GetEntityInfo(entity->type);
                    EntityRotateData data {};
                    data.direction = EntityRotateData::Direction::CW;
                    info->Behavior(entity, EntityBehaviorInvoke::Rotate, &data);
                }
            }
            Entity* entity = GetEntity(&context->gameWorld, hitEntity); {
                if (entity) {
                    UIDrawEntityInfo(&context->ui, entity);
                }
            }
        } else if (hitBlock.x != GameWorld::InvalidCoord) {
            auto block = GetBlockValue(&context->gameWorld, hitBlock);
            if (block != BlockValue::Empty) {
                UIDrawBlockInfo(&context->ui, hitBlock);
            }
        }



        bool buildBlock = false;
        bool useItem = true;

        if (hitBlock.x != GameWorld::InvalidCoord) {
            if (MouseButtonPressed(MouseButton::Left)) {
                ConvertBlockToPickup(&context->gameWorld, hitBlock);
            }
            if (MouseButtonPressed(MouseButton::Right)) {
                buildBlock = true;
                if (hitEntity != 0) {
                    auto entity = GetEntity(&context->gameWorld, hitEntity);
                    if (entity) {
                        auto info = GetEntityInfo(entity->type);
                        if (info->hasUI) {
                            if (UIOpenForEntity(&context->ui, hitEntity)) {
                                UIOpenForEntity(ui, context->gameWorld.playerID);
                                buildBlock = false;
                                useItem = false;
                            }
                        }
                    }
                }
            }

            if (buildBlock) {
                auto blockToBuild = player->toolbelt->slots[player->toolbeltSelectIndex].item;
                if (blockToBuild != Item::None) {
                    auto info = GetItemInfo(blockToBuild);
                    if (!info->Use) {
                        // TODO: Check is item exist in inventory before build?
                        auto result = BuildBlock(context, &context->gameWorld, hitBlock + hitNormalInt, blockToBuild);
                        useItem = false;
                        if (!Globals::CreativeModeEnabled) {
                            if (result) {
                                EntityInventoryPopItem(player->toolbelt, player->toolbeltSelectIndex);
                            }
                        }
                    }
                    //assert(result);
                }
            }

            iv3 selectedBlockPos = player->selectedBlock;

            v3 minP = WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(selectedBlockPos));
            v3 maxP = WorldPos::Relative(camera->targetWorldPosition, WorldPos::Make(selectedBlockPos));
            minP -= V3(Globals::BlockHalfDim);
            maxP += V3(Globals::BlockHalfDim);
            DrawAlignedBoxOutline(group, minP, maxP, V3(0.0f, 0.0f, 1.0f), 2.0f);
        }

        if (useItem && MouseButtonPressed(MouseButton::Right)) {
            auto item = player->toolbelt->slots[player->toolbeltSelectIndex].item;
            auto info = GetItemInfo(item);
            if (info->Use) {
                auto result = info->Use((ItemID)item, player);
                if (result.used && result.destroyAfterUse) {
                    EntityInventoryPopItem(player->toolbelt, player->toolbeltSelectIndex);
                }
            }
        }



    }

    ForEach(&context->gameWorld.entitiesToMove, [&](auto it) {
        auto entity = *it;
        assert(entity);
        UpdateEntityResidence(&context->gameWorld, entity);
    });

    FlatArrayClear(&context->gameWorld.entitiesToMove);

    ForEach(&context->gameWorld.entitiesToDelete, [&](Entity** it) {
        auto entity = *it;
        assert(entity);
        assert(entity->deleted);
        DeleteEntity(world, entity);
    });

    BucketArrayClear(&context->gameWorld.entitiesToDelete);


    Begin(renderer, group);
    ShadowPass(renderer, group);
    MainPass(renderer, group);
    End(renderer);

    UpdateDebugProfiler(&context->debugProfiler);
    DebugUIUpdateAndRender(debugUI);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
