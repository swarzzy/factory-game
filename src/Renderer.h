#pragma once

struct RenderGroup;

#include "RendererAPI.h"

void RendererChangeResolution(u32 w, u32 h, u32 sampleCount);
void RendererBeginFrame(RenderGroup* group);
void RendererShadowPass(RenderGroup* group);
void RendererMainPass(RenderGroup* group);
void RendererEndFrame();
void RendererLoadResource(CubeTexture* texture);
void RendererLoadResource(Mesh* mesh);
void RendererLoadResource(ChunkMesh* mesh, bool async);
void RendererLoadResource(Texture* texture);
void RendererBeginLoadResource(ChunkMesh* mesh);
bool RendererEndLoadResource(ChunkMesh* mesh);
void RendererFreeTexture(uptr handle);
void RendererFreeBuffer(uptr handle);
TexTransferBufferInfo RendererBeginLoadTexture(u32 bufferSize);
void RendererEndLoadTexture(TexTransferBufferInfo info, Texture* outTexture);
void RendererSetBlockTexture(u32 value, void* data);
void RendererRecompileShaders();
void RendererInitialize(MemoryArena* tempArena, uv2 renderRes, u32 sampleCount);
void RendererSetLogger(LoggerFn* logger, void* loggerData, AssertHandlerFn* assertHandler, void* assertHandlerData);

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
