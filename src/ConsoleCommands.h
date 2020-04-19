#pragma once

struct Context;
struct ConsoleCommandArgs;

void ConsoleClearCommand(Context* context, ConsoleCommandArgs* args);
void ConsoleHelpCommand(Context* context, ConsoleCommandArgs* args);
void ConsoleHistoryCommand(Context* context, ConsoleCommandArgs* args);
void ConsoleEchoCommand(Context* context, ConsoleCommandArgs* args);


void RecompileShadersCommand(Context* context, ConsoleCommandArgs* args);
void ResetPlayerPositionCommand(Context* context, ConsoleCommandArgs* args);
