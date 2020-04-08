#pragma once

struct GameWorld;
struct RenderGroup;
struct Camera;

struct Region {
    GameWorld* world;
    iv3 origin;
    i32 dim;
    bool debugShowBoundaries;
    bool debugRender;
};

constexpr u32 RegionBlockCount(u32 dim) {
    u32 span = dim * 2 + 1;
    u32 count = span * span * span;
    return count;
}

Region BeginRegion(GameWorld* world, iv3 origin, i32 dim);
void DrawRegion(Region* region, RenderGroup* renderGroup, Camera* camera);
