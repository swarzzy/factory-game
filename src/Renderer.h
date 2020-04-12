#pragma once

struct Renderer;
struct CubeTexture;
struct Texture;
struct AssetManager;
struct ChunkMesh;

#include "World.h"

enum struct TextureFilter : u32 {
    None = 0, Bilinear, Trilinear, Anisotropic, Default = Bilinear
};

enum struct TextureFormat : u32 {
    Unknown = 0,
    SRGBA8,
    SRGB8,
    RGBA8,
    RGB8,
    RGB16F,
    RG16F,
    R8,
    RG8,
    RG32F,
};

enum struct TextureWrapMode : u32 {
    Repeat = 0, ClampToEdge, Default = Repeat
};

struct Texture {
    void* base;
    TextureFilter filter;
    TextureFormat format;
    TextureWrapMode wrapMode;
    DynamicRange range;
    u32 width;
    u32 height;
    void* data;
    u32 gpuHandle;
};

struct CubeTexture {
    GLuint gpuHandle;
    TextureFilter filter;
    TextureFormat format;
    TextureWrapMode wrapMode;
    u32 width;
    u32 height;
    b32 useMips;
    union {
        void* data[6];
        struct {
            void* rightData;
            void* leftData;
            void* upData;
            void* downData;
            void* frontData;
            void* backData;
        };
    };
};


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

struct TexTransferBufferInfo {
    u32 index;
    void* ptr;
    Renderer* renderer;
};

Renderer* InitializeRenderer(MemoryArena* arena, MemoryArena* tempArena, uv2 renderRes, u32 sampleCount);

void GenIrradanceMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle);
void GenEnvPrefiliteredMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle, u32 mipLevels);

// TODO: Temporary hack while struct is defined in cpp file
uv2 GetRenderResolution(Renderer* renderer);
u32 GetRenderSampleCount(Renderer* renderer);
u32 GetRenderMaxSampleCount(Renderer* renderer);
void ChangeRenderResolution(Renderer* renderer, uv2 newRes, u32 newSampleCount);

struct RenderGroup;

void Begin(Renderer* renderer, RenderGroup* group);
void ShadowPass(Renderer* renderer, RenderGroup* group);
void MainPass(Renderer* renderer, RenderGroup* group);
void End(Renderer* renderer);

void UploadToGPU(CubeTexture* texture);
void UploadToGPU(Mesh* mesh);
bool UploadToGPU(ChunkMesh* mesh, bool async);
void UploadToGPU(Texture* texture);

void BeginGPUUpload(ChunkMesh* mesh);
bool EndGPUpload(ChunkMesh* mesh);

void FreeGPUBuffer(u32 id);
void FreeGPUTexture(u32 id);

TexTransferBufferInfo GetTextureTransferBuffer(Renderer* renderer, u32 size);
void CompleteTextureTransfer(TexTransferBufferInfo* info, Texture* texture);

u16 VoxelValueToTerrainIndex(VoxelValue value);
void SetVoxelTexture(Renderer* renderer, VoxelValue value, void* data);

void RecompileShaders(MemoryArena* tempArena, Renderer* renderer);
