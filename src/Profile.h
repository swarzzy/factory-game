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

enum struct ProfilerSort : u32  {
    ByTimeInc, ByTimeDec
};

struct ProfilerBuffer {
    ProfilerSort sortMode;
    u32 at;
    ProfilerBufferRecord records[512];
};

void UpdateDebugProfiler(ProfilerBuffer* buffer);
void DrawDebugProfiler(ProfilerBuffer* buffer);

struct DebugScopedTimer {
    u64 beginCount;
    u32 recordIndex;
    inline DebugScopedTimer(u32 index, const char* file, const char* func, u32 line) {
        assert(index < GlobalDebugCallRecordsCount);
        auto record = GlobalDebugCallRecords + index;
        record->file = file;
        record->func = func;
        record->line = line;
        beginCount = GetTimeStamp();
        recordIndex = index;
    }

    inline ~DebugScopedTimer() {
        auto record = GlobalDebugCallRecords + recordIndex;
        record->cycleCount += (u32)(GetTimeStamp() - beginCount);
        record->invokeCount++;
    }
};
