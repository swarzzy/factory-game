#pragma once

#include "RenderGroup.h"
#include "World.h"

struct Player;

enum struct CameraMode {
    DebugFree, DebugFollowing, Gameplay
};

enum struct GameInputMode {
    UI,       // UI captures keyboard and mouse input
    Game,     // All input is passed to game
    InGameUI  // UI captures mouse, but keyboard input is still passed to game
};

struct Camera : public CameraBase {
    GameInputMode inputMode;
    CameraMode mode;
    //WorldPos targetPosition;
    WorldPos targetWorldPosition;
    f32 targetPitch;
    f32 targetYaw;
    f32 pitch;
    f32 yaw;
    f32 longitude;
    f32 latitude;
    f32 distance;
    v2 targetOrbit;
    f32 targetDistance;
    f32 rotSpeed = 165.0f;
    f32 zoomSpeed = 200.0f;
    f32 latSmooth = 30.0f;
    f32 longSmooth = 30.0f;
    f32 distSmooth = 30.0f;
    f32 moveSpeed = 500.0f;
    f32 moveFriction = 10.0f;
    v3 velocity;
    v3 frameAcceleration;
    i32 frameScrollOffset;
};

void Update(Camera* camera, Player* player, f32 dt);
