#pragma once

struct Context;
struct Console;
struct ConsoleCommandArgs;

void ConsoleClearCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ConsoleHelpCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ConsoleHistoryCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ConsoleEchoCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ConsoleSetCommand(Console* console, Context* context, ConsoleCommandArgs* args);

void RecompileShadersCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ResetPlayerPositionCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void CameraCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void AddEntityCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void PrintEntitiesCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void ToggleDebugOverlayCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void SetEntityPosCommand(Console* console, Context* context, ConsoleCommandArgs* args);
void PrintPlayerInventoryCommand(Console* console, Context* context, ConsoleCommandArgs* args);
