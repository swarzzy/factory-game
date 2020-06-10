#pragma once

#include "Common.h"

enum struct ChunkToolState {
    Closed = 0, Main, ChunkInfo
};

struct DebugUI {
    ChunkToolState chunkToolState;
    int chunkLayer;
    iv3 selectedChunk;
};

void DebugUIToggleChunkTool(DebugUI* ui) {
    if (ui->chunkToolState != ChunkToolState::Closed) {
        ui->chunkToolState = ChunkToolState::Closed;
    } else {
        ui->chunkToolState = ChunkToolState::Main;
    }
}

void DebugUIUpdateAndRender(DebugUI* ui);
