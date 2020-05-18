#include "ConsoleCommands.h"
#include "Console.h"

#define REGISTER_SET_FLOAT(name, variable) \
if (StringsAreEqual(varName, name)) {\
    variableRecognized = true;                          \
    const char* varValue = PullCommandArg(args);\
    if (varValue) {\
        auto parseResult = StringToFloat(varValue);\
        if (parseResult.succeed) {\
            LogMessage(console->logger, "Setting a value of %s to %f. Old value was %f\n", name, parseResult.value, variable);\
            variable = parseResult.value;\
            return;\
        } else {\
            LogMessage(console->logger, "Failed to parse a value\n");\
        }\
    } else {\
        LogMessage(console->logger, "No value specified\n");\
    }\
} do {} while(false)\

#define REGISTER_SET_IV3(name, variable) \
if (StringsAreEqual(varName, name)) {       \
    variableRecognized = true;                          \
    const char* varValue1 = PullCommandArg(args);       \
    const char* varValue2 = PullCommandArg(args);       \
    const char* varValue3 = PullCommandArg(args);       \
    if (varValue1 && varValue2 && varValue3) {\
        auto parseResult1 = StringToInt(varValue1);\
        auto parseResult2 = StringToInt(varValue2);\
        auto parseResult3 = StringToInt(varValue3);\
        if (parseResult1.succeed && parseResult2.succeed && parseResult3.succeed) {\
            LogMessage(console->logger, "Setting a value of %s to (%d, %d, %d). Old value was (%d, %d, %d)\n", name, (int)parseResult1.value, (int)parseResult2.value, (int)parseResult2.value, (int)variable.x, (int)variable.y, (int)variable.z); \
            variable.x = parseResult1.value;\
            variable.y = parseResult2.value;\
            variable.z = parseResult3.value;\
            return;\
        } else {\
            LogMessage(console->logger, "Failed to parse a value\n");\
        }\
    } else {\
        LogMessage(console->logger, "No value specified\n");\
    }\
} do {} while(false)\



void ConsoleSetCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    const char* varName = PullCommandArg(args);
    if (varName) {
        bool variableRecognized = false;
        // NOTE: Register variables here
        REGISTER_SET_FLOAT("playerRunSpeed", context->gameWorld.player.runAcceleration);
        //REGISTER_SET_IV3("playerP", context->gameWorld.playerEntity.p.voxel);

        if (!variableRecognized) {
            LogMessage(console->logger, "Unknown variable name %s\n", varName);
        }
    } else {
        LogMessage(console->logger, "No variable specified\n");
    }
}

void ConsoleClearCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    ClearLogger(console->logger);
}

void ConsoleHelpCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    if (args->args) {
        bool found = false;
        for (u32x i = 0; i < array_count(GlobalConsoleCommands); i++) {
            auto command = GlobalConsoleCommands + i;
            if (MatchStrings(command->name, args->args)) {
                found = true;
                LogMessage(console->logger, "%s", command->name);
                if (command->description) {
                    LogMessage(console->logger, " - %s\n", command->description);
                } else {
                    LogMessage(console->logger, "\n");
                }
            }
        }
        if (!found) {
            LogMessage(console->logger, "Can't help you. Commad %s is not exist\n", args->args);
        }
    } else {
        LogMessage(console->logger, "Available commands are:\n");
        for (u32x i = 0; i < array_count(GlobalConsoleCommands); i++) {
            auto command = GlobalConsoleCommands + i;
            LogMessage(console->logger, "%s\n", command->name);
        }
    }
}

void ConsoleHistoryCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto logger = console->logger;
    LogMessage(logger, "History of commands:\n");
    auto record = console->firstCommandRecord;
    while(record) {
        LogMessage(logger, "%s\n", record->string);
        record = record->next;
    }
}

void ConsoleEchoCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto logger = console->logger;
    if (args->args) {
        LogMessage(logger, "%s\n", args->args);
    }
}

void RecompileShadersCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    RecompileShaders(context->tempArena, context->renderer);
}

void ResetPlayerPositionCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    LogMessage(console->logger, "Reset player position to (0, 30, 0)\n");
    context->camera.targetWorldPosition = MakeWorldPos(IV3(0, 15, 0));
    MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(context->camera.targetWorldPosition.voxel).chunk);
    // TODO: Make this work
    //context->gameWorld.playerEntity.p = MakeWorldPos(IV3(0, 30, 0));
}

void CameraCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    if (StringsAreEqual(args->args, "free")) {
        LogMessage(console->logger, "Camera mode changed to free\n");
        context->camera.mode = CameraMode::DebugFree;
        GlobalPlatform.inputMode = InputMode::FreeCursor;
    } else if (StringsAreEqual(args->args, "follow")) {
        LogMessage(console->logger, "Camera mode changed to follow\n");
        context->camera.mode = CameraMode::DebugFollowing;
        GlobalPlatform.inputMode = InputMode::FreeCursor;
    } else if (StringsAreEqual(args->args, "game")) {
        LogMessage(console->logger, "Camera mode changed to game\n");
        context->camera.mode = CameraMode::Gameplay;
        GlobalPlatform.inputMode = InputMode::CaptureCursor;
    } else {
        LogMessage(console->logger, "Unknown camera mode\n");
    }
}

void AddEntityCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto chunk = GetChunk(&context->gameWorld, 0, 0, 0);
    if (chunk) {
        auto entity = AddSpatialEntity(&context->gameWorld, IV3(0, 30, 0));
        if (entity) {
            entity->scale = 0.4f;
            entity->type = SpatialEntityType::Pickup;
            entity->pickupItem = Item::CoalOre;
            LogMessage(console->logger, "Entity with id %llu added\n", entity->id.id);
        } else {
            LogMessage(console->logger, "Failed to add entity. AddSpatialEntity() failed\n", entity->id.id);
        }
    } else {
        LogMessage(console->logger, "Failed to add entity. Chunk is not exist\n");
    }
}

void SetEntityPosCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto arg1 = PullCommandArg(args);
    auto arg2 = PullCommandArg(args);
    auto arg3 = PullCommandArg(args);
    auto arg4 = PullCommandArg(args);

    if (arg1 && arg2 && arg3 && arg4) {
        auto arg1Value = StringToInt(arg1);
        auto arg2Value = StringToInt(arg2);
        auto arg3Value = StringToInt(arg3);
        auto arg4Value = StringToInt(arg4);
        if (arg1Value.succeed && arg2Value.succeed && arg3Value.succeed && arg4Value.succeed) {
            auto id = arg1Value.value;
            auto entity = GetSpatialEntity(&context->playerRegion, EntityID {(u64)id});
            if (entity) {
                entity->p = MakeWorldPos(IV3(arg2Value.value, arg3Value.value, arg4Value.value));
            } else {
                LogMessage(console->logger, "Entity with id %lu not found\n", id);
            }
        } else {
            LogMessage(console->logger, "Invalid args\n");
        }
    } else {
        LogMessage(console->logger, "Invalid args\n");
    }
}

void PrintEntitiesCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto region = &context->playerRegion;
    LogMessage(console->logger, "Entities in region:\n");
    for (auto entity : region->spatialEntityTable) {
        if (entity->type == SpatialEntityType::Pickup) {
            LogMessage(console->logger, "%llu: %s:%s at position (%d, %d, %d)\n", entity->id.id, ToString(entity->type), ToString(entity->pickupItem), entity->p.voxel.x, entity->p.voxel.y, entity->p.voxel.z);
        } else {
            LogMessage(console->logger, "%llu: %s at position (%d, %d, %d)\n", entity->id.id, ToString(entity->type), entity->p.voxel.x, entity->p.voxel.y, entity->p.voxel.z);
        }
    }
}

void ToggleDebugOverlayCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    GlobalDrawDebugOverlay = !GlobalDrawDebugOverlay;
}

void PrintPlayerInventoryCommand(Console* console, Context* context, ConsoleCommandArgs* args) {
    auto region = &context->playerRegion;
    auto player = GetSpatialEntity(region, context->gameWorld.player.entityID);
    assert(player);
    assert(player->inventory);
    auto inventory = player->inventory;
    LogMessage(console->logger, "Player inventory:\n");
    for (usize i = 0; i < inventory->slotCount; i++) {
        auto slot = inventory->slots + i;
        if (slot->item != Item::None) {
            LogMessage(console->logger, "%s: %lu\n", ToString(slot->item), slot->count);
        }
    }
}
