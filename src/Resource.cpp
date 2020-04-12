#include "Resource.h"

int STBDesiredBPPFromTextureFormat(TextureFormat format) {
    int desiredBpp = 0;
    switch (format) {
    case TextureFormat::SRGBA8:
    case TextureFormat::RGBA8: { desiredBpp = 4; } break;
    case TextureFormat::SRGB8:
    case TextureFormat::RGB8:
    case TextureFormat::RGB16F: { desiredBpp = 3; } break;
    case TextureFormat::RG16F: { desiredBpp = 2; } break;
    case TextureFormat::R8: { desiredBpp = 1; } break;
    case TextureFormat::RG8: { desiredBpp = 2; } break;
        invalid_default();
    }
    return desiredBpp;
}

TextureFormat GuessTexFormatFromNumChannels(u32 num) {
    TextureFormat format;
    switch (num) {
    case 1: { format = TextureFormat::R8; } break;
    case 2: { format = TextureFormat::RG8; } break; // TODO: Implement in renderer
    case 3: { format = TextureFormat::RGB8; } break;
    case 4: { format = TextureFormat::SRGBA8; } break;
        invalid_default();
    }
    return format;
}

Texture LoadTextureFromFile(const char* filename, TextureFormat format, TextureWrapMode wrapMode, TextureFilter filter, DynamicRange range) {
    Texture t = {};

    i32 desiredBpp = 0;

    if (format != TextureFormat::Unknown) {
        desiredBpp = STBDesiredBPPFromTextureFormat(format);
    }

    auto image = ResourceLoaderLoadImage(filename, range, true, desiredBpp, PlatformAlloc);
    assert(image);

    if (format == TextureFormat::Unknown) {
        format = GuessTexFormatFromNumChannels(image->channels);
    }

    // TODO: Store texture struct in memory chunk
    t.base = image;
    t.format = format;
    t.width = image->width;
    t.height = image->height;
    t.wrapMode = wrapMode;
    t.filter = filter;
    t.data = image->bits;

    return t;
}

Texture CreateTexture(i32 width, i32 height, TextureFormat format, TextureWrapMode wrapMode, TextureFilter filter, void* data) {
    Texture t = {};

    t.format = format;
    t.width = width;
    t.height = height;
    t.filter = filter;
    t.wrapMode = wrapMode;
    t.data = data;

    return t;
}

CubeTexture LoadCubemap(const char* backPath, const char* downPath, const char* frontPath,
                        const char* leftPath, const char* rightPath, const char* upPath,
                        DynamicRange range, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode) {
    CubeTexture texture = {};

    // TODO: Use memory arena
    // TODO: Free memory
    auto back = ResourceLoaderLoadImage(backPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(back->base); };
    auto down = ResourceLoaderLoadImage(downPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(down->base); };
    auto front = ResourceLoaderLoadImage(frontPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(front->base); };
    auto left = ResourceLoaderLoadImage(leftPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(left->base); };
    auto right = ResourceLoaderLoadImage(rightPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(right->base); };
    auto up = ResourceLoaderLoadImage(upPath, range, false, 0, PlatformAlloc);
    //defer { PlatformFree(up->base); };

    assert(back->width == down->width);
    assert(back->width == front->width);
    assert(back->width == left->width);
    assert(back->width == right->width);
    assert(back->width == up->width);

    assert(back->height == down->height);
    assert(back->height == front->height);
    assert(back->height == left->height);
    assert(back->height == right->height);
    assert(back->height == up->height);

    texture.format = format;
    texture.width = back->width;
    texture.height = back->height;
    texture.backData = back->bits;
    texture.downData = down->bits;
    texture.frontData = front->bits;
    texture.leftData = left->bits;
    texture.rightData = right->bits;
    texture.upData = up->bits;

    return texture;
}

CubeTexture MakeEmptyCubemap(u32 w, u32 h, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode, bool useMips) {
    CubeTexture texture = {};
    texture.useMips = useMips;
    texture.filter = filter;
    texture.format = format;
    texture.wrapMode = wrapMode;
    texture.width = w;
    texture.height = h;
    return texture;
}
