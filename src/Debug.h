#pragma once

#include "Common.h"

#define timed_scope() DebugScopedTimer concat(scopeTimer_, __LINE__)(__COUNTER__, __FILE__, __FUNCTION__, __LINE__);

struct DebugCallRecord {
    const char* file;
    const char* func;
    u32 line;
    u32 cycleCount;
    u32 invokeCount;
};

extern const u32 GlobalDebugCallRecordsCount;
extern DebugCallRecord GlobalDebugCallRecords[];

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
