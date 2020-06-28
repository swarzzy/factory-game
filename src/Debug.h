#pragma once

#include "Common.h"

#define timed_scope() DebugScopedTimer concat(scopeTimer_, __LINE__)(__COUNTER__, __FILE__, __FUNCTION__, __LINE__);

const u32 DebugBufferSize = 128;

struct DebugCallRecord {
    const char* file;
    const char* func;
    u32 line;
    u32 cycleCount;
    u32 invokeCount;
};

extern const u32 GlobalDebugCallRecordsCount;
extern DebugCallRecord GlobalDebugCallRecords[];

struct ProfilerBufferRecord {
    constant u32 Size = 128;
    const char* file;
    const char* func;
    u32 line;
    u32 cycleCount[Size];
    u32 invokeCount[Size];
    u32 cyclesAvg;
    u32 invokeCountAvg;
    u32 cyclesPerCallAvg;
};

struct ProfilerBuffer {
    u32 at;
    ProfilerBufferRecord records[512];
};

void UpdateDebugProfiler(ProfilerBuffer* buffer) {
    for (u32 i = 0 ; i < GlobalDebugCallRecordsCount; i++) {
        auto capture = GlobalDebugCallRecords + i;
        auto record = buffer->records + i;
        record->file = capture->file;
        record->func = capture->func;
        record->line = capture->line;
        record->cycleCount[buffer->at] = capture->cycleCount;
        record->invokeCount[buffer->at] = capture->invokeCount;
        record->cyclesAvg = 0;
        record->invokeCountAvg = 0;
        record->cyclesPerCallAvg = 0;
        for (u32 j = 0; j < ProfilerBufferRecord::Size; j++) {
            record->cyclesAvg += record->cycleCount[j];
            record->invokeCountAvg += record->invokeCount[j];
        }
        record->cyclesAvg /= ProfilerBufferRecord::Size;
        record->invokeCountAvg /= ProfilerBufferRecord::Size;
        if (record->invokeCountAvg != 0) {
            record->cyclesPerCallAvg = record->cyclesAvg / record->invokeCountAvg;
        } else {
            record->cyclesPerCallAvg = 0;
        }

        capture->cycleCount = 0;
        capture->invokeCount = 0;
    }
    buffer->at = (buffer->at + 1) % ProfilerBufferRecord::Size;
}

struct DebugScopedTimer {
    u32 beginCount;
    u32 recordIndex;
    inline DebugScopedTimer(u32 index, const char* file, const char* func, u32 line) {
        assert(index < GlobalDebugCallRecordsCount);
        auto record = GlobalDebugCallRecords + index;
        record->file = file;
        record->func = func;
        record->line = line;
        beginCount = (u32)GetCycleCount();
        recordIndex = index;
    }

    inline ~DebugScopedTimer() {
        auto record = GlobalDebugCallRecords + recordIndex;
        record->cycleCount += (u32)GetCycleCount() - beginCount;
        record->invokeCount++;
    }
};
