#include "Camera.h"

void GatherInput(Camera* camera) {
    auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
    auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
    auto y = V3(0.0f, 1.0f, 0.0f);
    DEBUG_OVERLAY_TRACE(z);
    DEBUG_OVERLAY_TRACE(x);

    camera->frameAcceleration = {};
    if (KeyHeld(Key::W)) {
        camera->frameAcceleration -= z;
    }
    if (KeyHeld(Key::S)) {
        camera->frameAcceleration += z;
    }
    if (KeyHeld(Key::A)) {
        camera->frameAcceleration -= x;
    }
    if (KeyHeld(Key::D)) {
        camera->frameAcceleration += x;
    }
    if ((KeyHeld(Key::Space))) {
        camera->frameAcceleration += y;
    }
    if ((KeyHeld(Key::Shift))) {
        camera->frameAcceleration -= y;
    }

    camera->frameAcceleration = Normalize(camera->frameAcceleration);
    camera->frameAcceleration *= camera->moveSpeed;

    v2 mousePos;
    f32 speed = camera->rotSpeed;
    mousePos.x = GlobalInput.mouseFrameOffsetX * speed;
    mousePos.y = GlobalInput.mouseFrameOffsetY * speed;
    camera->targetYaw += mousePos.x;
    camera->targetPitch -= mousePos.y;

    if (MouseButtonHeld(MouseButton::Right))
    {
        camera->targetOrbit.x += mousePos.x;
        camera->targetOrbit.y -= mousePos.y;
    }

    camera->frameScrollOffset = GlobalInput.scrollFrameOffset;
}


void Update(Camera* camera, Player* player, f32 dt) {
    GatherInput(camera);

    if (camera->mode == CameraMode::DebugFree || camera->mode == CameraMode::DebugFollowing){
        if (camera->mode == CameraMode::DebugFree) {
            auto acceleration = camera->frameAcceleration - camera->velocity * camera->moveFriction;

            v3 movementDelta = 0.5f * acceleration * dt * dt + camera->velocity * dt;

            camera->targetWorldPosition = Offset(camera->targetWorldPosition, movementDelta);
            camera->velocity += acceleration * dt;
        } else {
            camera->targetWorldPosition = player->entity->p;
        }

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

    } else {
        camera->targetWorldPosition = player->entity->p;

        camera->targetPitch = Clamp(camera->targetPitch, -89.0f, 89.0f);
        if (camera->targetYaw > 360.0f) {
            camera->targetYaw -= 360.0f;
        }

        camera->pitch = Lerp(camera->pitch, camera->targetPitch, 1.0f);
        camera->yaw = Lerp(camera->yaw, camera->targetYaw, 1.0f);

        f32 pitch = camera->pitch;
        f32 yaw = camera->yaw;

        v3 front;
         front.x = Cos(ToRad(pitch)) * Cos(ToRad(yaw));
         front.y = Sin(ToRad(pitch));
         front.z = Cos(ToRad(pitch)) * Sin(ToRad(yaw));

         camera->position = V3(0.0f, player->height - Voxel::HalfDim, 0.0f);
         camera->front = Normalize(front);
    }

    v2 normMousePos;
    DEBUG_OVERLAY_TRACE(GlobalInput.mouseX);
    DEBUG_OVERLAY_TRACE(GlobalInput.mouseY);
    normMousePos.x = 2.0f * GlobalInput.mouseX - 1.0f;
    normMousePos.y = 2.0f * GlobalInput.mouseY - 1.0f;
    v4 mouseClip = V4(normMousePos, -1.0f, 0.0f);

    camera->viewMatrix = LookAtGLRH(camera->position, camera->front, V3(0.0f, 1.0f, 0.0f));
    camera->projectionMatrix = PerspectiveGLRH(camera->nearPlane, camera->farPlane, camera->fovDeg, camera->aspectRatio);
    camera->invViewMatrix = Inverse(camera->viewMatrix);
    camera->invProjectionMatrix = Inverse(camera->projectionMatrix);

    v4 mouseView = camera->invProjectionMatrix * mouseClip;
    mouseView = V4(mouseView.xy, -1.0f, 0.0f);
    v3 mouseWorld = (camera->invViewMatrix * mouseView).xyz;
    mouseWorld = Normalize(mouseWorld);
    camera->mouseRay = V3(mouseWorld.x, mouseWorld.y, mouseWorld.z);
}
