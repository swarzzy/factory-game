#pragma once

#include "Common.h"
#include "Math.h"
#include "ChunkMesh.h"

struct RenderGroup;
struct MemoryArena;
struct PlatformState;

struct Mesh {
    char name[32];
    void* base;
    Mesh* head;
    Mesh* next;
    u32 vertexCount;
    u32 indexCount;
    v3* vertices;
    v3* normals;
    v2* uvs;
    v3* tangents;
    v3* bitangents;
    v3* colors;
    u32* indices;
    BBoxAligned aabb;
    u32 gpuVertexBufferHandle;
    u32 gpuIndexBufferHandle;
};

static_assert(sizeof(Mesh) % 8 == 0);

enum struct DynamicRange : u32 {
    LDR, HDR
};

const char* ToString(DynamicRange value) {
    switch (value) {
    case DynamicRange::LDR: { return "LDR"; } break;
    case DynamicRange::HDR: { return "HDR"; } break;
        invalid_default();
    }
    return "";
}

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
    u32 gpuHandle;
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
    enum  Workflow : u32 { Phong = 0, PBR = 1 } workflow;
    union {
        struct {
            b32 useDiffuseMap;
            b32 useSpecularMap;
            union {
                Texture* diffuseMap;
                v3 diffuseValue;
            };
            union {
                Texture* specularMap;
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
                Texture* albedoMap;
                v3 albedoValue;
            };
            union {
                Texture* roughnessMap;
                f32 roughnessValue;
            };
            union {
                Texture* metallicMap;
                f32 metallicValue;
            };
            union {
                Texture* emissionMap;
                struct {
                    v3 emissionValue;
                    f32 emissionIntensity;
                };
            };
            Texture* AOMap;
            Texture* normalMap;
        } pbr;
    };
};

struct TexTransferBufferInfo {
    u32 index;
    void* ptr;
};

struct RendererState {
    u32 wRenderResolution;
    u32 hRenderResolution;
    u32 sampleCount;
};

struct RendererCaps {
    u32 maxSampleCount;
};

struct RendererInfo {
    RendererState state;
    RendererCaps caps;
};

enum struct RendererCommand : u32 {
    Invalid = 0,
    ChangeRenderResolution,
    BeginFrame,
    ShadowPass,
    MainPass,
    EndFrame,
    LoadResource,
    BeginLoadResource,
    EndLoadResource,
    FreeResource,
    BeginLoadTexture,
    EndLoadTexture,
    SetBlockTexture,
    RecompileShaders,
    Initialize,
    LibraryReload
};

struct ChangeRenderResolutionArgs {
    u32 wRenderResolution;
    u32 hRenderResolution;
    u32 sampleCount;
};

struct BeginFrameArgs {
    RenderGroup* group;
};

struct ShadowPassArgs {
    RenderGroup* group;
};

struct MainPassArgs {
    RenderGroup* group;
};

enum struct RenderResourceType : u32 {
    CubeTexture, Mesh, ChunkMesh, Texture
};

struct LoadResourceArgs {
    RenderResourceType type;
    union {
        struct {
            CubeTexture* texture;
        } cubeTexture;
        struct {
            Mesh* mesh;
        } mesh;
        struct {
            ChunkMesh* mesh;
            b32 async;
        } chunkMesh;
        struct {
            Texture* texture;
        } texture;
    };
};

struct BeginLoadResourceArgs {
    ChunkMesh* mesh;
};

struct EndLoadResourceArgs {
    ChunkMesh* mesh;
    // TODO(swarzzy): For now just write result here. We will need to change this
    // if commands are become not immediate
    b32 result;
};

struct FreeResourceArgs {
    RenderResourceType type;
    uptr handle;
};

struct BeginLoadTextureArgs {
    TexTransferBufferInfo bufferInfo;
    uptr bufferSize;
};

struct EndLoadTextureArgs {
    TexTransferBufferInfo bufferInfo;
    Texture* result;
};

struct SetBlockTextureArgs {
    u32 value;
    void* imageBits;
};

struct InitializeArgs {
    MemoryArena* tempArena;
    u32 wResolution;
    u32 hResolution;
    u32 sampleCount;
};

enum struct RendererInvoke : u32 {
    Init, Reload
};

typedef RendererInfo(__cdecl RendererGetInfoFn)();
typedef void(__cdecl RendererExecuteCommandFn)(RendererCommand command, void* args);
typedef void(__cdecl RendererPlatformInvokeFn)(RendererInvoke invoke, PlatformState* platform, void* apiData, void** rendererData);
