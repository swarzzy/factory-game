#include "Pickup.h"
#include "RenderGroup.h"

void PickupEntity::Render(RenderGroup* renderGroup, Camera* camera) {
    auto context = GetContext();

    RenderCommandDrawMesh command{};
    command.transform = Translate(WorldPos::Relative(camera->targetWorldPosition, this->p));
    switch (this->item) {
        case Item::CoalOre: {
            command.mesh = context->coalOreMesh;
            command.material = &context->coalOreMaterial;
        } break;
        case Item::Container: {
            command.mesh = context->containerMesh;
            command.material = &context->containerMaterial;
            command.transform = command.transform * Scale(V3(0.2));
        } break;
        case Item::Pipe: {
            command.mesh = context->pipeStraightMesh;
            command.material = &context->pipeMaterial;
            command.transform = command.transform * Scale(V3(0.2));
        } break;

            invalid_default();
    }
    Push(renderGroup, &command);
}
