#include "Intrinsics.h"

#if defined(PLATFORM_WINDOWS)

u32 AtomicCompareExchange(u32 volatile* dest, u32 comp, u32 newValue) {
    return (u32)_InterlockedCompareExchange((LONG volatile*)dest, (LONG)newValue, (LONG)comp);
}

u32 AtomicExchange(u32 volatile* dest, u32 value) {
    return (u32)_InterlockedExchange((LONG volatile*)dest, (LONG)value);
}

u64 AtomicExchange(u64 volatile* dest, u64 value) {
    return (u64)_InterlockedExchange64((__int64 volatile*)dest, (__int64)value);
}

u32 AtomicIncrement(u32 volatile* dest) {
    return (u32)_InterlockedIncrement((LONG volatile*)dest);
}

u32 AtomicDecrement(u32 volatile* dest) {
    return (u32)_InterlockedDecrement((LONG volatile*)dest);
}

u32 AtomicLoad(u32 volatile* value) {
    return *value;
}

u64 GetTimeStamp() {
    LARGE_INTEGER count;
    auto result = QueryPerformanceCounter(&count);
    return count.QuadPart;
}

static u64 _TicksPerSecond = 0;

u64 GetTicksPerSecond() {
    if (_TicksPerSecond == 0) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        _TicksPerSecond = frequency.QuadPart;
        assert(_TicksPerSecond);
    }
    return _TicksPerSecond;
}



#endif
