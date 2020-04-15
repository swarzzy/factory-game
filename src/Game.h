#pragma once
#include "Camera.h"
#include "Renderer.h"
#include "RenderGroup.h"
#include "World.h"
#include "MeshGenerator.h"

struct Context {
    MemoryArena* gameArena;
    MemoryArena* tempArena;
    ChunkMesher chunkMesher;
    GameWorld gameWorld;
    Mesh* playerMesh;
    Material playerMaterial;
    Camera camera;
    Renderer* renderer;
    RenderGroup renderGroup;
    CubeTexture skybox;
    CubeTexture hdrMap;
    CubeTexture irradanceMap;
    CubeTexture enviromentMap;
};

void FluxInit(Context* context);
void FluxReload(Context* context);
void FluxUpdate(Context* context);
void FluxRender(Context* context);
