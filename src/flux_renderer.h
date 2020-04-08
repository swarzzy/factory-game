#pragma once

struct Renderer;
struct CubeTexture;
struct Texture;
struct AssetManager;
struct ChunkMesh;

#include "World.h"

struct TexTransferBufferInfo {
    u32 index;
    void* ptr;
    Renderer* renderer;
};

Renderer* InitializeRenderer(uv2 renderRes, u32 sampleCount);

void GenIrradanceMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle);
void GenEnvPrefiliteredMap(const Renderer* renderer, CubeTexture* t, GLuint sourceHandle, u32 mipLevels);

// TODO: Temporary hack while struct is defined in cpp file
uv2 GetRenderResolution(Renderer* renderer);
u32 GetRenderSampleCount(Renderer* renderer);
u32 GetRenderMaxSampleCount(Renderer* renderer);
void ChangeRenderResolution(Renderer* renderer, uv2 newRes, u32 newSampleCount);

struct RenderGroup;

void Begin(Renderer* renderer, RenderGroup* group);
void ShadowPass(Renderer* renderer, RenderGroup* group, AssetManager* manager);
void MainPass(Renderer* renderer, RenderGroup* group, AssetManager* manager);
void End(Renderer* renderer);

void UploadToGPU(CubeTexture* texture);
void UploadToGPU(Mesh* mesh);
void UploadToGPU(ChunkMesh* mesh);
void UploadToGPU(Texture* texture);

void FreeGPUBuffer(u32 id);
void FreeGPUTexture(u32 id);

TexTransferBufferInfo GetTextureTransferBuffer(Renderer* renderer, u32 size);
void CompleteTextureTransfer(TexTransferBufferInfo* info, Texture* texture);

u16 VoxelValueToTerrainIndex(VoxelValue value);
void SetVoxelTexture(Renderer* renderer, VoxelValue value, void* data);

void RecompileShaders(Renderer* renderer);
