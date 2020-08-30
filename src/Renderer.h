#pragma once

struct Renderer;
struct CubeTexture;
struct Texture;
struct AssetManager;
struct ChunkMesh;
struct RenderGroup;

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
    Renderer* renderer;
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
    BlockValue value;
    void* imageBits;
};

struct InitializeArgs {
    MemoryArena* arena;
    MemoryArena* tempArena;
    u32 wResolution;
    u32 hResolution;
    u32 sampleCount;
    Renderer* renderer;
};

RendererInfo GetRendererInfo(Renderer* renderer);
void RendererExecuteCommandImmediately(Renderer* renderer, RendererCommand command, void* args);


inline void RendererChangeResolution(Renderer* renderer, u32 w, u32 h, u32 sampleCount) {
    ChangeRenderResolutionArgs args {};
    args.wRenderResolution = w;
    args.hRenderResolution = h;
    args.sampleCount = sampleCount;

    RendererExecuteCommandImmediately(renderer, RendererCommand::ChangeRenderResolution, &args);
}

inline void RendererBeginFrame(Renderer* renderer, RenderGroup* group) {
    BeginFrameArgs args {};
    args.group = group;
    RendererExecuteCommandImmediately(renderer, RendererCommand::BeginFrame, &args);
}

inline void RendererShadowPass(Renderer* renderer, RenderGroup* group) {
    ShadowPassArgs args {};
    args.group = group;
    RendererExecuteCommandImmediately(renderer, RendererCommand::ShadowPass, &args);
}

inline void RendererMainPass(Renderer* renderer, RenderGroup* group) {
    MainPassArgs args {};
    args.group = group;
    RendererExecuteCommandImmediately(renderer, RendererCommand::MainPass, &args);
}

inline void RendererEndFrame(Renderer* renderer) {
    RendererExecuteCommandImmediately(renderer, RendererCommand::EndFrame, nullptr);
}

inline void RendererLoadResource(Renderer* renderer, CubeTexture* texture) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::CubeTexture;
    args.cubeTexture.texture = texture;
    RendererExecuteCommandImmediately(renderer, RendererCommand::LoadResource, &args);
}

inline void RendererLoadResource(Renderer* renderer, Mesh* mesh) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::Mesh;
    args.mesh.mesh = mesh;
    RendererExecuteCommandImmediately(renderer, RendererCommand::LoadResource, &args);
}

inline void RendererLoadResource(Renderer* renderer, ChunkMesh* mesh, bool async) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::ChunkMesh;
    args.chunkMesh.mesh = mesh;
    args.chunkMesh.async = async;
    RendererExecuteCommandImmediately(renderer, RendererCommand::LoadResource, &args);
}

inline void RendererLoadResource(Renderer* renderer, Texture* texture) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::Texture;
    args.texture.texture = texture;
    RendererExecuteCommandImmediately(renderer, RendererCommand::LoadResource, &args);
}

inline void RendererBeginLoadResource(Renderer* renderer, ChunkMesh* mesh) {
    BeginLoadResourceArgs args {};
    args.mesh = mesh;
    RendererExecuteCommandImmediately(renderer, RendererCommand::BeginLoadResource, &args);
}

inline bool RendererEndLoadResource(Renderer* renderer, ChunkMesh* mesh) {
    EndLoadResourceArgs args {};
    args.mesh = mesh;
    RendererExecuteCommandImmediately(renderer, RendererCommand::EndLoadResource, &args);
    return args.result;
}

inline void RendererFreeTexture(Renderer* renderer, uptr handle) {
    FreeResourceArgs args {};
    args.type = RenderResourceType::Texture;
    args.handle = handle;
    RendererExecuteCommandImmediately(renderer, RendererCommand::FreeResource, &args);
}

inline void RendererFreeBuffer(Renderer* renderer, uptr handle) {
    FreeResourceArgs args {};
    args.type = RenderResourceType::Mesh;
    args.handle = handle;
    RendererExecuteCommandImmediately(renderer, RendererCommand::FreeResource, &args);
}

inline TexTransferBufferInfo RendererBeginLoadTexture(Renderer* renderer, u32 bufferSize) {
    BeginLoadTextureArgs args {};
    args.bufferSize = bufferSize;
    RendererExecuteCommandImmediately(renderer, RendererCommand::BeginLoadTexture, &args);
    return args.bufferInfo;
}

inline void RendererEndLoadTexture(Renderer* renderer, TexTransferBufferInfo info, Texture* outTexture) {
    EndLoadTextureArgs args {};
    args.bufferInfo = info;
    args.result = outTexture;
    RendererExecuteCommandImmediately(renderer, RendererCommand::EndLoadTexture, &args);
}

inline void RendererSetBlockTexture(Renderer* renderer, BlockValue value, void* data) {
    SetBlockTextureArgs args {};
    args.value = value;
    args.imageBits = data;
    RendererExecuteCommandImmediately(renderer, RendererCommand::SetBlockTexture, &args);
}

inline void RendererRecompileShaders(Renderer* renderer) {
    RendererExecuteCommandImmediately(renderer, RendererCommand::RecompileShaders, nullptr);
}

inline Renderer* RendererInitialize(MemoryArena* arena, MemoryArena* tempArena, uv2 renderRes, u32 sampleCount) {
    InitializeArgs args {};
    args.arena = arena;
    args.tempArena = tempArena;
    args.wResolution = renderRes.x;
    args.hResolution = renderRes.y;
    args.sampleCount = sampleCount;
    RendererExecuteCommandImmediately(nullptr, RendererCommand::Initialize, &args);
    return args.renderer;
}


u16 BlockValueToTerrainIndex(BlockValue value) {
    u16 index = 0;
    switch(value) {
    case BlockValue::Stone: { index = 0; } break;
    case BlockValue::Grass: { index = 1; } break;
    case BlockValue::CoalOre: { index = 2; } break;
    case BlockValue::Water: { index = 3; } break;
    invalid_default();
    }
    return index;
}
