#pragma once
#include "Camera.h"
#include "Renderer.h"
#include "RenderGroup.h"
#include "World.h"
#include "MeshGenerator.h"
#include "ChunkPool.h"
#include "Console.h"
#include "UI.h"
#include "DebugUI.h"
#include "Debug.h"
#include "EntityInfo.h"

struct Context {
    EntityInfo entityInfo;
    MemoryArena* gameArena;
    MemoryArena* tempArena;
    ChunkMesher chunkMesher;
    GameWorld gameWorld;
    UI ui;
    DebugUI debugUI;
    Mesh* cubeMesh;
    Material playerMaterial;
    Mesh* coalOreMesh;
    Material coalOreMaterial;
    Mesh* containerMesh;
    Texture containerAlbedo;
    Texture containerMetallic;
    Texture containerNormal;
    Texture containerAO;
    Material containerMaterial;
    Texture pipeAlbedo;
    Texture pipeMetallic;
    Texture pipeRoughness;
    Texture pipeNormal;
    Texture pipeAO;
    Material pipeMaterial;
    Texture barrelAlbedo;
    Texture barrelRoughness;
    Texture barrelNormal;
    Texture barrelAO;
    Material barrelMaterial;
    Mesh* pipeStraightMesh;
    Mesh* pipeTurnMesh;
    Mesh* pipeCrossMesh;
    Mesh* pipeTeeMesh;
    Mesh* barrelMesh;
    Mesh* beltStraightMesh;
    Material beltMaterial;
    Texture beltIcon;
    Texture beltDiffuse;
    Texture coalIcon;
    Texture containerIcon;
    Texture extractorIcon;
    Texture pipeIcon;
    Mesh* extractorMesh;
    Material extractorMaterial;
    Texture extractorDiffuse;

    Texture stoneDiffuse;
    Texture grassDiffuse;
    Texture coalOreBlockDiffuse;
    Texture waterDiffuse;
    Material stoneMaterial;
    Material grassMaterial;
    Material coalOreBlockMaterial;
    Material waterMaterial;

    Texture grenadeAlbedo;
    Texture grenadeMetallic;
    Texture grenadeRoughness;
    Texture grenadeNormal;
    Texture grenadeAO;
    Material grenadeMaterial;
    Texture grenadeIcon;
    Mesh* grenadeMesh;



    Camera camera;
    Renderer* renderer;
    RenderGroup renderGroup;
    //CubeTexture skybox;
    CubeTexture hdrMap;
    CubeTexture irradanceMap;
    CubeTexture enviromentMap;
    b32 consoleEnabled;
    Logger logger;
    Console console;
    ProfilerBuffer debugProfiler;
};

void FluxInit(Context* context);
void FluxReload(Context* context);
void FluxUpdate(Context* context);
void FluxRender(Context* context);
