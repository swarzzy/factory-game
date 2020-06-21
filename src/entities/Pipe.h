#pragma once

#include "Entity.h"

struct Pipe : BlockEntity {
    v3 rotation;
    Mesh* mesh;

    b32 nxConnected;
    b32 pxConnected;
    b32 nyConnected;
    b32 pyConnected;
    b32 nzConnected;
    b32 pzConnected;

    inline static const f32 MaxCapacity = 2.0f;
    inline static const f32 PressureDrop = 0.0001f;
    b32 source;
    b32 filled;
    Liquid liquid;
    f32 amount;
    f32 pressure;
};

Entity* CreatePipeEntity(GameWorld* world, WorldPos p);
void PipeDelete(Entity* entity, GameWorld* world);
void PipeUpdateAndRender(Entity* entity, EntityBehaviorInvoke reason, void* data);
void PipeDropPickup(Entity* entity, GameWorld* world, WorldPos p);
void PipeUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason);
