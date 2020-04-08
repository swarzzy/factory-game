#pragma once
#include "flux_resource_manager.h"
#include "flux_hash_map.h"

enum struct NormalFormat {
    OpenGL = 0, DirectX = 1
};

const char* ToString(NormalFormat value) {
    static const char* strings[] = {
        "OpenGL",
        "DirectX"
    };
    assert((u32)value < array_count(strings));
    return strings[(u32)value];
}

struct Material {
    enum  Workflow : u32 { Phong = 0, PBRMetallic = 1, PBRSpecular = 2, } workflow;
    union {
        struct {
            b32 useDiffuseMap;
            b32 useSpecularMap;
            union {
                u32 diffuseMap;
                v3 diffuseValue;
            };
            union {
                u32 specularMap;
                v3 specularValue;
            };
        } phong;
        struct {
            b32 useAlbedoMap;
            b32 useRoughnessMap;
            b32 useMetallicMap;
            b32 useNormalMap;
            b32 useAOMap;
            b32 emitsLight;
            b32 useEmissionMap;
            NormalFormat normalFormat;
            union {
                u32 albedoMap;
                v3 albedoValue;
            };
            union {
                u32 roughnessMap;
                f32 roughnessValue;
            };
            union {
                u32 metallicMap;
                f32 metallicValue;
            };
            union {
                u32 emissionMap;
                struct {
                    v3 emissionValue;
                    f32 emissionIntensity;
                };
            };
            u32 AOMap;
            u32 normalMap;
        } pbrMetallic;
        struct {
            b32 useAlbedoMap;
            b32 useSpecularMap;
            b32 useGlossMap;
            b32 useNormalMap;
            b32 useAOMap;
            b32 emitsLight;
            b32 useEmissionMap;
            NormalFormat normalFormat;
            union {
                u32 albedoMap;
                v3 albedoValue;
            };
            union {
                u32 specularMap;
                v3 specularValue;
            };
            union {
                u32 glossMap;
                f32 glossValue;
            };
            union {
                u32 emissionMap;
                struct {
                    v3 emissionValue;
                    f32 emissionIntensity;
                };
            };
            u32 AOMap;
            u32 normalMap;
        } pbrSpecular;
    };
};

const char* ToString(Material::Workflow value) {
    switch (value) {
    case Material::Phong: { return "Phong"; } break;
    case Material::PBRMetallic: { return "PBR metallic"; } break;
    case Material::PBRSpecular: { return "PBR specular"; } break;
    invalid_default();
    }
    return "";
}

struct Entity {
    u32 id;
    v3 p;
    v3 scale = V3(1.0f);
    v3 rotationAngles;
    u32 mesh;
    Material material;
    m4x4 transform;
    m4x4 invTransform;
};

// TODO: Entity iterators
struct World {
    static u32 Hasher(void* key) { return *((u32*)key); }
    static bool Comparator(void* a, void* b) { return *((u32*)a) == *((u32*)b); }
    u32 nextEntitySerialNumber = 1;
    u32 entityCount;
    HashMap<u32, Entity, Hasher, Comparator> entityTable = HashMap<u32, Entity, Hasher, Comparator>::Make();
    char name[WorldNameSize];
};


struct RaycastResult {
    u32 entityId;
};

struct Context ;

void Update(World* world);
Option<RaycastResult> Raycast(Context* context, AssetManager* manager, World* world, v3 ro, v3 rd);
Entity* AddEntity(World* world);
void DeleteEntity(World* world, u32 id);
Entity* GetEntity(World* world, u32 id);
bool SaveToDisk(AssetManager* assetManager, World* world, const wchar_t* filename);
World* LoadWorldFromDisc(AssetManager* assetManager, const wchar_t* filename);
