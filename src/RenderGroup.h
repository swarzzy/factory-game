#pragma once
#include "Renderer.h"

struct DirectionalLight {
    v3 from;
    v3 dir;
    v3 ambient;
    v3 diffuse;
    v3 specular;
};

enum struct RenderCommand : u32 {
    DrawMesh = 1,
    SetDirLight,
    LineBegin,
    LinePushVertex,
    LineEnd,
    BeginChunkBatch,
    PushChunk,
    EndChunkBatch
};

struct Mesh;
struct Material;
struct Texture;

struct ChunkMesh;

struct RenderCommandBeginChunkBatch {};

struct RenderCommandEndChunkBatch {};

struct RenderCommandPushChunk {
    ChunkMesh* mesh;
    v3 offset;
};

struct RenderCommandDrawMesh {
    m4x4 transform;
    Mesh* mesh;
    // TODO: Pointer?
    Material* material;
    enum DrawMeshFlags : u32 { Highlight, Wireframe } flags;
};

struct RenderCommandSetDirLight {
    DirectionalLight light;
};

struct RenderCommandLineBegin {
    enum RenderLineType : u32 { Segments, Strip } type;
    v3 color;
    f32 width;
};

struct RenderCommandPushLineVertex {
    v3 vertex;
};

struct RenderCommandLineEnd {};

struct CommandQueueEntry {
    uptr rbOffset;
    RenderCommand type;
    u32 instanceCount;
};

struct RenderGroup {
    const CameraBase* camera;

    DirectionalLight dirLight;

    byte* renderBuffer;
    byte* renderBufferAt;
    uptr renderBufferSize;
    uptr renderBufferFree;

    CommandQueueEntry* pendingLineBatchCommandHeader;
    CommandQueueEntry* pendingChunkBatchCommandHeader;

    CommandQueueEntry* commandQueue;
    u32 commandQueueCapacity;
    u32 commandQueueAt;

    b32 drawSkybox;
    u32 skyboxHandle;

    u32 irradanceMapHandle;
    u32 envMapHandle;

    static RenderGroup Make(MemoryArena* arena, uptr renderBufferSize, u32 commandQueueCapacity);
};

void Push(RenderGroup* group, RenderCommandDrawMesh* command);
void Push(RenderGroup* group, RenderCommandSetDirLight* command);
void Push(RenderGroup* group, RenderCommandLineBegin* command);
void Push(RenderGroup* group, RenderCommandPushLineVertex* command);
void Push(RenderGroup* group, RenderCommandLineEnd* command);
void Push(RenderGroup* group, RenderCommandBeginChunkBatch* command);
void Push(RenderGroup* group, RenderCommandPushChunk* command);
void Push(RenderGroup* group, RenderCommandEndChunkBatch* command);

void Reset(RenderGroup* group);

void DrawAlignedBoxOutline(RenderGroup* renderGroup, v3 min, v3 max, v3 color, f32 lineWidth);
void DrawStraightLine(RenderGroup* renderGroup, v3 begin, v3 end, v3 color, f32 lineWidth);
