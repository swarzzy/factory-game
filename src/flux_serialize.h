#pragma once

#pragma pack(push, 1)

// TODO: This is totally memory wasteland
// We should need store it more efficient
struct StoredTexture {
    char filename[MaxAssetPathSize];
    u32 format;
    u32 wrapMode;
    u32 filter;
    u32 range;
};

struct StoredMaterial {
    u32 workflow;
    union {
        struct {
            u32 useDiffuseMap;
            u32 useSpecularMap;
            union {
                StoredTexture diffuseMap;
                v3 diffuseValue;
            };
            union {
                StoredTexture specularMap;
                v3 specularValue;
            };
        } phong;
        struct {
            u32 useAlbedoMap;
            u32 useRoughnessMap;
            u32 useMetallicMap;
            u32 useNormalMap;
            u32 useAOMap;
            u32 emitsLight;
            u32 useEmissionMap;
            u32 normalFormat;
            byte roughnessMask;
            byte metallicMask;
            byte aoMask;
            byte emissionMask;
            union {
                StoredTexture albedoMap;
                v3 albedoValue;
            };
            union {
                StoredTexture roughnessMap;
                f32 roughnessValue;
            };
            union {
                StoredTexture metallicMap;
                f32 metallicValue;
            };
            union {
                StoredTexture emissionMap;
                struct {
                    v3 emissionValue;
                    f32 emissionIntensity;
                };
            };
            StoredTexture AOMap;
            StoredTexture normalMap;
        } pbrMetallic;
        struct {
            u32 useAlbedoMap;
            u32 useSpecularMap;
            u32 useGlossMap;
            u32 useNormalMap;
            u32 useAOMap;
            u32 emitsLight;
            u32 useEmissionMap;
            u32 normalFormat;
            byte specularMask; // ???
            byte glossMask;
            byte emissionMask; // ???
            byte aoMask;
            union {
                StoredTexture albedoMap;
                v3 albedoValue;
            };
            union {
                StoredTexture specularMap;
                v3 specularValue;
            };
            union {
                StoredTexture glossMap;
                f32 glossValue;
            };
            union {
                StoredTexture emissionMap;
                struct {
                    v3 emissionValue;
                    f32 emissionIntensity;
                };
            };
            StoredTexture AOMap;
            StoredTexture normalMap;
        } pbrSpecular;
    };
};

struct StoredEntity {
    u32 id;
    v3 p;
    v3 scale;
    v4 rotationAngles; // w is reserved for quaternions
    StoredMaterial material;
    u32 meshFileFormat;
    char meshFileName[MaxAssetPathSize];
};

struct WorldFile {
    u32 nextEntitySerialNumber;
    u32 entityCount;
    u32 firstEntityOffset;
    char name[WorldNameSize];
};
#pragma pack(pop)
