#include "Console.h"
#include "String.h"

void LoggerGrowBuffer(Logger* logger) {
    LoggerBufferBlock* newBlock = nullptr;
    bool reused = false;
    if (logger->freeBlock) {
        // Taking a block from a freelist
        newBlock = logger->freeBlock;
        logger->freeBlock = newBlock->next;
        reused = true;
        logger->usedBlockCount++;
    } else if (logger->allocatedBlockCount < Logger::BlockBudget) {
        // Allocating a new block
        newBlock = (LoggerBufferBlock*)PushSize(logger->arena, sizeof(LoggerBufferBlock));
        logger->allocatedBlockCount++;
        logger->usedBlockCount++;
    } else {
        // Reuse the first block in buffer
        reused = true;
        if (logger->firstBlock == logger->lastBlock) {
            newBlock = logger->firstBlock;
        } else {
            newBlock = logger->firstBlock;
            // There are should be blocks taht was allocated previously
            assert(newBlock);
            auto secondBlock = logger->firstBlock->next;
            logger->firstBlock = secondBlock;
            secondBlock->prev = nullptr;
        }
    }

    if (reused) {
        newBlock->at = 0;
        newBlock->data[0] = 0;
        newBlock->next = nullptr;
        newBlock->prev = nullptr;
    }

    assert(newBlock);
    auto last = logger->lastBlock;
    logger->lastBlock = newBlock;
    newBlock->prev = last;
    if (last) {
        last->next = newBlock;
    } else {
        assert(!logger->firstBlock);
        logger->firstBlock = newBlock;
    }
    newBlock->data[LoggerBufferBlock::Capacity] = 0;
}

void InitLogger(Logger* logger, MemoryArena* arena) {
    logger->arena = arena;
    LoggerGrowBuffer(logger);
}

void ClearLogger(Logger* logger) {
    logger->freeBlock = logger->firstBlock;
    logger->firstBlock = nullptr;
    logger->lastBlock = nullptr;
    logger->usedBlockCount = 0;
    LoggerGrowBuffer(logger);
}

void LoggerPushString(Logger* logger, const char* string) {
    const char* at = string;
    while (*at) {
        if (logger->lastBlock->at == LoggerBufferBlock::Capacity) {
            LoggerGrowBuffer(logger);
        }
        logger->lastBlock->data[logger->lastBlock->at++] = *at;
        at++;
    }
    if (logger->lastBlock->at != LoggerBufferBlock::Capacity) {
        logger->lastBlock->data[logger->lastBlock->at] = 0;
    }
}

void LogMessageAPI(void* loggerData, const char* fmt, va_list* args) {
    auto logger = (Logger*)loggerData;
    vsnprintf(logger->formatBuffer, array_count(logger->formatBuffer), fmt, *args);
    LoggerPushString(logger, logger->formatBuffer);
    fputs(logger->formatBuffer, stdout);
    logger->formatBuffer[0] = 0;
}

void LogMessage(Logger* logger, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessageAPI(logger, fmt, &args);
    va_end(args);
}

void LogMessageAPI(void* loggerData, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessageAPI((Logger*)loggerData, fmt, &args);
    va_end(args);
}

void InitConsole(Console* console, Logger* logger, MemoryArena* arena, Context* gameContext) {
    console->logger = logger;
    console->arena = arena;
    console->autoScrollEnabled = true;
    console->gameContext = gameContext;
}

const char* PullCommandArg(ConsoleCommandArgs* args) {
    const char* result = nullptr;
    if (args->args) {
        if (*(args->args)) {
            const char* begin = args->args;
            auto at = args->args;
            // Lookin' for space or null
            while (*(args->args) && !IsSpace(*(args->args))) args->args++;
            if (*(args->args)) {
                *(args->args) = 0;
                args->args++;
            }
            // SkipSpaces
            while (*(args->args) && IsSpace(*(args->args))) args->args++;
            result = begin;
        }
    }
    return result;
}

bool MatchAndExecuteCommand(Console* console) {
    bool executed = false;
    for (u32x i = 0; i < array_count(GlobalConsoleCommands); i++) {
        auto command = GlobalConsoleCommands + i;
        if (MatchStrings(command->name, console->inputBuffer)) {
            char* args = console->inputBuffer;
            u32 p = 0;
            // Skip untill space
            while (*args && (p < array_count(console->inputBuffer)) && !IsSpace(*args)) {
                p++;
                args++;
            }
            // Skip spaces
            while (*args && (p < array_count(console->inputBuffer)) && IsSpace(*args)) {
                p++;
                args++;
            }
            if (!(*args)) {
                args = nullptr;
            }

            ConsoleCommandArgs commandArgs = {};
            commandArgs.args = args;
            command->command(console, console->gameContext, &commandArgs);
            executed = true;
            break;
        }
    }
    return executed;
}

u32 MatchCount(const char* a, const char* b) {
    u32 count = 0;
    while(*a) {
        if (!(*b)) break;
        if (*a != *b) {
            break;
        }
        count++;
        a++;
        b++;
    }
    return count;
}

const char* ConsoleFindBestMathCommand(Console* console) {
    const char* result = nullptr;
    auto input = console->inputBuffer;
    u32 bestWeight = 0;
    u32 bestIndex = 0;
    for (u32x i = 0; i < array_count(GlobalConsoleCommands); i++) {
        auto command = GlobalConsoleCommands + i;
        u32 weight = MatchCount(command->name, input);
        if (weight > bestWeight) {
            bestWeight = weight;
            bestIndex = i;
        }
    }

    if (bestWeight) {
        auto command = GlobalConsoleCommands + bestIndex;
        result = command->name;
    }
    return result;
}

void PushConsoleCommandToHistory(Console* console) {
    const char* string = console->inputBuffer;
    auto length = strlen(string) + 1;
    auto record = (ConsoleCommandRecord*)PushSize(console->arena, sizeof(ConsoleCommandRecord));
    if (record) {
        record->string = (char*)PushSize(console->arena, sizeof(char) * length);
        if (record->string) {
            memcpy(record->string, string, length);
            auto prev = console->lastCommandRecord;
            console->lastCommandRecord = record;
            record->next = nullptr;
            record->prev = prev;
            if (prev) {
                prev->next = record;
            } else {
                assert(!console->firstCommandRecord);
                console->firstCommandRecord = record;
            }
            console->commandHistoryCount++;
        }
    }
}

int ConsoleInputCallback(ImGuiInputTextCallbackData* data) {
    auto console = (Console*)data->UserData;
    if (data->EventFlag & ImGuiInputTextFlags_CallbackHistory) {
        if (ImGui::IsKeyPressedMap(ImGuiKey_UpArrow)) {
            if (!console->commandHistoryCursor) {
                console->commandHistoryCursor = console->lastCommandRecord;
            } else if (console->commandHistoryCursor) {
                console->commandHistoryCursor = console->commandHistoryCursor->prev;
                if (!console->commandHistoryCursor) {
                    console->commandHistoryCursor = console->lastCommandRecord;
                }
            }
        }
        if (ImGui::IsKeyPressedMap(ImGuiKey_DownArrow)) {
            if (!console->commandHistoryCursor) {
                console->commandHistoryCursor = console->firstCommandRecord;
            } else if (console->commandHistoryCursor) {
                console->commandHistoryCursor = console->commandHistoryCursor->next;
                if (!console->commandHistoryCursor) {
                    console->commandHistoryCursor = console->firstCommandRecord;
                }
            }
        }
        if (console->commandHistoryCursor) {
            data->DeleteChars(0, sizeof(char) * data->BufTextLen);
            data->InsertChars(0, console->commandHistoryCursor->string);
        }
    } else if (data->EventFlag & ImGuiInputTextFlags_CallbackCompletion) {
        if (ImGui::IsKeyPressedMap(ImGuiKey_Tab)) {
            const char* command = ConsoleFindBestMathCommand(console);
            if (command) {
                data->DeleteChars(0, sizeof(char) * data->BufTextLen);
                data->InsertChars(0, command);
            }
        }
    }
    return 0;
}

void DrawConsole(Console* console) {
    auto windowWidth = GlobalPlatform.windowWidth;
    auto windowHeight = GlobalPlatform.windowHeight;

    ImGui::SetNextWindowSize(ImVec2((f32)(windowWidth - 20), 300.0f));
    ImGui::SetNextWindowPos(ImVec2(10.0f, 0.0f), ImGuiCond_Always);
    if (ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration)) {
        float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        auto logBlock = console->logger->firstBlock;
        ImGui::PushTextWrapPos();
        while (logBlock) {
            // TODO: Clipping
            // TODO: TextUnformatted puts newline at each iteration so the text will be torn at block boundaries
            //ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(logBlock->data);
            logBlock = logBlock->next;
        }
        if (console->autoScrollEnabled) {
            ImGui::SetScrollHere(1.0f);
        }
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::Separator();
        if (console->justOpened) {
            ImGui::SetKeyboardFocusHere();
            console->justOpened = false;
        }
        if (ImGui::InputText("##Input", console->inputBuffer, array_count(console->inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion, ConsoleInputCallback, console)) {
            ImGui::SetKeyboardFocusHere(-1);
            if (!console->inputBuffer[0]) {
                LogMessage(console->logger, "\n");
            } else {
                LogMessage(console->logger, "%s\n", console->inputBuffer);
                PushConsoleCommandToHistory(console);
                if (!MatchAndExecuteCommand(console)) {
                    LogMessage(console->logger, "Command not found: %s\n", console->inputBuffer);
                }
            }
            console->commandHistoryCursor = nullptr;
            console->inputBuffer[0] = 0;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::Checkbox("Auto scroll", &console->autoScrollEnabled);

        ImGui::End();
    }
}
