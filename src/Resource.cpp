#include "Resource.h"

#include "FileFormats.h"
#include "Globals.h"

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

Mesh* ReadMeshFileFlux(void* file, u32 fileSize) {
    auto header = (FluxMeshHeader*)file;
    uptr memorySize = header->dataSize + sizeof(Mesh) * header->entryCount;
    auto memory = PlatformAlloc(memorySize, 0, nullptr);

    auto entries = (FluxMeshEntry*)((byte*)file + header->entries);
    Mesh* loadedHeaders = (Mesh*)memory;
    void* loadedData= (byte*)memory + sizeof(Mesh) * header->entryCount;
    void* data = (byte*)file + header->data;
    assert((uptr)data % 4 == 0);
    memcpy(loadedData, data, header->dataSize);

    // TODO: Maybe stop copying whole data

    for (u32 i = 0; i < header->entryCount; i++) {
        auto loaded = loadedHeaders + i;
        auto entry = entries + i;

        loaded->base = memory;
        loaded->head = loadedHeaders;
        loaded->next = i == header->entryCount - 1 ? nullptr : loadedHeaders + i + 1;
        loaded->vertexCount = entry->vertexCount;
        loaded->indexCount = entry->indexCount;
        loaded->vertices = (v3*)((byte*)loadedData + (entry->vertices - header->data));
        loaded->normals = (v3*)((byte*)loadedData + (entry->normals - header->data));
        loaded->tangents = (v3*)((byte*)loadedData + (entry->tangents - header->data));
        loaded->bitangents = entry->bitangents ? (v3*)((byte*)loadedData + (entry->bitangents - header->data)) : nullptr;
        loaded->indices = (u32*)((byte*)loadedData + (entry->indices - header->data));
        if (entry->uv) {
            loaded->uvs = (v2*)((byte*)loadedData + (entry->uv - header->data));
        } else {
            loaded->uvs = nullptr;
        }
        if (entry->colors) {
            loaded->colors = (v3*)((byte*)loadedData + (entry->colors - header->data));
        } else {
            loaded->colors = nullptr;
        }

        loaded->aabb.min = V3(entry->aabbMin.x, entry->aabbMin.y, entry->aabbMin.z);
        loaded->aabb.max = V3(entry->aabbMax.x, entry->aabbMax.y, entry->aabbMax.z);

        loaded->gpuVertexBufferHandle = 0;
        loaded->gpuIndexBufferHandle = 0;
    }

    return (Mesh*)memory;
}

struct OpenMeshResult {
    enum Result {UnknownError = 0, Ok, FileNameIsTooLong, FileNotFound, ReadFileError, InvalidFileFormat } status;
    void* file;
    u32 fileSize;
};

OpenMeshResult OpenMeshFileFlux(const char* filename) {
    OpenMeshResult result = {};

    if (strlen(filename) < MaxAssetPathSize) {
        wchar_t filenameW[MaxAssetPathSize];
        mbstowcs(filenameW, filename, array_count(filenameW));
        auto fileSize = PlatformDebugGetFileSize(filenameW);

        if (fileSize) {
            void* file = PlatformAlloc(fileSize, 0, nullptr);
            u32 bytesRead = PlatformDebugReadFile(file, fileSize, filenameW);

            if (bytesRead == fileSize) {

                auto header = (FluxMeshHeader*)file;
                if ((header->header.magicValue == FluxFileHeader::MagicValue) &&
                    (header->header.type == FluxFileHeader::Mesh) &&
                    (header->version == 1) &&
                    (header->entryCount > 0)) {

                    result = { OpenMeshResult::Ok, file, fileSize};
                } else {
                    PlatformFree(file, nullptr);
                    result = { OpenMeshResult::InvalidFileFormat, nullptr, 0};
                }
            } else {
                PlatformFree(file, nullptr);
                result = { OpenMeshResult::ReadFileError, nullptr, 0 };
            }
        } else {
            result = { OpenMeshResult::FileNotFound, nullptr, 0 };
        }
    } else {
        result = { OpenMeshResult::FileNameIsTooLong, nullptr, 0 };
    }
    return result;
}

void CloseMeshFile(void* file) {
    PlatformFree(file, nullptr);
}

Mesh* LoadMeshFlux(const char* filename) {
    Mesh* result = nullptr;
    auto status = OpenMeshFileFlux(filename);
    if (status.status == OpenMeshResult::Ok) {
        result = ReadMeshFileFlux(status.file, status.fileSize);
        CloseMeshFile(status.file);
    }
    return result;
}
