#include "Renderer.h"

void RendererChangeResolution(u32 w, u32 h, u32 sampleCount) {
    ChangeRenderResolutionArgs args {};
    args.wRenderResolution = w;
    args.hRenderResolution = h;
    args.sampleCount = sampleCount;

    Renderer.ExecuteCommand(RendererCommand::ChangeRenderResolution, &args);
}

void RendererBeginFrame(RenderGroup* group) {
    BeginFrameArgs args {};
    args.group = group;
    Renderer.ExecuteCommand(RendererCommand::BeginFrame, &args);
}

void RendererShadowPass(RenderGroup* group) {
    ShadowPassArgs args {};
    args.group = group;
    Renderer.ExecuteCommand(RendererCommand::ShadowPass, &args);
}

void RendererMainPass(RenderGroup* group) {
    MainPassArgs args {};
    args.group = group;
    Renderer.ExecuteCommand(RendererCommand::MainPass, &args);
}

void RendererEndFrame() {
    Renderer.ExecuteCommand(RendererCommand::EndFrame, nullptr);
}

void RendererLoadResource(CubeTexture* texture) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::CubeTexture;
    args.cubeTexture.texture = texture;
    Renderer.ExecuteCommand(RendererCommand::LoadResource, &args);
}

void RendererLoadResource(Mesh* mesh) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::Mesh;
    args.mesh.mesh = mesh;
    Renderer.ExecuteCommand(RendererCommand::LoadResource, &args);
}

void RendererLoadResource(ChunkMesh* mesh, bool async) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::ChunkMesh;
    args.chunkMesh.mesh = mesh;
    args.chunkMesh.async = async;
    Renderer.ExecuteCommand(RendererCommand::LoadResource, &args);
}

void RendererLoadResource(Texture* texture) {
    LoadResourceArgs args {};
    args.type = RenderResourceType::Texture;
    args.texture.texture = texture;
    Renderer.ExecuteCommand(RendererCommand::LoadResource, &args);
}

void RendererBeginLoadResource(ChunkMesh* mesh) {
    BeginLoadResourceArgs args {};
    args.mesh = mesh;
    Renderer.ExecuteCommand(RendererCommand::BeginLoadResource, &args);
}

bool RendererEndLoadResource(ChunkMesh* mesh) {
    EndLoadResourceArgs args {};
    args.mesh = mesh;
    Renderer.ExecuteCommand(RendererCommand::EndLoadResource, &args);
    return args.result;
}

void RendererFreeTexture(uptr handle) {
    FreeResourceArgs args {};
    args.type = RenderResourceType::Texture;
    args.handle = handle;
    Renderer.ExecuteCommand(RendererCommand::FreeResource, &args);
}

void RendererFreeBuffer(uptr handle) {
    FreeResourceArgs args {};
    args.type = RenderResourceType::Mesh;
    args.handle = handle;
    Renderer.ExecuteCommand(RendererCommand::FreeResource, &args);
}

TexTransferBufferInfo RendererBeginLoadTexture(u32 bufferSize) {
    BeginLoadTextureArgs args {};
    args.bufferSize = bufferSize;
    Renderer.ExecuteCommand(RendererCommand::BeginLoadTexture, &args);
    return args.bufferInfo;
}

void RendererEndLoadTexture(TexTransferBufferInfo info, Texture* outTexture) {
    EndLoadTextureArgs args {};
    args.bufferInfo = info;
    args.result = outTexture;
    Renderer.ExecuteCommand(RendererCommand::EndLoadTexture, &args);
}

void RendererSetBlockTexture(u32 value, void* data) {
    SetBlockTextureArgs args {};
    args.value = value;
    args.imageBits = data;
    Renderer.ExecuteCommand(RendererCommand::SetBlockTexture, &args);
}

void RendererRecompileShaders() {
    Renderer.ExecuteCommand(RendererCommand::RecompileShaders, nullptr);
}

void RendererInitialize(MemoryArena* tempArena, uv2 renderRes, u32 sampleCount) {
    InitializeArgs args {};
    args.tempArena = tempArena;
    args.wResolution = renderRes.x;
    args.hResolution = renderRes.y;
    args.sampleCount = sampleCount;
    Renderer.ExecuteCommand(RendererCommand::Initialize, &args);
}
