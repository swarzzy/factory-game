#include "ConsoleCommands.h"
#include "Console.h"

void ConsoleClearCommand(Context* context, ConsoleCommandArgs* args) {
    ClearLogger(&context->logger);
}

void ConsoleHelpCommand(Context* context, ConsoleCommandArgs* args) {
    LogMessage(&context->logger, "Available commands are:\n");
    for (u32x i = 0; i < array_count(GlobalConsoleCommands); i++) {
        auto command = GlobalConsoleCommands + i;
        LogMessage(&context->logger, "%s\n", command->name);
    }
}

void ConsoleHistoryCommand(Context* context, ConsoleCommandArgs* args) {
    auto logger = &context->logger;
    auto console = &context->console;
    LogMessage(logger, "History of commands:\n");
    auto record = console->firstCommandRecord;
    while(record) {
        LogMessage(logger, "%s\n", record->string);
        record = record->next;
    }
}

void ConsoleEchoCommand(Context* context, ConsoleCommandArgs* args) {
    auto logger = &context->logger;
    if (args->args) {
        LogMessage(logger, "%s\n", args->args);
    }
}

void RecompileShadersCommand(Context* context, ConsoleCommandArgs* args) {
    RecompileShaders(context->tempArena, context->renderer);
}

void ResetPlayerPositionCommand(Context* context, ConsoleCommandArgs* args) {
    LogMessage(&context->logger, "Reset player position to (0, 30, 0)\n");
    context->camera.targetWorldPosition = WorldPos::Make(IV3(0, 15, 0));
    MoveRegion(&context->playerRegion, ChunkPosFromWorldPos(context->camera.targetWorldPosition.voxel).chunk);
    context->gameWorld.playerEntity.p = WorldPos::Make(IV3(0, 30, 0));
}
