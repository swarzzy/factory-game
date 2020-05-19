#pragma once
#include "Camera.h"
#include "Renderer.h"
#include "RenderGroup.h"
#include "World.h"
#include "MeshGenerator.h"
#include "SimRegion.h"
#include "Console.h"

struct Context {
    MemoryArena* gameArena;
    MemoryArena* tempArena;
    ChunkMesher chunkMesher;
    GameWorld gameWorld;
    SimRegion playerRegion;
    Mesh* playerMesh;
    Material playerMaterial;
    Mesh* coalOreMesh;
    Material coalOreMaterial;
    Mesh* containerMesh;
    Texture containerAlbedo;
    Texture containerMetallic;
    Texture containerNormal;
    Texture containerAO;
    Material containerMaterial;
    Camera camera;
    Renderer* renderer;
    RenderGroup renderGroup;
    CubeTexture skybox;
    CubeTexture hdrMap;
    CubeTexture irradanceMap;
    CubeTexture enviromentMap;
    b32 consoleEnabled;
    Logger logger;
    Console console;
};

void FluxInit(Context* context);
void FluxReload(Context* context);
void FluxUpdate(Context* context);
void FluxRender(Context* context);
