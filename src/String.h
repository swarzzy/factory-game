#pragma once

bool MatchStrings(const char* a, const char* b) {
    bool result = true;
    while(*a) {
        if (!(*b)) {
            result = false;
            break;
        }
        if (*a != *b) {
            result = false;
            break;
        }
        a++;
        b++;
    }
    return result;
}

bool StringsAreEqual(const char* a, const char* b) {
    bool result = true;
    while(*a) {
        if (!(*b)) {
            result = false;
            break;
        }
        if (*a != *b) {
            result = false;
            break;
        }
        a++;
        b++;
    }
    if (*b) {
        result = false;
    }
    return result;
}

bool IsSpace(char c) {
    bool result = false;
    if (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v') {
        result = true;
    }
    return result;
}

struct FloatParseResult {
    b32 succeed;
    f32 value;
};

FloatParseResult StringToFloat(const char* string) {
    FloatParseResult result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    f32 value = strtof(string, &end);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != HUGE_VAL) && (value != HUGE_VALF) && (value != HUGE_VALL) && (errno == 0)) {
        result.succeed = true;
        result.value = value;
    }
    return result;
}

struct IntParseResult {
    b32 succeed;
    i32 value;
};

IntParseResult StringToInt(const char* string) {
    IntParseResult result  = {};
    char* end;
    // Is that crazy global variable error checking even thread safe?
    errno = 0;
    i32 value = (i32)strtol(string, &end, 10);
    // TODO: At some point I should stop using this crazy CRT crappiness and write my own routine
    if ((end != string) && (value != LONG_MAX) && (value != LONG_MIN) && (value != HUGE_VALL) && (errno == 0)) {
        result.succeed = true;
        result.value = value;
    }
    return result;
}

bool PrettySize(char* buffer, u32 bufferSize, uptr bytes) {
    bool result = false;
    f64 size = (f64)bytes;
    const char* suffixes[4] = {"bytes", "kb", "mb", "gb"};
    i32 s = 0;
    while (size >= 1024 && s < 4)
    {
        s++;
        size /= 1024;
    }
    if (size - Floor(size) == 0.0) {
        u32 requiredSize = (u32)snprintf(nullptr, 0, "%ld %s", (long)size, suffixes[s]) + 1;
        if (requiredSize <= bufferSize) {
            sprintf(buffer, "%ld %s", (long)size, suffixes[s]);
            result = true;
        }
    }
    else {
        u32 requiredSize = (u32)snprintf(nullptr, 0, "%.1f %s", size, suffixes[s]) + 1;
        if (requiredSize <= bufferSize) {
            sprintf(buffer, "%.1f %s", size, suffixes[s]);
            result = true;
        }
    }
    return result;
}
