#pragma once

#include "Common.h"

struct BinaryBlob {
    constant f32 GrowFactor = 2.0f;

    void* data;
    usize size;
    usize at;
    usize free;
    Allocator allocator;

    void* Write(usize size) {
        if (size < this->free) {
            auto newSize = (u32)((f32)this->size * GrowFactor);
            if ((newSize - this->at) < size) {
                newSize = size - (this->size - this->at);
            }
            // TODO: Realloc or something
            auto newMem = this->allocator.Alloc(newSize, DefaultAligment);
            assert(newMem);
            memcpy(newMem, this->data, this->at);
            this->allocator.Dealloc(this->data);
            this->data = newMem;
            this->size = newSize;
            this->free = newSize - this->at;
        }

        assert(size < this->free);

        void* result = ((u8*)this->data + this->at);
        this->at += size;
        this->free -= size;
        return result;
    }

    template <typename T>
    void Write(T* data) {
        auto mem = this->Write(sizeof(T));
        memcpy(mem, data, sizeof(T));
    }

    void Destroy() {
        this->allocator.Dealloc(this->data);
        *this = {};
    }

    static void Init(BinaryBlob* blob, Allocator allocator, usize size = 256) {
        blob->allocator = allocator;
        blob->at = 0;

        blob->data = allocator.Alloc(size, DefaultAligment);
        assert(blob->data);
        blob->size = size;
        blob->free = size;
    }
};
