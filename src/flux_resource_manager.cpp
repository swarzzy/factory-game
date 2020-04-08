#include "flux_resource_manager.h"
#include "flux_file_formats.h"

u32 PixelSize(TextureFormat format) {
    u32 size = 0;
    switch (format) {
    case TextureFormat::Unknown: { size = 0; } break;
    case TextureFormat::SRGBA8: { size = 4; } break;
    case TextureFormat::SRGB8: { size = 3; } break;
    case TextureFormat::RGBA8: { size = 4; } break;
    case TextureFormat::RGB8: { size = 3; } break;
    case TextureFormat::RGB16F: { size = 6; } break;
    case TextureFormat::RG16F: { size = 4; } break;
    case TextureFormat::RG32F: { size = 8; } break;
    case TextureFormat::R8: { size = 1; } break;
    case TextureFormat::RG8: { size = 2; } break;
    invalid_default();
    }
    return size;
}

u32 NumberOfChannels(TextureFormat format) {
    u32 size = 0;
    switch (format) {
    case TextureFormat::Unknown: { size = 0; } break;
    case TextureFormat::SRGBA8: { size = 4; } break;
    case TextureFormat::SRGB8: { size = 3; } break;
    case TextureFormat::RGBA8: { size = 4; } break;
    case TextureFormat::RGB8: { size = 3; } break;
    case TextureFormat::RGB16F: { size = 3; } break;
    case TextureFormat::RG16F: { size = 2; } break;
    case TextureFormat::RG32F: { size = 2; } break;
    case TextureFormat::R8: { size = 1; } break;
    case TextureFormat::RG8: { size = 2; } break;
    invalid_default();
    }
    return size;
}

TextureFormat GuessTextureFormat(u32 numChannels, DynamicRange range) {
    TextureFormat format = TextureFormat::Unknown;
    if (range == DynamicRange::HDR) {
        switch(numChannels) {
        case 2: { format = TextureFormat::RG16F; } break;
        case 3: { format = TextureFormat::RGB16F; } break;
        default: {} break;
        }
    } else if (range == DynamicRange::LDR) {
        switch(numChannels) {
        case 1: { format = TextureFormat::R8; } break;
        case 2: { format = TextureFormat::RG8; } break;
        case 3: { format = TextureFormat::SRGB8; } break;
        case 4: { format = TextureFormat::SRGBA8; } break;
        default: {} break;
        }
    } else {
        unreachable();
    }
    return format;
}

u32 AddName(AssetNameTable* table, const char* _name) {
    u32 result = 0;
    AssetName name;
    strcpy_s(name.name, array_count(name.name), _name);
    u32* guid = Add(&table->table, &name);
    if (guid) {
        *guid = table->serialCount++;
        result = *guid;
    }
    return result;
}

void RemoveName(AssetNameTable* table, const char* _name) {
    // TODO: Stop copying string!!!
    AssetName name;
    strcpy_s(name.name, array_count(name.name), _name);
    auto result = Delete(&table->table, &name);
    assert(result);
}

u32 GetID(AssetNameTable* table, const char* _name) {
    u32 result = 0;
    // TODO: Stop copying string!!!
    AssetName name;
    strcpy_s(name.name, array_count(name.name), _name);
    u32* id = Get(&table->table, &name);
    if (id) {
        result = *id;
    }
    return result;
}

Mesh* ReadMeshFileFlux(void* file, u32 fileSize) {
    auto header = (FluxMeshHeader*)file;
    uptr memorySize = header->dataSize + sizeof(Mesh) * header->entryCount;
    auto memory = PlatformAlloc(memorySize);

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


const char* ToString(OpenMeshResult::Result value) {
    static const char* strings[] = {
        "Unknown error",
        "Ok",
        "File name is too long",
        "File not found",
        "Read file error",
        "Invalid file format",
    };
    assert((u32)value < array_count(strings));
    return strings[(u32)value];
}

OpenMeshResult OpenMeshFileFlux(const char* filename) {
    OpenMeshResult result = {};

    if (strlen(filename) < MaxAssetPathSize) {
        wchar_t filenameW[MaxAssetPathSize];
        mbstowcs(filenameW, filename, array_count(filenameW));
        auto fileSize = PlatformDebugGetFileSize(filenameW);

        if (fileSize) {
            void* file = PlatformAlloc(fileSize);
            u32 bytesRead = PlatformDebugReadFile(file, fileSize, filenameW);

            if (bytesRead == fileSize) {

                auto header = (FluxMeshHeader*)file;
                if ((header->header.magicValue == FluxFileHeader::MagicValue) &&
                    (header->header.type == FluxFileHeader::Mesh) &&
                    (header->version == 1) &&
                    (header->entryCount > 0)) {

                    result = { OpenMeshResult::Ok, file, fileSize};
                } else {
                    PlatformFree(file);
                    result = { OpenMeshResult::InvalidFileFormat, nullptr, 0};
                }
            } else {
                PlatformFree(file);
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
    PlatformFree(file);
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

const u32 AAB_FILE_MAGIC_VALUE = 0xaabaabaa;
#pragma pack(push, 1)
struct AABMeshHeaderV2 {
    u32 magicValue;
    u32 version = 2;
    u64 assetSize;
    u32 assetType;

    u32 vertexCount;
    u32 indexCount;
    u64 vertexOffset;
    u64 normalsOffset;
    u64 uvOffset;
    u64 indicesOffset;
    u64 tangentsOffset;
};
#pragma pack(pop)

Mesh* ReadMeshFileAAB(void* memory, void* fileData, u32 fileSize) {
    auto mesh = (Mesh*)memory;
    *mesh = {};
    mesh->base = memory;

    auto header = (AABMeshHeaderV2*)fileData;
    assert(header->magicValue == AAB_FILE_MAGIC_VALUE);

    mesh->vertexCount = header->vertexCount;
    mesh->indexCount = header->indexCount;

    mesh->vertices = (v3*)((byte*)fileData + header->vertexOffset);
    mesh->normals = (v3*)((byte*)fileData + header->normalsOffset);
    mesh->uvs = (v2*)((byte*)fileData + header->uvOffset);
    mesh->indices = (u32*)((byte*)fileData + header->indicesOffset);
    mesh->tangents = (v3*)((byte*)fileData + header->tangentsOffset);

    mesh->aabb = BBoxAligned::From(mesh);
    return mesh;
}


OpenMeshResult OpenMeshFileAAB(const char* filename) {
    OpenMeshResult result = {};

    if (strlen(filename) < MaxAssetPathSize) {
        wchar_t filenameW[MaxAssetPathSize];
        mbstowcs(filenameW, filename, array_count(filenameW));

        auto fileSize = PlatformDebugGetFileSize(filenameW);

        if (fileSize) {

            void* fileData = PlatformAlloc(fileSize);
            u32 bytesRead = PlatformDebugReadFile(fileData, fileSize, filenameW);

            if (bytesRead == fileSize) {
                auto header = (AABMeshHeaderV2*)fileData;
                if (header->magicValue == AAB_FILE_MAGIC_VALUE) {
                    result = { OpenMeshResult::Ok, fileData, fileSize};
                } else {
                    PlatformFree(fileData);
                    result = { OpenMeshResult::InvalidFileFormat, nullptr, 0};
                }
            } else {
                PlatformFree(fileData);
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


Mesh* LoadMeshAAB(const char* filename) {
    Mesh* mesh = nullptr;
    // TODO: Error checking
    if (strlen(filename) < MaxAssetPathSize) {
        wchar_t filenameW[512];
        mbstowcs(filenameW, filename, array_count(filenameW));

        // Make shure loaded data will be propperly aligned
        auto headerSize = sizeof(Mesh) + CalculatePadding(sizeof(Mesh), 16);
        auto dataOffset = headerSize;

        u32 fileSize = PlatformDebugGetFileSize(filenameW);
        if (fileSize) {
            auto totalSize = fileSize + headerSize;
            void* memory = PlatformAlloc(totalSize);
            mesh = (Mesh*)memory;
            *mesh = {};
            mesh->base = memory;
            auto fileData = (void*)((byte*)memory + dataOffset);
            u32 result = PlatformDebugReadFile(fileData, fileSize, filenameW);
            if (result) {
                auto header = (AABMeshHeaderV2*)fileData;
                if (header->magicValue == AAB_FILE_MAGIC_VALUE) {
                    mesh->vertexCount = header->vertexCount;
                    mesh->indexCount = header->indexCount;

                    mesh->vertices = (v3*)((byte*)fileData + header->vertexOffset);
                    mesh->normals = (v3*)((byte*)fileData + header->normalsOffset);
                    mesh->uvs = (v2*)((byte*)fileData + header->uvOffset);
                    mesh->indices = (u32*)((byte*)fileData + header->indicesOffset);
                    mesh->tangents = (v3*)((byte*)fileData + header->tangentsOffset);
                    mesh->bitangents = nullptr;

                    mesh->aabb = BBoxAligned::From(mesh);
                } else {
                    PlatformFree(memory);
                }
            } else {
                PlatformFree(memory);
            }
        }
    }
    return mesh;
}

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

AssetQueueEntry* AssetQueuePush(AssetManager* manager) {
    AssetQueueEntry* ptr = nullptr;
    if (manager->assetQueueUsage < array_count(manager->assetQueue)) {
        for (u32x i = 0; i < array_count(manager->assetQueue); i++) {
            auto entry = manager->assetQueue + i;
            if (!entry->used) {
                ptr = entry;
                ptr->used = true;
                break;
            }
        }
        assert(ptr);
        manager->assetQueueUsage++;
    }
    return ptr;
}

void AssetQueueRemove(AssetManager* manager, u32 index) {
    assert(index < array_count(manager->assetQueue));
    auto entry = manager->assetQueue + index;
    assert(manager->assetQueueUsage > 0);
    manager->assetQueueUsage--;
    entry->used = false;
}

AssetQueueEntry* AssetQueueGet(AssetManager* manager, u32 index) {
    assert(index < array_count(manager->assetQueue));
    AssetQueueEntry* result;
    result = manager->assetQueue + index;
    return result;
}

void LoadMeshWork(void* data, u32 threadIndex) {
    auto queueEntry = (AssetQueueEntry*)data;
    assert(queueEntry->used);
    assert(queueEntry->type == AssetType::Mesh);
    auto slot = (MeshSlot*)(&queueEntry->meshSlot);
    printf("[Asset manager] Thread %d: Loading mesh %s\n", (int)threadIndex, slot->filename);
    Mesh* mesh = nullptr;
    switch (slot->format)
    {
    case MeshFileFormat::AAB: {
        mesh = LoadMeshAAB(slot->filename);
    } break;
    case MeshFileFormat::Flux: {
        mesh = LoadMeshFlux(slot->filename);
    } break;
        invalid_default();
    }
    if (mesh) {
        slot->mesh = mesh;
        auto prevState = AtomicExchange((u32 volatile*)&slot->state, (u32)AssetState::JustLoaded);
        assert(prevState == (u32)AssetState::Queued);
    } else {
        printf("[Asset manager] Failed to load mesh: %s\n", slot->filename);
        auto prevState = AtomicExchange((u32 volatile*)&slot->state, (u32)AssetState::Error);
        assert(prevState == (u32)AssetState::Queued);
    }
}

void LoadTextureWork(void* data, u32 threadIndex) {
    auto queueEntry = (AssetQueueEntry*)data;
    assert(queueEntry->used);
    assert(queueEntry->type == AssetType::Texture);
    auto slot = (TextureSlot*)(&queueEntry->textureSlot);
    printf("[Asset manager] Thread %d: Loading texture %s\n", (int)threadIndex, slot->filename);
    Texture tex = LoadTextureFromFile(slot->filename, slot->format, slot->wrapMode, slot->filter, slot->range);
    if (tex.data) {
        auto bitmapSize = tex.width * tex.height * PixelSize(slot->format);
        if (bitmapSize == slot->bitmapSize) {
            slot->texture = tex;
            // TODO: Load directly to buffer
            for (uptr i = 0; i < bitmapSize; i++) {
                ((byte*)queueEntry->texTransferBufferInfo.ptr)[i] = ((byte*)tex.data)[i];
            }
            //memcpy(queueEntry->texTransferBufferInfo.ptr, tex.data, bitmapSize);
            auto prevState = AtomicExchange((u32 volatile*)&slot->state, (u32)AssetState::JustLoaded);
            assert(prevState == (u32)AssetState::Queued);
        } else {
            printf("[Asset manager] Failed to load texture: %s. The file is different than one that was added before\n", slot->filename);
            auto prevState = AtomicExchange((u32 volatile*)&slot->state, (u32)AssetState::Error);
            assert(prevState == (u32)AssetState::Queued);
            PlatformFree(tex.data);
        }
    } else {
        printf("[Asset manager] Failed to load texture: %s\n", slot->filename);
        auto prevState = AtomicExchange((u32 volatile*)&slot->state, (u32)AssetState::Error);
        assert(prevState == (u32)AssetState::Queued);
    }
}

void GetAssetName(const char* filename, AssetName* name) {
    // TODO: This is slow for now
    u32 len = (u32)strlen(filename);
    u32 extBeginIndex = len;
    u32 dirBeginIndex = len;
    for (i32 i = len; i >= 0; i--) {
        char at = filename[i];
        if (at == '.') {
            extBeginIndex = i;
            break;
        }
    }
    for (i32 i = len; i >= 0; i--) {
        char at = filename[i];
        if (at == '/' || at == '\\') {
            dirBeginIndex = i;
            break;
        }
    }
    u32 begin = 0;
    if (dirBeginIndex < extBeginIndex) {
        begin = dirBeginIndex + 1;
    }
    u32 end = len;
    if (extBeginIndex > dirBeginIndex) {
        end = extBeginIndex;
    }

    u32 count = end - begin;

    strncpy_s(name->name, array_count(name->name), filename + begin, count);
}

AddAssetResult AddMesh(AssetManager* manager, const char* filename, MeshFileFormat format) {
    AddAssetResult result = {};
    OpenMeshResult fileStatus = {};
    switch (format) {
    case MeshFileFormat::AAB: { fileStatus = OpenMeshFileAAB(filename); } break;
    case MeshFileFormat::Flux: { fileStatus = OpenMeshFileFlux(filename); } break;    invalid_default();
    }

    if (fileStatus.status == OpenMeshResult::Ok) {
        CloseMeshFile(fileStatus.file);
        AssetName name;
        GetAssetName(filename, &name);
        bool alreadyExists = GetID(&manager->nameTable, name.name) != 0;
        if (!alreadyExists) {
            u32 id = AddName(&manager->nameTable, name.name);
            assert(id);
            auto slot = Add(&manager->meshTable, &id);
            assert(slot);
            *slot = {};
            slot->id = id;
            strcpy_s(slot->name, array_count(slot->name), name.name);
            strcpy_s(slot->filename, array_count(slot->filename), filename);
            slot->format = format;
            result = { AddAssetResult::Ok, id };
        } else {
            printf("[Asset manager] Failed to load asset %s. An asset with the same name is already loaded.\n", filename);
            result = { AddAssetResult::AlreadyExists, 0 };
        }
    } else {
        printf("[Asset manager] Failed to open asset file: %s. Error: %s\n", filename, ToString(fileStatus.status));
    }
    return result;
}

AddAssetResult AddTexture(AssetManager* manager, const char* filename, TextureFormat format, TextureWrapMode wrapMode, TextureFilter filter, DynamicRange range) {
    // TODO: Validate file
    AddAssetResult result = {};
    auto info = ResourceLoaderValidateImageFile(filename);
    if (info.valid) {
        if (format == TextureFormat::Unknown) {
            format = GuessTextureFormat(info.channelCount, range);
        }
        if (format != TextureFormat::Unknown) {
            auto formatNumChannels = NumberOfChannels(format);
            if (formatNumChannels <= info.channelCount) {
                AssetName name;
                GetAssetName(filename, &name);
                bool alreadyExists = GetID(&manager->nameTable, name.name) != 0;
                if (!alreadyExists) {
                    u32 id = AddName(&manager->nameTable, name.name);
                    assert(id);
                    auto slot = Add(&manager->textureTable, &id);
                    assert(slot);
                    *slot = {};
                    slot->id = id;
                    slot->format = format;
                    slot->wrapMode = wrapMode;
                    slot->filter = filter;
                    slot->range = range;
                    slot->bitmapSize = info.width * info.height * PixelSize(slot->format);
                    strcpy_s(slot->name, array_count(slot->name), name.name);
                    strcpy_s(slot->filename, array_count(slot->filename), filename);
                    result = { AddAssetResult::Ok, id };
                } else {
                    printf("[Asset manager] Failed to load tuexture %s. An asset with the same name is already loaded.\n", filename);
                    result = { AddAssetResult::AlreadyExists, 0 };
                }
            } else {
                printf("[Asset manager] Failed to load texture %s. Texture format has %ld channels, but image has only %ld channels\n", filename, (long)formatNumChannels, (long)info.channelCount);
                result = { AddAssetResult::UnknownFormat, 0 };
            }
        } else {
            printf("[Asset manager] Failed to load texture %s. Unknown texture format\n", filename);
            result = { AddAssetResult::UnknownFormat, 0 };
        }
    } else {
        printf("[Asset manager] Failed to load asset %s. Invalid image file.\n", filename);
    }
    return result;
}

void LoadMesh(AssetManager* manager, u32 id) {
    auto slot = Get(&manager->meshTable, &id);
    if (slot) {
        auto queueEntry = AssetQueuePush(manager);
        if (queueEntry) {
            queueEntry->id = id;
            queueEntry->type = AssetType::Mesh;
            auto slotPtr = (MeshSlot*)queueEntry->meshSlot;
            *slotPtr = *slot;

            // TODO: Spin-locking here for now. We should handle the case when platform queue is full propperly
            while (!PlatformPushWork(GlobalPlaformWorkQueue, queueEntry, LoadMeshWork)) {}
        }
    }
}

void UnloadMesh(AssetManager* manager, MeshSlot* slot) {
    if (slot->state == AssetState::Loaded) {
        FreeGPUBuffer(slot->mesh->gpuVertexBufferHandle);
        FreeGPUBuffer(slot->mesh->gpuIndexBufferHandle);
        PlatformFree(slot->mesh->base);
        slot->mesh = nullptr;
        slot->state = AssetState::Unloaded;
    }
}

void UnloadMesh(AssetManager* manager, u32 id) {
    auto slot = GetMeshSlot(manager, id);
    if (slot) {
        UnloadMesh(manager, slot);
    }
}

void RemoveMesh(AssetManager* manager, u32 id) {
    auto slot = GetMeshSlot(manager, id);
    if (slot && (slot->state == AssetState::Loaded || slot->state == AssetState::Unloaded)) {
        UnloadMesh(manager, slot);
        RemoveName(&manager->nameTable, slot->name);
        Delete(&manager->meshTable, &id);
    }
}

void UnloadTexture(AssetManager* manager, TextureSlot* slot) {
    if (slot->state == AssetState::Loaded) {
        FreeGPUTexture(slot->texture.gpuHandle);
        PlatformFree(slot->texture.base);
        slot->texture = {};
        slot->state = AssetState::Unloaded;
    }
}

void UnloadTexture(AssetManager* manager, u32 id) {
    auto slot = GetTextureSlot(manager, id);
    if (slot) {
        UnloadTexture(manager, slot);
    }
}

void RemoveTexture(AssetManager* manager, u32 id) {
    auto slot = GetTextureSlot(manager, id);
    if (slot && (slot->state == AssetState::Loaded || slot->state == AssetState::Unloaded)) {
        UnloadTexture(manager, slot);
        RemoveName(&manager->nameTable, slot->name);
        Delete(&manager->textureTable, &id);
    }
}

void LoadTexture(AssetManager* manager, u32 id) {
    auto slot = Get(&manager->textureTable, &id);
    if (slot) {
        auto transferBuffer = GetTextureTransferBuffer(manager->renderer, slot->bitmapSize);
        if (transferBuffer.ptr) {
            auto queueEntry = AssetQueuePush(manager);
            if (queueEntry) {
                queueEntry->id = id;
                queueEntry->type = AssetType::Texture;
                queueEntry->texTransferBufferInfo = transferBuffer;
                slot->state = AssetState::Queued;
                auto slotPtr = (TextureSlot*)queueEntry->textureSlot;
                *slotPtr = *slot;

                // TODO: Spin-locking here for now. We should handle the case when platform queue is full propperly
                while (!PlatformPushWork(GlobalPlaformWorkQueue, queueEntry, LoadTextureWork)) {}
            }
        } else {
            printf("[Asset manager] Failed to load the texture %s. Unable to get transfer buffer\n", slot->name);
        }
    }
}

void CompleteAssetLoad(AssetManager* manager, AssetType type, u32 queueIndex, u32 id) {
    switch (type) {
    case AssetType::Mesh: {
        auto queueEntry = AssetQueueGet(manager, queueIndex);
        auto queueSlot = (MeshSlot*)queueEntry->meshSlot;
        if (queueSlot->state == AssetState::JustLoaded) {
            auto slot = Get(&manager->meshTable, &id);
            assert(slot);
            assert(slot->state == AssetState::Queued);
            assert(slot->id == queueEntry->id);
            *slot = *queueSlot;
            AssetQueueRemove(manager, queueIndex);
            auto begin = PlatformGetTimeStamp();
            UploadToGPU(slot->mesh);
            auto end = PlatformGetTimeStamp();
            printf("[Asset manager] Loaded mesh on gpu: %f ms\n", (end - begin) * 1000.0f);
            slot->state = AssetState::Loaded;
        } else if (queueSlot->state == AssetState::Error) {
            auto slot = Get(&manager->meshTable, &id);
            assert(slot);
            assert(slot->state == AssetState::Queued);
            assert(slot->id == queueEntry->id);
            AssetQueueRemove(manager, queueIndex);
            slot->state = AssetState::Error;
        }
    } break;
    case AssetType::Texture: {
        auto queueEntry = AssetQueueGet(manager, queueIndex);
        auto queueSlot = (TextureSlot*)queueEntry->meshSlot;
        if (queueSlot->state == AssetState::JustLoaded) {
            auto slot = Get(&manager->textureTable, &id);
            assert(slot);
            assert(slot->state == AssetState::Queued);
            assert(slot->id == queueEntry->id);
            *slot = *queueSlot;
            AssetQueueRemove(manager, queueIndex);
            auto begin = PlatformGetTimeStamp();
            CompleteTextureTransfer(&queueEntry->texTransferBufferInfo, &slot->texture);
            auto end = PlatformGetTimeStamp();
            printf("[Asset manager] Loaded material on gpu: %f ms\n", (end - begin) * 1000.0f);
            slot->state = AssetState::Loaded;
        } else if (queueSlot->state == AssetState::Error) {
            auto slot = Get(&manager->textureTable, &id);
            assert(slot);
            assert(slot->state == AssetState::Queued);
            assert(slot->id == queueEntry->id);
            AssetQueueRemove(manager, queueIndex);
            slot->state = AssetState::Error;
        }
    } break;
        invalid_default();
    }
}

void CompletePendingLoads(AssetManager* manager) {
    for (u32 i = 0; i < array_count(manager->assetQueue); i++) {
        auto entry = manager->assetQueue + i;
        if (entry->used) {
            CompleteAssetLoad(manager, entry->type, i, entry->id);
        }
    }
}

MeshSlot* GetMeshSlot(AssetManager* manager, u32 id) {
    return Get(&manager->meshTable, &id);
}

TextureSlot* GetTextureSlot(AssetManager* manager, u32 id) {
    return Get(&manager->textureTable, &id);
}

Mesh* GetMesh(AssetManager* manager, u32 id) {
    Mesh* result = nullptr;
    if (id) {
        auto mesh = Get(&manager->meshTable, &id);
        if (mesh) {
            switch (mesh->state) {
            case AssetState::Loaded: {
                result = mesh->mesh;
            } break;
            case AssetState::Unloaded: {
                mesh->state = AssetState::Queued;
                LoadMesh(manager, id);
            } break;
            case AssetState::JustLoaded: {} break;
            case AssetState::Queued: {} break;
            case AssetState::Error: {} break;
                invalid_default();
            }
        }
    }
    return result;
}

Texture* GetTexture(AssetManager* manager, u32 id) {
    Texture* result = nullptr;
    if (id) {
        auto texture = Get(&manager->textureTable, &id);
        if (texture) {
            switch (texture->state) {
            case AssetState::Loaded: {
                result = &texture->texture;
            } break;
            case AssetState::Unloaded: {
                LoadTexture(manager, id);
            } break;
            case AssetState::JustLoaded: {} break;
            case AssetState::Queued: {} break;
            case AssetState::Error: {} break;
                invalid_default();
            }
        }
    }
    return result;
}
