#include "Pipe.h"
#include "World.h"
#include "RenderGroup.h"
#include "../ext/imgui/imgui.h"
#include "BinaryBlob.h"

void OrientPipe(GameWorld* world, Pipe* pipe);

void NeighborhoodChangedUpdate(Pipe* pipe) {
    auto world = GetWorld();
    OrientPipe(pipe->world, pipe);

    auto downBlock = GetBlock(pipe->world, pipe->p + IV3(0, -1, 0));
    if (downBlock.value == BlockValue::Water) {
        // TODO: Orient pipe to water
        pipe->filled = true;
        pipe->liquid = Liquid::Water;
        pipe->source = true;
    } else {
        pipe->filled = false;
        pipe->source = false;
    }
}

void PipeUpdateAndRender(Entity* _entity, EntityBehaviorInvoke reason, void* _data) {
    if (reason == EntityBehaviorInvoke::UpdateAndRender) {
        auto data = (EntityUpdateAndRenderData*)_data;
        auto pipe = (Pipe*)_entity;
        if (pipe->dirtyNeighborhood) {
            NeighborhoodChangedUpdate(pipe);
        }

        auto entity = pipe;
        if (pipe->source) {
            entity->amount = 0.01;
            entity->pressure = 2.0f;
        } else {
            f32 pressureSum = 0.0f;
            u32 connectionCount = 0;

            if (entity->nxConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p - IV3(1, 0, 0));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (entity->pxConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p + IV3(1, 0, 0));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (entity->pyConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p + IV3(0, 1, 0));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (entity->nyConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p - IV3(0, 1, 0));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (entity->pzConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p + IV3(0, 0, 1));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (entity->nzConnected) {
                BlockEntity* _neighbor = GetBlockEntity(entity->world, entity->p - IV3(0, 0, 1));
                if (_neighbor->type == EntityType::Pipe) {
                    auto neighbor = static_cast<Pipe*>(_neighbor);
                    if (neighbor && neighbor->liquid == entity->liquid) {
                        pressureSum += Clamp(neighbor->pressure - Pipe::PressureDrop, 0.0f, 999.0f);
                        connectionCount++;
                        if (neighbor->pressure > entity->pressure) {
                            f32 freeSpace = Pipe::MaxCapacity - entity->amount;
                            entity->amount = Clamp(neighbor->amount + entity->amount, 0.0f, Pipe::MaxCapacity);
                            neighbor->amount = Clamp(neighbor->amount - freeSpace, 0.0f, Pipe::MaxCapacity);
                        }
                    }
                }
            }
            if (connectionCount) {
                entity->pressure = pressureSum / connectionCount;
            } else {
                entity->pressure = 0.0f;
            }
        }

        auto context = GetContext();
        RenderCommandDrawMesh command {};
        command.transform = Translate(WorldPos::Relative(data->camera->targetWorldPosition, WorldPos::Make(entity->p))) * Rotate(entity->rotation);
        command.mesh = entity->mesh;
        command.material = &context->pipeMaterial;
        Push(data->group, &command);
    }
}

Entity* CreatePipeEntity(GameWorld* world, WorldPos p) {
    Pipe* pipe = AddBlockEntity<Pipe>(world, p.block);
    if (pipe) {
        pipe->type = EntityType::Pipe;
        pipe->flags |= EntityFlag_Collides;
        pipe->dirtyNeighborhood = true;
    }
    return pipe;
}

void PipeDelete(Entity* entity, GameWorld* world) {
}

void PipeDropPickup(Entity* entity, GameWorld* world, WorldPos p) {
    auto pickup = CreatePickup(p, (ItemID)Item::Pipe, 1);
}

void PipeUpdateAndRenderUI(Entity* entity, EntityUIInvoke reason) {
    auto pipe = (Pipe*)entity;
    ImGui::Text("source: %s", pipe->source ? "true" : "false");
    ImGui::Text("filled: %s", pipe->filled ? "true" : "false");
    ImGui::Text("liquid: %s", ToString(pipe->liquid));
    ImGui::Text("amount: %f", pipe->amount);
    ImGui::Text("pressure: %f", pipe->pressure);
    ImGui::Text("nx connection: %s", pipe->nxConnected ? "true" : "false");
    ImGui::Text("px connection: %s", pipe->pxConnected ? "true" : "false");
    ImGui::Text("ny connection: %s", pipe->nyConnected ? "true" : "false");
    ImGui::Text("py connection: %s", pipe->pyConnected ? "true" : "false");
    ImGui::Text("nz connection: %s", pipe->nzConnected ? "true" : "false");
    ImGui::Text("pz connection: %s", pipe->pzConnected ? "true" : "false");
}

// This proc is crazy mess for now. We need some smart algorithm for orienting pipes
void OrientPipe(GameWorld* world, Pipe* pipe) {
    auto context = GetContext();
    iv3 p = pipe->p;
    pipe->mesh = context->pipeStraightMesh;

    // TODO: Be carefull with pointers in blocks
    BlockEntity* westNeighbour = GetBlockEntity(world, p + IV3(-1, 0, 0));
    BlockEntity* eastNeighbour = GetBlockEntity(world, p + IV3(1, 0, 0));
    BlockEntity* northNeighbour = GetBlockEntity(world, p + IV3(0, 0, -1));
    BlockEntity* southNeighbour = GetBlockEntity(world, p + IV3(0, 0, 1));
    BlockEntity* upNeighbour = GetBlockEntity(world, p + IV3(0, 1, 0));
    BlockEntity* downNeighbour = GetBlockEntity(world, p + IV3(0, -1, 0));

    bool px = 0;
    bool nx = 0;
    bool py = 0;
    bool ny = 0;
    bool pz = 0;
    bool nz = 0;
    if (westNeighbour && westNeighbour->type == EntityType::Pipe) { nx = true; }
    if (eastNeighbour && eastNeighbour->type == EntityType::Pipe) { px = true; }
    if (northNeighbour && northNeighbour->type == EntityType::Pipe) { nz = true; }
    if (southNeighbour && southNeighbour->type == EntityType::Pipe) { pz = true; }
    if (upNeighbour && upNeighbour->type == EntityType::Pipe) { py = true; }
    if (downNeighbour && downNeighbour->type == EntityType::Pipe) { ny = true; }

    bool xConnected = px && nx;
    bool yConnected = py && ny;
    bool zConnected = pz && nz;

    pipe->nxConnected = false;
    pipe->pxConnected = false;
    pipe->nyConnected = false;
    pipe->pyConnected = false;
    pipe->nzConnected = false;
    pipe->pzConnected = false;

    // Check for crossing
    if (xConnected && zConnected) {
        pipe->rotation = V3(0.0f, 0.0f, 0.0f);
        pipe->mesh = context->pipeCrossMesh;
        pipe->nxConnected = true;
        pipe->pxConnected = true;
        pipe->nzConnected = true;
        pipe->pzConnected = true;
    } else if (xConnected && yConnected) {
        pipe->mesh = context->pipeCrossMesh;
        pipe->rotation = V3(90.0f, 0.0f, 0.0f);
        pipe->nxConnected = true;
        pipe->pxConnected = true;
        pipe->nyConnected = true;
        pipe->pyConnected = true;
    } else if (yConnected && zConnected) {
        pipe->mesh = context->pipeCrossMesh;
        pipe->rotation = V3(0.0f, 0.0f, 90.0f);
        pipe->nyConnected = true;
        pipe->pyConnected = true;
        pipe->nzConnected = true;
        pipe->pzConnected = true;
    } else {
        // Not a crossing
        if (xConnected) {
            pipe->nxConnected = true;
            pipe->pxConnected = true;
            // checking for tee
            // TODO: Check whether side pipe as a turn. If it is then place straight pipe instead of a tee
            if (ny) {
                pipe->nyConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(180.0f, 0.0f, 0.0f);
            } else if (py) {
                pipe->pyConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 0.0f, 0.0f);
            } else if (nz) {
                pipe->nzConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(90.0f, 0.0f, 0.0f);
            } else if (pz) {
                pipe->pzConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(-90.0f, 0.0f, 0.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->rotation = V3(0.0f, 0.0f, 0.0f);
            }
        } else if (yConnected) {
            pipe->nyConnected = true;
            pipe->pyConnected = true;
            // checking for tee
            if (nx) {
                pipe->nxConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 0.0f, -90.0f);
            } else if (px) {
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 0.0f, 90.0f);
            } else if (nz) {
                pipe->nzConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(90.0f, 0.0f, 90.0f);
            } else if (pz) {
                pipe->pzConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(-90.0f, 0.0f, 90.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->rotation = V3(0.0f, 0.0f, 90.0f);
            }
        } else if (zConnected) {
            pipe->nzConnected = true;
            pipe->pzConnected = true;
            // checking for tee
            if (nx) {
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 90.0f, -90.0f);
            } else if (px) {
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 90.0f, 90.0f);
            } else if (ny) {
                pipe->nyConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 90.0f, 180.0f);
            } else if (py) {
                pipe->pyConnected = true;
                pipe->mesh = context->pipeTeeMesh;
                pipe->rotation = V3(0.0f, 90.0f, 0.0f);
            } else { // Straight pipe
                pipe->mesh = context->pipeStraightMesh;
                pipe->rotation = V3(0.0f, 90.0f, 0.0f);
            }
        } else { // Turn or staight end
            // checking for turn
            if (nx && nz) {
                pipe->nzConnected = true;
                pipe->nxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, -270.0f, 0.0f);
            } else if (px && nz) {
                pipe->nzConnected = true;
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, 0.0f, 0.0f);
            } else if (px && pz) {
                pipe->pzConnected = true;
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, -90.0f, 0.0f);
            } else if (nx && pz) {
                pipe->pzConnected = true;
                pipe->nxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, -180.0f, 0.0f);
            } else if (nx && py) {
                pipe->pyConnected = true;
                pipe->nxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(-90.0f, 180.0f, 0.0f);
            } else if (nz && py) {
                pipe->pyConnected = true;
                pipe->nzConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, 0.0f, -90.0f);
            } else if (px && py) {
                pipe->pyConnected = true;
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(-90.0f, 0.0f, 0.0f);
            } else if (pz && py) {
                pipe->pyConnected = true;
                pipe->pzConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(-90.0f, -90.0f, 0.0f);
            } else if (nx && ny) {
                pipe->nyConnected = true;
                pipe->nxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(90.0f, 180.0f, 0.0f);
            } else if (nz && ny) {
                pipe->nyConnected = true;
                pipe->nzConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(0.0f, 0.0f, 90.0f);
            } else if (px && ny) {
                pipe->nyConnected = true;
                pipe->pxConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(90.0f, 0.0f, 0.0f);
            } else if (pz && ny) {
                pipe->nyConnected = true;
                pipe->pzConnected = true;
                pipe->mesh = context->pipeTurnMesh;
                pipe->rotation = V3(90.0f, -90.0f, 0.0f);
            } else { // straight end
                if (px || nx) {
                    pipe->pxConnected = px;
                    pipe->nxConnected = nx;
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->rotation = V3(0.0f, 0.0f, 0.0f);
                } else if (py || ny) {
                    pipe->pyConnected = py;
                    pipe->nyConnected = ny;
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->rotation = V3(0.0f, 0.0f, 90.0f);
                } else if (pz || nz) {
                    pipe->pzConnected = pz;
                    pipe->nzConnected = nz;
                    pipe->mesh = context->pipeStraightMesh;
                    pipe->rotation = V3(0.0f, 90.0f, 0.0f);
                }
            }
        }
    }
}

void PipeSerialize(Entity* entity, BinaryBlob* out) {
    auto self = (Pipe*)entity;
    WriteField(out, &self->filled);
    WriteField(out, &self->liquid);
    WriteField(out, &self->amount);
    WriteField(out, &self->pressure);
}

void PipeDeserialize(Entity* entity, EntitySerializedData data) {
    auto self = (Pipe*)entity;
    ReadField(&data, &self->filled);
    ReadField(&data, &self->liquid);
    ReadField(&data, &self->amount);
    ReadField(&data, &self->pressure);
    //NeighborhoodChangedUpdate(self);
    self->dirtyNeighborhood = true;
    PostEntityNeighborhoodUpdate(self->world, self);
}
