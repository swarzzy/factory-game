#pragma once

#include "World.h"

struct CameraBase {
    v3 position;
    v3 front = V3(0.0f, 0.0f, 1.0f);
    f32 fovDeg = 45.0f;
    f32 aspectRatio = 16.0f / 9.0f;
    f32 nearPlane = 0.01f;
    f32 farPlane = 400.0f;
    v3 mouseRay;
    // NOTE: OpenGL conformant
    m4x4 viewMatrix;
    m4x4 invViewMatrix;
    m4x4 projectionMatrix;
    m4x4 invProjectionMatrix;
};

struct Camera : public CameraBase {
    //WorldPos targetPosition;
    WorldPos targetWorldPosition;
    f32 longitude;
    f32 latitude;
    f32 distance;
    v2 targetOrbit;
    f32 targetDistance;
    f32 rotSpeed = 1000.0f;
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

void Update(Camera* camera, f32 dt);
