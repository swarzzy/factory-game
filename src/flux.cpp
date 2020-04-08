#include "flux.h"
#include "flux_platform.h"
#include "flux_debug_overlay.h"
#include "flux_resource_manager.h"

#include "Region.h"

void Work(void* data, u32 id) {
    //while (true) {
        printf("Thread %d does some work\n", (int)id);
        //}
}


void FluxInit(Context* context) {
    AssetManager::Init(&context->assetManager, context->renderer);

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

    auto defaultWorld = LoadWorldFromDisc(&context->assetManager, DefaultWorldW);
    if (defaultWorld) {
        wprintf(L"[Flux] Loaded default world: %ls", DefaultWorldW);
        context->world = defaultWorld;
    } else {
        context->world = (World*)PlatformAlloc(sizeof(World));
        *context->world = {};
        strcpy_s(context->world->name, array_count(context->world->name), DefaultWorld);
    }
    auto gameWorld = &context->gameWorld;
    gameWorld->mesher = &context->chunkMesher;

    auto stone = ResourceLoaderLoadImage("../res/tile_stone.png", DynamicRange::LDR, true, 3, PlatformAlloc);
    SetVoxelTexture(context->renderer, VoxelValue::Stone, stone->bits);

    auto grass = ResourceLoaderLoadImage("../res/tile_grass.png", DynamicRange::LDR, true, 3, PlatformAlloc);
    SetVoxelTexture(context->renderer, VoxelValue::Grass, grass->bits);

    Material stoneMaterial = {};
    stoneMaterial.workflow = Material::PBRMetallic;
    stoneMaterial.pbrMetallic.albedoValue = V3(0.3f);
    stoneMaterial.pbrMetallic.roughnessValue = 0.85f;
    stoneMaterial.pbrMetallic.metallicValue = 0.0f;

    context->stoneMaterial = stoneMaterial;

    auto addCubeResult = AddMesh(&context->assetManager, "../res/cube.mesh", MeshFileFormat::Flux);
    assert(addCubeResult.status = AddAssetResult::Ok);
    context->cubeMeshID = addCubeResult.id;
}

void FluxReload(Context* context) {
}

void FluxUpdate(Context* context) {
    auto ui = &context->ui;
    auto world = context->world;
    auto renderer = context->renderer;
    auto assetManager = &context->assetManager;

    i32 rendererSampleCount = GetRenderSampleCount(renderer);
    DEBUG_OVERLAY_SLIDER(rendererSampleCount, 0, GetRenderMaxSampleCount(renderer));
    if (rendererSampleCount != GetRenderSampleCount(renderer)) {
        ChangeRenderResolution(renderer, GetRenderResolution(renderer), rendererSampleCount);
    }

    DEBUG_OVERLAY_TRACE(assetManager->assetQueueUsage);
    CompletePendingLoads(assetManager);

    auto renderRes = GetRenderResolution(renderer);
    if (renderRes.x != GlobalPlatform.windowWidth ||
        renderRes.y != GlobalPlatform.windowHeight) {
        ChangeRenderResolution(renderer, UV2(GlobalPlatform.windowWidth, GlobalPlatform.windowHeight), GetRenderSampleCount(renderer));
    }


    Update(&context->camera, 1.0f / 60.0f);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //DrawDebugPerformanceCounters();

    UpdateUi(context);

    if (ui->wantsLoadLoadFrom) {
        auto newWorld = LoadWorldFrom(ui, assetManager);
        if (newWorld) {
            // TODO: Loading and unloading levels
            // TODO: Get rid if this random deallocation confusion
            Drop(&context->world->entityTable);
            PlatformFree(context->world);
            ui->selectedEntity = 0;
            context->world = newWorld;
            world = newWorld;
        }
    }

    if (ui->wantsSaveAs) {
        SaveWorldAs(ui, world, assetManager);
    }

    if (ui->wantsAddEntity) {
        ui->wantsAddEntity = false;
        auto entity = AddEntity(world);
        entity->mesh = GetID(&assetManager->nameTable, "../res/meshes/sphere.aab");
        // TODO: Assign material
        entity->material = {};
    }

    // TODO: Factor this out
    if (ui->wantsSave || (KeyHeld(Key::Ctrl) && KeyPressed(Key::S))) {
        ui->wantsSave = false;
        if (world->name[0]) {
            wchar_t buffer[array_count(world->name)];
            mbstowcs(buffer, world->name, array_count(world->name));
            // Back-up previous save
            if (PlatformDebugGetFileSize(buffer)) {
                wchar_t backupBuffer[array_count(world->name) + sizeof(L".backup")];
                wcscpy(backupBuffer, buffer);
                wcscat(backupBuffer, L".backup");
                PlatformDebugCopyFile(buffer, backupBuffer, true);
            }
            if (SaveToDisk(assetManager, world, buffer)) {
                printf("[Info] World %s was saved successfully\n", world->name);
            } else {
                printf("[Error] Failed to save world %s\n", world->name);
            }
        }
    }

    auto entity = GetEntity(world, 11);
    if (entity) {
        entity->rotationAngles.y += GlobalGameDeltaTime * 50.0f;
        if (entity->rotationAngles.y >= 360.0f) {
            entity->rotationAngles.y = 0.0f;
        }
    }

    Update(world);

    auto group = &context->renderGroup;

    group->camera = &context->camera;
    auto camera = &context->camera;

    DirectionalLight light = {};
    light.dir = Normalize(V3(0.3f, -1.0f, -0.95f));
    light.from = V3(4.0f, 200.0f, 0.0f);
    light.ambient = V3(0.3f);
    light.diffuse = V3(5.8f);
    light.specular = V3(1.0f);
    RenderCommandSetDirLight lightCommand = { light };
    Push(group, &lightCommand);

    if (ui->selectedEntity) {
        auto entity = Get(&world->entityTable, &ui->selectedEntity);
        assert(entity);
        auto mesh = GetMesh(assetManager, entity->mesh);
        if (mesh) {
            auto aabb = mesh->aabb;
            aabb.min = (entity->transform * V4(aabb.min, 1.0f)).xyz;
            aabb.max = (entity->transform * V4(aabb.max, 1.0f)).xyz;
            DrawAlignedBoxOutline(&context->renderGroup, aabb.min, aabb.max, V3(0.0f, 0.0f, 1.0f), 2.0f);
        }
    }

#if 0
    RenderCommandDrawMesh gizmosCommand = {};
    gizmosCommand.transform = Scale(V3(0.1f, 0.1f, 0.1f));
    gizmosCommand.mesh = context->meshes[(u32)EntityMesh::Gizmos];
    gizmosCommand.material = context->materials[(u32)EntityMaterial::Checkerboard];
    Push(group, &gizmosCommand);
#endif

    for (Entity& entity : context->world->entityTable) {
        assert(entity.id);
        if (entity.id) {
            auto mesh = GetMesh(assetManager, entity.mesh);
            if (mesh) {
                RenderCommandDrawMesh command = {};
                command.transform = entity.transform;
                command.meshID = entity.mesh;
                command.material = entity.material;
                Push(group, &command);
                if (context->ui.showBoundingVolumes) {
                    auto aabb = mesh->aabb;
                    aabb.min = (entity.transform * V4(aabb.min, 1.0f)).xyz;
                    aabb.max = (entity.transform * V4(aabb.max, 1.0f)).xyz;
                    DrawAlignedBoxOutline(&context->renderGroup, aabb.min, aabb.max, V3(1.0f, 0.0f, 0.0f), 2.0f);
                }
            }
        }
    }

    auto camChunkPos = ChunkPosFromWorldPos(camera->targetWorldPosition.voxel);
    auto playerChunk = GetChunk(&context->gameWorld, camChunkPos.chunk.x, camChunkPos.chunk.y, camChunkPos.chunk.z);
    if (!playerChunk) {
        auto addedChunk = AddChunk(&context->gameWorld, camChunkPos.chunk);
        DebugFillChunk(addedChunk);
        addedChunk = AddChunk(&context->gameWorld, camChunkPos.chunk - IV3(0, 1, 0));
        DebugFillChunk(addedChunk);
    }

    DEBUG_OVERLAY_TRACE(context->gameWorld.mesher->freeBlockCount);
    DEBUG_OVERLAY_TRACE(context->gameWorld.mesher->totalBlockCount);

    auto region = BeginRegion(&context->gameWorld, camera->targetWorldPosition.voxel, GameWorld::ViewDistance);
    region.debugShowBoundaries = true;
    region.debugRender = true;
    DEBUG_OVERLAY_TRACE(camera->targetWorldPosition.voxel);
    DEBUG_OVERLAY_TRACE(camera->targetWorldPosition.offset);
    DrawRegion(&region, group, camera);

    Begin(renderer, group);
    ShadowPass(renderer, group, assetManager);
    MainPass(renderer, group, assetManager);
    End(renderer);

    // Alpha
    //ImGui::PopStyleVar();
}

void FluxRender(Context* context) {}
