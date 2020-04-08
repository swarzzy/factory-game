#include "flux_world.h"
#include "flux_serialize.h"

void Update(World* world) {
    for (Entity& entity : world->entityTable) {
        if (entity.id) {
            entity.transform = Translate(entity.p) * Scale(entity.scale) * Rotate(entity.rotationAngles.x, entity.rotationAngles.y, entity.rotationAngles.z);
            entity.invTransform = Inverse(entity.transform).Unwrap();
        }
    }
}

// TODO: Remove context
Option<RaycastResult> Raycast(Context* context, AssetManager* manager, World* world, v3 ro, v3 rd) {
    auto result = Option<RaycastResult>::None();
    f32 tMin = F32::Max;
    u32 hitID = 0;
    for (Entity& entity : world->entityTable) {
        v3 roMesh = (entity.invTransform * V4(ro, 1.0f)).xyz;
        v3 rdMesh = (entity.invTransform * V4(rd, 0.0f)).xyz;
        auto mesh = GetMesh(manager, entity.mesh);
        while (mesh) {
            bool hit = IntersectFast(mesh->aabb, roMesh, rdMesh, 0.0f, F32::Max);
            if (hit) {
                auto intersection = Intersect(mesh->aabb, roMesh, rdMesh, 0.0f, F32::Max);
                if (intersection.hit && intersection.t < tMin) {
                    //assert(mesh->indexCount % 3 || mesh->indexCount == 0);
                    printf("[Raycast] Index count is %lu\n", mesh->indexCount);
                    for (u32 i = 0; i < mesh->indexCount; i += 3) {
                        v3 vt0 = mesh->vertices[i];
                        v3 vt1 = mesh->vertices[i + 1];
                        v3 vt2 = mesh->vertices[i + 2];
                        auto triIntersection =  IntersectRayTriangle(roMesh, rdMesh, vt0, vt1, vt2);
                        if (triIntersection.hit && triIntersection.t > 0.0f) {
                            tMin = intersection.t;
                            hitID = entity.id;
                            break;
                        }
                    }
                }
            }
            mesh = mesh->next;
        }
    }
    if (hitID) {
        result = Option<RaycastResult>::Some({hitID});
    }
    return result;
}

Entity* AddEntity(World* world) {
    Entity* result = nullptr;
    auto id = world->nextEntitySerialNumber;
    assert(id);
    auto entity = Add(&world->entityTable, &id);
    if (entity) {
        world->nextEntitySerialNumber++;
        *entity = {};
        entity->id = id;
        world->entityCount++;
        result = entity;
    }
    return result;
}

void DeleteEntity(World* world, u32 id) {
    assert(id);
    Delete(&world->entityTable, &id);
    world->entityCount--;
}

StoredTexture StoreTexture(AssetManager* manager, u32 id) {
    StoredTexture result = {};
    auto texture = GetTextureSlot(manager, id);
    if (texture) {
        strcpy_s(result.filename, array_count(result.filename), texture->filename);
        result.format = (u32)texture->format;
        result.wrapMode = (u32)texture->wrapMode;
        result.filter = (u32)texture->filter;
        result.range = (u32)texture->range;
    }
    return result;
}

Entity* GetEntity(World* world, u32 id) {
    return Get(&world->entityTable, &id);
}

StoredMaterial StoreMaterial(AssetManager* manager, const Material* m) {
    StoredMaterial result = {};
    result.workflow = (u32)m->workflow;
    switch (m->workflow) {
    case Material::Phong: {
        result.phong.useDiffuseMap = m->phong.useDiffuseMap;
        if (m->phong.useDiffuseMap) {
            result.phong.diffuseMap = StoreTexture(manager, m->phong.diffuseMap);
        } else {
            result.phong.diffuseValue = m->phong.diffuseValue;
        }

        result.phong.useSpecularMap = m->phong.useSpecularMap;
        if (m->phong.useSpecularMap) {
            result.phong.specularMap = StoreTexture(manager, m->phong.specularMap);
        } else {
            result.phong.specularValue = m->phong.specularValue;
        }
    } break;
    case Material::PBRMetallic: {
        result.pbrMetallic.useAlbedoMap = m->pbrMetallic.useAlbedoMap;
        if (m->pbrMetallic.useAlbedoMap) {
            result.pbrMetallic.albedoMap = StoreTexture(manager, m->pbrMetallic.albedoMap);
        } else {
            result.pbrMetallic.albedoValue = m->pbrMetallic.albedoValue;
        }

        result.pbrMetallic.useRoughnessMap = m->pbrMetallic.useRoughnessMap;
        if (m->pbrMetallic.useRoughnessMap) {
            result.pbrMetallic.roughnessMap = StoreTexture(manager, m->pbrMetallic.roughnessMap);
        } else {
            result.pbrMetallic.roughnessValue = m->pbrMetallic.roughnessValue;
        }

        result.pbrMetallic.useMetallicMap = m->pbrMetallic.useMetallicMap;
        if (m->pbrMetallic.useMetallicMap) {
            result.pbrMetallic.metallicMap = StoreTexture(manager, m->pbrMetallic.metallicMap);
        } else {
            result.pbrMetallic.metallicValue = m->pbrMetallic.metallicValue;
        }

        result.pbrMetallic.useNormalMap = m->pbrMetallic.useNormalMap;
        if (m->pbrMetallic.useNormalMap) {
            result.pbrMetallic.normalMap = StoreTexture(manager, m->pbrMetallic.normalMap);
            result.pbrMetallic.normalFormat = (u32)m->pbrMetallic.normalFormat;
        }

        result.pbrMetallic.useAOMap = m->pbrMetallic.useAOMap;
        if (m->pbrMetallic.useAOMap) {
            result.pbrMetallic.AOMap = StoreTexture(manager, m->pbrMetallic.AOMap);
        }

        result.pbrMetallic.emitsLight = m->pbrMetallic.emitsLight;
        result.pbrMetallic.useEmissionMap = m->pbrMetallic.useEmissionMap;
        if (m->pbrMetallic.useEmissionMap) {
            result.pbrMetallic.emissionMap = StoreTexture(manager, m->pbrMetallic.emissionMap);
        } else {
            result.pbrMetallic.emissionValue = m->pbrMetallic.emissionValue;
            result.pbrMetallic.emissionIntensity = m->pbrMetallic.emissionIntensity;
        }
    } break;
    case Material::PBRSpecular: {
        result.pbrSpecular.useAlbedoMap = m->pbrSpecular.useAlbedoMap;
        if (m->pbrSpecular.useAlbedoMap) {
            result.pbrSpecular.albedoMap = StoreTexture(manager, m->pbrSpecular.albedoMap);
        } else {
            result.pbrSpecular.albedoValue = m->pbrSpecular.albedoValue;
        }

        result.pbrSpecular.useSpecularMap = m->pbrSpecular.useSpecularMap;
        if (m->pbrSpecular.useSpecularMap) {
            result.pbrSpecular.specularMap = StoreTexture(manager, m->pbrSpecular.specularMap);
        } else {
            result.pbrSpecular.specularValue = m->pbrSpecular.specularValue;
        }

        result.pbrSpecular.useGlossMap = m->pbrSpecular.useGlossMap;
        if (m->pbrSpecular.useGlossMap) {
            result.pbrSpecular.glossMap = StoreTexture(manager, m->pbrSpecular.glossMap);
        } else {
            result.pbrSpecular.glossValue = m->pbrSpecular.glossValue;
        }

        result.pbrSpecular.useNormalMap = m->pbrSpecular.useNormalMap;
        if (m->pbrSpecular.useNormalMap) {
            result.pbrSpecular.normalMap = StoreTexture(manager, m->pbrSpecular.normalMap);
            result.pbrSpecular.normalFormat = (u32)m->pbrSpecular.normalFormat;
        }

        result.pbrSpecular.useAOMap = m->pbrSpecular.useAOMap;
        if (m->pbrSpecular.useAOMap) {
            result.pbrSpecular.AOMap = StoreTexture(manager, m->pbrSpecular.AOMap);
        }

        result.pbrSpecular.emitsLight = m->pbrSpecular.emitsLight;
        result.pbrSpecular.useEmissionMap = m->pbrSpecular.useEmissionMap;
        if (m->pbrSpecular.useEmissionMap) {
            result.pbrSpecular.emissionMap = StoreTexture(manager, m->pbrSpecular.emissionMap);
        } else {
            result.pbrSpecular.emissionValue = m->pbrSpecular.emissionValue;
            result.pbrSpecular.emissionIntensity = m->pbrSpecular.emissionIntensity;
        }
    } break;
    invalid_default();
    }
    return result;
}

bool SaveToDisk(AssetManager* manager, World* world, const wchar_t* filename) {
    bool result = false;
    if (world->name[0]) {
        u32 fileSize = sizeof(WorldFile) + sizeof(StoredEntity) * world->entityCount;
        void* memory = PlatformAlloc(fileSize);
        defer { PlatformFree(memory); };
        auto header = (WorldFile*)memory;
        auto entities = (StoredEntity*)((byte*)memory + sizeof(WorldFile));

        header->nextEntitySerialNumber = world->nextEntitySerialNumber;
        header->entityCount = world->entityCount;
        header->firstEntityOffset = (u32)((uptr)entities - (uptr)memory);
        strcpy_s(header->name, array_count(header->name), world->name);

        u32 at = 0;
        for (Entity& entity : world->entityTable) {
            assert(at < world->entityCount);
            auto out = entities + at++;
            out->id = entity.id;
            out->p = entity.p;
            out->scale = entity.scale;
            out->rotationAngles.x = entity.rotationAngles.x;
            out->rotationAngles.y = entity.rotationAngles.y;
            out->rotationAngles.z = entity.rotationAngles.z;

            auto mesh = GetMeshSlot(manager, entity.mesh);
            // TODO: Decide what to do when mesh is null
            if (mesh) {
                strcpy_s(out->meshFileName, array_count(out->meshFileName), mesh->filename);
                out->meshFileFormat = (u32)mesh->format;
            }
            out->material = StoreMaterial(manager, &entity.material);
        }

        result = PlatformDebugWriteFile(filename, memory, fileSize);
    }
    return result;
}

u32 LoadTexture(AssetManager* assetManager, StoredTexture* stored) {
    u32 result = 0;
    auto status = AddTexture(assetManager, stored->filename, (TextureFormat)stored->format, (TextureWrapMode)stored->wrapMode, (TextureFilter)stored->filter, (DynamicRange)stored->range);
    switch (status.status) {
    case AddAssetResult::AlreadyExists: {
        AssetName name;
        GetAssetName(stored->filename, &name);
        result = GetID(&assetManager->nameTable, name.name);
        assert(result);
    } break;
    case AddAssetResult::Ok: {
        result = status.id;
    } break;
    default: {
        printf("[World] Texture %s not found\n", stored->filename);
    } break;
    }
    return result;
}

u32 LoadTextureIfExist(AssetManager* assetManager, StoredTexture* stored) {
    u32 result = 0;
    if (stored->filename[0]) {
        result = LoadTexture(assetManager, stored);
    } else {
        printf("[World]: Missing texture!\n");
    }
    return result;
}

Material LoadMaterial(AssetManager* assetManager, StoredMaterial* stored) {
    Material mat = {};
    mat.workflow = (Material::Workflow)stored->workflow;
    switch (mat.workflow) {
    case Material::Phong: {
        if (stored->phong.useDiffuseMap) {
            mat.phong.useDiffuseMap = true;
            mat.phong.diffuseMap = LoadTextureIfExist(assetManager, &stored->phong.diffuseMap);
        } else {
            mat.phong.useDiffuseMap = false;
            mat.phong.diffuseValue = stored->phong.diffuseValue;
        }

        if (stored->phong.useSpecularMap) {
            mat.phong.useSpecularMap = true;
            mat.phong.specularMap = LoadTextureIfExist(assetManager, &stored->phong.specularMap);
        } else {
            mat.phong.useSpecularMap = false;
            mat.phong.specularValue = stored->phong.specularValue;
        }
    } break;
    case Material::PBRMetallic: {
        if (stored->pbrMetallic.useAlbedoMap) {
            mat.pbrMetallic.useAlbedoMap = true;
            mat.pbrMetallic.albedoMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.albedoMap);
        } else {
            mat.pbrMetallic.useAlbedoMap = false;
            mat.pbrMetallic.albedoValue = stored->pbrMetallic.albedoValue;
        }

        if (stored->pbrMetallic.useRoughnessMap) {
            mat.pbrMetallic.useRoughnessMap = true;
            mat.pbrMetallic.roughnessMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.roughnessMap);
        } else {
            mat.pbrMetallic.useRoughnessMap = false;
            mat.pbrMetallic.roughnessValue = stored->pbrMetallic.roughnessValue;
        }

        if (stored->pbrMetallic.useMetallicMap) {
            mat.pbrMetallic.useMetallicMap = true;
            mat.pbrMetallic.metallicMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.metallicMap);
        } else {
            mat.pbrMetallic.useMetallicMap = false;
            mat.pbrMetallic.metallicValue = stored->pbrMetallic.metallicValue;
        }

        if (stored->pbrMetallic.useNormalMap) {
            mat.pbrMetallic.useNormalMap = true;
            mat.pbrMetallic.normalFormat = (NormalFormat)stored->pbrMetallic.normalFormat;
            mat.pbrMetallic.normalMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.normalMap);
        } else {
            mat.pbrMetallic.useNormalMap = false;
        }

        if (stored->pbrMetallic.useAOMap) {
            mat.pbrMetallic.useAOMap = true;
            mat.pbrMetallic.AOMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.AOMap);
        } else {
            mat.pbrMetallic.useAOMap = false;
        }

        mat.pbrMetallic.emitsLight = stored->pbrMetallic.emitsLight;
        if (stored->pbrMetallic.useEmissionMap) {
            mat.pbrMetallic.useEmissionMap = true;
            mat.pbrMetallic.emissionMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.emissionMap);
        } else {
            mat.pbrMetallic.useEmissionMap = false;
            mat.pbrMetallic.emissionValue = stored->pbrMetallic.emissionValue;
            mat.pbrMetallic.emissionIntensity = stored->pbrMetallic.emissionIntensity;
        }
    } break;
    case Material::PBRSpecular: {
        if (stored->pbrSpecular.useAlbedoMap) {
            mat.pbrSpecular.useAlbedoMap = true;
            mat.pbrSpecular.albedoMap = LoadTextureIfExist(assetManager, &stored->pbrSpecular.albedoMap);
        } else {
            mat.pbrSpecular.useAlbedoMap = false;
            mat.pbrSpecular.albedoValue = stored->pbrSpecular.albedoValue;
        }

        if (stored->pbrSpecular.useSpecularMap) {
            mat.pbrSpecular.useSpecularMap = true;
            mat.pbrSpecular.specularMap = LoadTextureIfExist(assetManager, &stored->pbrSpecular.specularMap);
        } else {
            mat.pbrSpecular.useSpecularMap = false;
            mat.pbrSpecular.specularValue = stored->pbrSpecular.specularValue;
        }

        if (stored->pbrSpecular.useGlossMap) {
            mat.pbrSpecular.useGlossMap = true;
            mat.pbrSpecular.glossMap = LoadTextureIfExist(assetManager, &stored->pbrSpecular.glossMap);
        } else {
            mat.pbrSpecular.useGlossMap = false;
            mat.pbrSpecular.glossValue = stored->pbrSpecular.glossValue;
        }

        if (stored->pbrSpecular.useNormalMap) {
            mat.pbrSpecular.useNormalMap = true;
            mat.pbrSpecular.normalFormat = (NormalFormat)stored->pbrSpecular.normalFormat;
            mat.pbrSpecular.normalMap = LoadTextureIfExist(assetManager, &stored->pbrSpecular.normalMap);
        } else {
            mat.pbrSpecular.useNormalMap = false;
        }

        if (stored->pbrMetallic.useAOMap) {
            mat.pbrMetallic.useAOMap = true;
            mat.pbrMetallic.AOMap = LoadTextureIfExist(assetManager, &stored->pbrMetallic.AOMap);
        } else {
            mat.pbrMetallic.useAOMap = false;
        }

        mat.pbrSpecular.emitsLight = stored->pbrSpecular.emitsLight;
        if (stored->pbrSpecular.useEmissionMap) {
            mat.pbrSpecular.useEmissionMap = true;
            mat.pbrSpecular.emissionMap = LoadTextureIfExist(assetManager, &stored->pbrSpecular.emissionMap);
        } else {
            mat.pbrSpecular.useEmissionMap = false;
            mat.pbrSpecular.emissionValue = stored->pbrSpecular.emissionValue;
            mat.pbrSpecular.emissionIntensity = stored->pbrSpecular.emissionIntensity;
        }

    } break;
    invalid_default();
    }
    return mat;
}

World* LoadWorldFromDisc(AssetManager* assetManager, const wchar_t* filename) {
    World* world = nullptr;
    auto fileSize = PlatformDebugGetFileSize(filename);
    if (fileSize) {
        auto fileBuffer = PlatformAlloc(fileSize);
        defer { PlatformFree(fileBuffer); };
        u32 bytesRead = PlatformDebugReadFile(fileBuffer, fileSize, filename);
        assert(fileSize == bytesRead);
        auto header = (WorldFile*)fileBuffer;

        // TODO: Pretty zeroed allocations
        world = (World*)PlatformAlloc(sizeof(World));
        *world = {};

        world->nextEntitySerialNumber = header->nextEntitySerialNumber;
        world->entityCount = header->entityCount;
        assert(header->name[0]);
        strcpy_s(world->name, array_count(world->name), header->name);

        auto fileEntities = (StoredEntity*)((byte*)fileBuffer + header->firstEntityOffset);
        for (u32 i = 0; i < world->entityCount; i++) {
            auto stored = fileEntities + i;
            auto entry = Add(&world->entityTable, &stored->id);
            assert(entry);
            entry->id = stored->id;
            entry->p = stored->p;
            entry->scale = stored->scale;
            entry->rotationAngles = V3(stored->rotationAngles.x, stored->rotationAngles.y, stored->rotationAngles.z);
            entry->material = LoadMaterial(assetManager, &stored->material);
            auto result = AddMesh(assetManager, stored->meshFileName, (MeshFileFormat)stored->meshFileFormat);
            switch (result.status) {
            case AddAssetResult::AlreadyExists: {
                AssetName name;
                GetAssetName(stored->meshFileName, &name);
                entry->mesh = GetID(&assetManager->nameTable, name.name);
                assert(entry->mesh);
            } break;
            case AddAssetResult::Ok: {
                entry->mesh = result.id;
            } break;
            default: {
                entry->mesh = 0;
                printf("[World] Mesh %s not found\n", stored->meshFileName);
            } break;
            }
        }
    }
    return world;
}
