#pragma once

#include "../RendererAPI.h"
#include "../Globals.h"

#include "../Resource.h"

#include "OpenglStd140.h"
#include "OpenglShaders.h"

struct Renderer {
    union {
        Shaders shaders;
        GLuint shaderHandles[ShaderCount];
    };

    u32 uniformBufferAligment;
    u32 maxSupportedSampleCount;
    u32 maxTexArrayLayers;

    GLuint lineBufferHandle;
    GLuint chunkIndexBuffer;
    v4 clearColor;

    uv2 renderRes;
    u32 sampleCount;
    f32 gamma = 2.4f;
    f32 exposure = 1.0f;

    GLuint offscreenBufferHandle;
    GLuint offscreenColorTarget;
    GLuint offscreenDepthTarget;

    GLuint offscreenDownsampledBuffer;
    GLuint offscreenDownsampledColorTarget;

    static constexpr u32 RandomValuesTextureSize = 1024;
    // NOTE: Number of cascades is always 3
    static constexpr u32 NumShadowCascades = 3;
    GLuint shadowMapFramebuffers[NumShadowCascades];
    GLuint shadowMapDepthTarget;
    GLuint shadowMapDebugColorTarget;
    u32 shadowMapRes = 2048;
    GLuint randomValuesTexture;
    b32 stableShadows = true;
    b32 showShadowCascadesBoundaries;
    f32 shadowFilterScale = 1.0f;

    // TODO: Camera should cares about that
    m4x4 shadowCascadeViewProjMatrices[NumShadowCascades];
    f32 shadowCascadeBounds[NumShadowCascades];

    f32 shadowConstantBias = 0.004f;
    f32 shadowSlopeBiasScale = 1.2f;
    f32 shadowNormalBiasScale;

    GLuint srgbBufferHandle;
    GLuint srgbColorTarget;

    GLfloat maxAnisotropy;

    GLuint captureFramebuffer;

    GLuint BRDFLutHandle;

    b32 debugF;
    b32 debugD;
    b32 debugG;
    b32 debugNormals;

    static constexpr u32 TextureTransferBufferCount = 32;
    u32 textureTransferBuffersUsageCount;
    b32 textureTransferBuffersUsage[TextureTransferBufferCount];
    GLuint textureTransferBuffers[TextureTransferBufferCount];

    Material fallbackPhongMaterial;
    Material fallbackMetallicMaterial;

    // TODO: Check is using 0 handle has consistant behavior on all GPUs
    // Maybe we need to create some placeholder texture here
    GLuint nullTexture2D = 0;

    // NOTE: ChunkSize * ChunkSize * ChunkSize * VerticesInQuad * QuadsInBlock
    // Dividing by two because the maximum index count will be used when chunk has chuckerboard pattern,
    // i.e. only half of voxels are filled
    static constexpr uptr ChunkIndexBufferCount = Globals::ChunkSize * Globals::ChunkSize * Globals::ChunkSize * 6 * 6 / 2;
    static constexpr uptr ChunkIndexBufferSize = Globals::ChunkSize * Globals::ChunkSize * Globals::ChunkSize * 6 * 6 / 2 * sizeof(u32);

    GLuint chunkIndexBufferHandle;

    // TODO: Chunk rendering
    static constexpr u32 TerrainTextureSize = 256;
    GLuint terrainTexArray;

    // TODO(swarzzy): They are temporary live here
    CubeTexture hdrMap;
    CubeTexture irradianceMap;
    CubeTexture environmentMap;


    UniformBuffer<ShaderFrameData, ShaderFrameData::Binding> frameUniformBuffer;
    UniformBuffer<ShaderMeshData, ShaderMeshData::Binding> meshUniformBuffer;
};

void Begin(Renderer* renderer, RenderGroup* group);
void ShadowPass(Renderer* renderer, RenderGroup* group);
void MainPass(Renderer* renderer, RenderGroup* group);
void End(Renderer* renderer);
void UploadToGPU(CubeTexture* texture);
void UploadToGPU(Mesh* mesh);
void UploadToGPU(ChunkMesh* mesh, bool async);
void UploadToGPU(Texture* texture);
void BeginGPUUpload(ChunkMesh* mesh);
bool EndGPUUpload(ChunkMesh* mesh);
void SetBlockTexture(Renderer* renderer, u32 value, void* data);
void InitializeRenderer(Renderer* renderer, MemoryArena* tempArena, uv2 renderRes, u32 sampleCount);
void ChangeRenderResolution(Renderer* renderer, u32 wNew, u32 hNew, u32 newSampleCount);
void FreeGPUBuffer(u32 id);
void FreeGPUTexture(u32 id);
TexTransferBufferInfo GetTextureTransferBuffer(Renderer* renderer, u32 size);
void CompleteTextureTransfer(TexTransferBufferInfo* info, Texture* texture);
