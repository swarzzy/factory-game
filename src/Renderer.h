#pragma once

struct Renderer;
struct CubeTexture;
struct Texture;
struct AssetManager;
struct ChunkMesh;
struct RenderGroup;

#include "World.h"
#include "RendererAPI.h"


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
    args.value = (u32)value;
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
