#pragma once

#include "Common.h"

enum struct ChunkToolState {
    Closed = 0, Main, ChunkInfo
};

struct DebugUI {
    ChunkToolState chunkToolState;
    bool perfCountersState;
    int chunkLayer;
    iv3 selectedChunk;
    bool shouldReleaseControl;
};

void DebugUIToggleChunkTool(DebugUI* ui) {
    if (ui->chunkToolState != ChunkToolState::Closed) {
        ui->chunkToolState = ChunkToolState::Closed;
    } else {
        ui->chunkToolState = ChunkToolState::Main;
    }
}

void DebugUITogglePerfCounters(DebugUI* ui) {
    ui->perfCountersState = !ui->perfCountersState;
}


void DebugUIUpdateAndRender(DebugUI* ui);
