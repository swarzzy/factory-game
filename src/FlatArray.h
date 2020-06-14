#pragma once

#include "Common.h"

template <typename T>
struct FlatArray {
    static const u32 GrowFactor = 2;
    Allocator allocator;

    T* data;
    usize capacity;
    usize count;
    usize initialCount;
};

template<typename T, typename F>
void ForEach(FlatArray<T>* array, F func);

template <typename T>
void FlatArrayInit(FlatArray<T>* array, Allocator allocator, usize size = 16);

template <typename T>
T* FlatArrayAt(FlatArray<T>* array, usize index);

template <typename T>
T* FlatArrayAtUnchecked(FlatArray<T>* array, usize index);

template <typename T>
T* FlatArrayPush(FlatArray<T>* array, T value);

template <typename T>
T* FlatArrayPush(FlatArray<T>* array);

template <typename T>
T* FlatArrayPushArray(FlatArray<T>* array, usize count);

template <typename T>
T* FlatArrayEnd(FlatArray<T>* array);

template <typename T>
void FlatArrayClear(FlatArray<T>* array);

template <typename T>
void FlatArrayResize(FlatArray<T>* array, usize size);

template <typename T>
void FlatArrayResize(FlatArray<T>* array);

template <typename T>
void FlatArrayGrow(FlatArray<T>* array, usize factor);
