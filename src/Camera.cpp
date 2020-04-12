#include "Camera.h"

void GatherInput(Camera* camera) {
    auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
    auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
    DEBUG_OVERLAY_TRACE(z);
    DEBUG_OVERLAY_TRACE(x);

    camera->frameAcceleration = {};

    if (KeyHeld(Key::W))
    {
        camera->frameAcceleration -= z;
    }
    if (KeyHeld(Key::S))
    {
        camera->frameAcceleration += z;
    }
    if (KeyHeld(Key::A))
    {
        camera->frameAcceleration -= x;
    }
    if (KeyHeld(Key::D))
    {
        camera->frameAcceleration += x;
    }

    camera->frameAcceleration = Normalize(camera->frameAcceleration);
    camera->frameAcceleration *= camera->moveSpeed;

    if (MouseButtonHeld(MouseButton::Right))
    {
        v2 mousePos;
        f32 speed = camera->rotSpeed;
        mousePos.x = GlobalInput.mouseFrameOffsetX * speed;
        mousePos.y = GlobalInput.mouseFrameOffsetY * speed;
        camera->targetOrbit.x += mousePos.x;
        camera->targetOrbit.y -= mousePos.y;
    }

    camera->frameScrollOffset = GlobalInput.scrollFrameOffset;
}


void Update(Camera* camera, f32 dt) {
    GatherInput(camera);

    auto acceleration = camera->frameAcceleration - camera->velocity * camera->moveFriction;

    v3 movementDelta = 0.5f * acceleration * dt * dt + camera->velocity * dt;

    camera->targetWorldPosition = Offset(camera->targetWorldPosition, movementDelta);
    camera->velocity += acceleration * dt;

    camera->targetDistance -= camera->frameScrollOffset * camera->zoomSpeed * dt;
    camera->frameScrollOffset = {};

    camera->targetDistance = Clamp(camera->targetDistance, 5.0f, 100.0f);
    camera->targetOrbit.y = Clamp(camera->targetOrbit.y, 95.0f, 170.0f);

    camera->latitude = Lerp(camera->latitude, camera->targetOrbit.y, dt * camera->latSmooth);
    camera->longitude = Lerp(camera->longitude, camera->targetOrbit.x, dt * camera->longSmooth);
    camera->distance = Lerp(camera->distance, camera->targetDistance, dt * camera->distSmooth);

    f32 latitude = ToRad(camera->latitude);
    f32 longitude = ToRad(camera->longitude);
    f32 polarAngle = F32::Pi - latitude;

    f32 x = camera->distance * Sin(polarAngle) * Cos(longitude);
    f32 y = camera->distance * Cos(polarAngle);
    f32 z = camera->distance * Sin(polarAngle) * Sin(longitude);

    camera->position = V3(x, y, z);
    camera->front = Normalize(V3(x, y, z));

    v2 normMousePos;
    DEBUG_OVERLAY_TRACE(GlobalInput.mouseX);
    DEBUG_OVERLAY_TRACE(GlobalInput.mouseY);
    normMousePos.x = 2.0f * GlobalInput.mouseX - 1.0f;
    normMousePos.y = 2.0f * GlobalInput.mouseY - 1.0f;
    v4 mouseClip = V4(normMousePos, -1.0f, 0.0f);

    camera->viewMatrix = LookAtGLRH(camera->position, V3(0.0f), V3(0.0f, 1.0f, 0.0f));
    camera->projectionMatrix = PerspectiveGLRH(camera->nearPlane, camera->farPlane, camera->fovDeg, camera->aspectRatio);
    camera->invViewMatrix = Inverse(camera->viewMatrix);
    camera->invProjectionMatrix = Inverse(camera->projectionMatrix);

    v4 mouseView = camera->invProjectionMatrix * mouseClip;
    mouseView = V4(mouseView.xy, -1.0f, 0.0f);
    v3 mouseWorld = (camera->invViewMatrix * mouseView).xyz;
    mouseWorld = Normalize(mouseWorld);
    camera->mouseRay = V3(mouseWorld.x, mouseWorld.y, mouseWorld.z);
}
