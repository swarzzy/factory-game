#include "Common.h"
#include "Math.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM

#include "../ext/stb/stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
static LARGE_INTEGER GlobalPerformanceFrequency = {};

LoggerFn* GlobalLogger = nullptr;
void* GlobalLoggerData = nullptr;

f64 GetTimeStamp() {
    if (GlobalPerformanceFrequency.QuadPart == 0) {
        QueryPerformanceFrequency(&GlobalPerformanceFrequency);
    }
    f64 time = 0.0;
    LARGE_INTEGER currentTime = {};
    if (QueryPerformanceCounter(&currentTime))
    {
        time = (f64)(currentTime.QuadPart) / (f64)GlobalPerformanceFrequency.QuadPart;
    }
    return time;
}
#else
f64 GetTimeStamp() { return 0; }
#endif

extern "C" GAME_CODE_ENTRY ImageInfo __cdecl ResourceLoaderValidateImageFile(const char* filename, LoggerFn* logger, void* loggerData) {
    GlobalLogger = logger;
    GlobalLoggerData = loggerData;

    ImageInfo info = {};
    auto startTime = GetTimeStamp();
    int x, y, comp;
    int r = stbi_info(filename, &x, &y, &comp);
    if (r) {
        info.valid = true;
        info.width = x;
        info.height = y;
        info.channelCount = comp;
    }
    auto endTime = GetTimeStamp();
    log_print("[Resource loader] Validating image file %s... %s. Time: %f ms\n", filename, info.valid ? "success" : "failture", (endTime - startTime) * 1000.0f);
    return info;
}

extern "C" GAME_CODE_ENTRY LoadedImage* __cdecl ResourceLoaderLoadImage(const char* filename, DynamicRange range, b32 flipY, u32 forceBPP, AllocateFn* allocator, LoggerFn* logger, void* loggerData) {
    GlobalLogger = logger;
    GlobalLoggerData = loggerData;

    auto startTime = GetTimeStamp();

    void* data = nullptr;
    int width;
    int height;
    i32 channels;
    u32 channelSize;

    if (flipY) {
        stbi_set_flip_vertically_on_load(1);
    } else {
        stbi_set_flip_vertically_on_load(0);
    }

    if (range == DynamicRange::LDR) {
        int n;
        data = stbi_load(filename, &width, &height, &n, forceBPP);
        channelSize = sizeof(u8);
        channels = forceBPP ? forceBPP : n;
    } else if (range == DynamicRange::HDR) {
        int n;
        data = stbi_loadf(filename, &width, &height, &n, forceBPP);
        channelSize = sizeof(f32);
        channels = forceBPP ? forceBPP : n;
    } else {
        unreachable();
    }

    LoadedImage* header = nullptr;

    if (data) {
        auto bitmapSize = channelSize * width * height * channels;
        auto size = sizeof(LoadedImage) + bitmapSize;
        auto memory = allocator(size, 0, nullptr);
        header = (LoadedImage*)memory;
        header->base = memory;
        header->bits = (byte*)memory + sizeof(LoadedImage);
        header->width = width;
        header->height = height;
        header->channels = channels;
        header->range = range;

        auto nameLen = strlen(filename);
        if (nameLen > array_count(header->name) - 2) {
            nameLen = array_count(header->name) - 2;
        }
        memcpy(header->name, filename, nameLen);
        header->name[nameLen + 1] = 0;

        memcpy(header->bits, data, bitmapSize);
        stbi_image_free(data);
    }

    auto endTime = GetTimeStamp();
    log_print("[Resource loader] Loaded image %s. Time: %f ms\n", filename, (endTime - startTime) * 1000.0f);

    return header;
}
