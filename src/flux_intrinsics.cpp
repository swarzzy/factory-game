#include "flux_intrinsics.h"

#if defined(PLATFORM_WINDOWS)

u32 AtomicCompareExhange(u32 volatile* dest, u32 comp, u32 newValue) {
    return (u32)InterlockedCompareExchange((LONG volatile*)dest, (LONG)newValue, (LONG)comp);
}

u32 AtomicExchange(u32 volatile* dest, u32 value) {
    return (u32)InterlockedExchange((LONG volatile*)dest, (LONG)value);
}

u32 AtomicIncrement(u32 volatile* dest) {
    return (u32)InterlockedIncrement((LONG volatile*)dest);
}

#endif
