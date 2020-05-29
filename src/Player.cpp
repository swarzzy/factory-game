#include "Player.h"

Entity* CreatePlayerEntity(GameWorld* world, WorldPos p, SimRegion* region, Camera* camera) {
    Player* entity = AddSpatialEntity<Player>(world, p);
    if (entity) {
        entity->type = EntityType::Player;
        entity->scale = 0.95f;
        entity->acceleration = 70.0f;
        entity->friction = 10.0f;
        entity->height = 1.8f;
        entity->selectedVoxel = GameWorld::InvalidPos;
        entity->jumpAcceleration = 420.0f;
        entity->runAcceleration = 140.0f;
        entity->region = region;
        entity->inventory = AllocateEntityInventory(16, 128);
        entity->camera = camera;
    }
    return entity;
}

void Player::Render(RenderGroup* group, Camera* camera) {
    auto context = GetContext();
    if (this->camera->mode != CameraMode::Gameplay) {
        RenderCommandDrawMesh command{};
        command.transform = Translate(WorldPos::Relative(camera->targetWorldPosition, this->p));
        command.mesh = context->playerMesh;
        command.material = &context->playerMaterial;
        Push(group, &command);
    }
}

void Player::Update(f32 deltaTime) {
    auto oldP = this->p;
    v3 frameAcceleration = {};

    f32 playerAcceleration;
    v3 drag = this->velocity * this->friction;

    auto z = Normalize(V3(camera->front.x, 0.0f, camera->front.z));
    auto x = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), z));
    auto y = V3(0.0f, 1.0f, 0.0f);

    if (this->camera->inputMode == GameInputMode::Game || this->camera->inputMode == GameInputMode::InGameUI) {

        if (KeyHeld(Key::W)) {
            frameAcceleration -= z;
        }
        if (KeyHeld(Key::S)) {
            frameAcceleration += z;
        }
        if (KeyHeld(Key::A)) {
            frameAcceleration -= x;
        }
        if (KeyHeld(Key::D)) {
            frameAcceleration += x;
        }

        if ((KeyHeld(Key::Space))) {
            //frameAcceleration += y;
        }

        if (KeyPressed(Key::Y)) {
            this->flightMode = !this->flightMode;
        }

        if (this->flightMode) {
            if (KeyHeld(Key::Space)) {
                frameAcceleration += y;
            }
            if (KeyHeld(Key::Ctrl)) {
                frameAcceleration -= y;
            }
        } else {
            drag.y = 0.0f;
        }

        if ((KeyHeld(Key::Shift))) {
            playerAcceleration = this->runAcceleration;
        } else {
            playerAcceleration = this->acceleration;
        }
        frameAcceleration *= playerAcceleration;
    }

    // TODO: Physically correct friction
    frameAcceleration -= drag;

    if (!this->flightMode) {
        if (camera->inputMode == GameInputMode::Game || camera->inputMode == GameInputMode::InGameUI) {
            if (KeyPressed(Key::Space) && this->grounded) {
                frameAcceleration += y * this->jumpAcceleration * (1.0f / deltaTime) / 60.0f;
            }
        }

        frameAcceleration.y += -20.8f;
    }


    v3 movementDelta = 0.5f * frameAcceleration * deltaTime * deltaTime + this->velocity * deltaTime;

    this->velocity += frameAcceleration * deltaTime;
    DEBUG_OVERLAY_TRACE(this->velocity);
    bool hitGround = false;
    MoveSpatialEntity(this->world, this, movementDelta, camera, nullptr);

    if (WorldPos::ToChunk(this->p).chunk != WorldPos::ToChunk(oldP).chunk) {
        MoveRegion(this->region, WorldPos::ToChunk(this->p).chunk);
    }
}
