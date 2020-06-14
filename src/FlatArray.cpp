#include "FlatArray.h"

template<typename T, typename F>
void ForEach(FlatArray<T>* array, F func) {
    for (usize i = 0; i < array->count; i++) {
        auto it = array->data + i;
        func(it);
    }
}

template <typename T>
void FlatArrayInit(FlatArray<T>* array, Allocator allocator, usize size) {
    assert(!array->capacity);

    array->allocator = allocator;
    array->capacity = size;
    array->count = 0;
    array->initialCount = size;

    if (array->capacity) {
        array->data = (T*)array->allocator.Alloc(sizeof(T) * array->capacity, alignof(T));
        assert(array->data);
    }
}

template <typename T>
T* FlatArrayAt(FlatArray<T>* array, usize index) {
    T* result = nullptr;
    if (index < array->count) {
        result = array->data + index;
    }
    return result;
}

template <typename T>
T* FlatArrayAtUnchecked(FlatArray<T>* array, usize index) {
    T* result = array->data + index;
    return result;
}

template <typename T>
T* FlatArrayPush(FlatArray<T>* array, T value) {
    auto e = array->Push();
    *e = value;
    return e;
}

template <typename T>
T* FlatArrayPush(FlatArray<T>* array) {
    if (array->count == array->capacity) {
        FlatArrayGrow(array, FlatArray<T>::GrowFactor);
    }
    assert(array->count < array->capacity);

    auto e = array->data + array->count;
    array->count++;
    return e;
}

template <typename T>
T* FlatArrayPushArray(FlatArray<T>* array, usize count) {
    assert(array->capacity >= array->count);
    auto free = array->capacity - array->count;
    if (free < count) {
        auto needed = count - free;
        auto factor = needed / array->capacity;
        if (factor < GrowFactor) factor = GrowFactor;
        array->Grow(factor);
    }
    free = array->capacity - array->count;
    assert(free >= count);
    auto e = array->data + array->count;
    array->count += count;
    return e;
}

template <typename T>
T* FlatArrayEnd(FlatArray<T>* array) {
    T* result = nullptr;
    if (array->count) {
        result = array->data + array->count - 1;
    }
    return result;
}

template <typename T>
void FlatArrayClear(FlatArray<T>* array) {
    array->count = 0;
}

template <typename T>
void FlatArrayResize(FlatArray<T>* array, usize size) {
    assert(!array->count);
    assert(array->data);
    array->free(array->data);
    array->data = (T*)array->allocator.Allocate(sizeof(T) * size, alignof(T));
    assert(array->data);
    array->initialCount = size;
}

template <typename T>
void FlatArrayResize(FlatArray<T>* array) {
    array->Resize(array->initialCount);
}

template <typename T>
void FlatArrayGrow(FlatArray<T>* array, usize factor) {
    // TODO:: Realloc
    assert(array->capacity >= array->count);
    assert(array->capacity < (Usize::Max / factor));
    array->capacity *= factor;
    auto newMemory = (T*)array->allocator.Alloc(sizeof(T) * array->capacity, alignof(T));
    assert(newMemory);
    if (array->count > 0) {
        memcpy(newMemory, array->data, sizeof(T) * array->count);
    }
    array->allocator.Dealloc(array->data);
    array->data = newMemory;
}
