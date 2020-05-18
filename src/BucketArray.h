#pragma once
#include "Common.h"

#define bucket_array_template_decl template<typename T, usize BucketCapacity>
#define bucket_array_decl BucketArray<T, BucketCapacity>

bucket_array_template_decl
struct BucketArray {
    struct Bucket {
        Bucket* next;
        usize at;
        T data[BucketCapacity];
    };

    Allocator allocator;

    Bucket* firstBucket;
    usize bucketCount;
    usize entryCount;

    struct Iterator {
        Bucket* currentBucket;
        usize index;
        T* Begin() {return currentBucket ? currentBucket->data : nullptr; }
        T* Get() { return currentBucket->data + index; }
        void Advance() { index++; if (index == currentBucket->at) { index = 0; currentBucket = currentBucket->next; } }
        bool End() { return currentBucket == nullptr; }
    };

    inline Iterator GetIterator() { return Iterator { firstBucket, 0 }; }
};

bucket_array_template_decl
void BucketArrayInit(bucket_array_decl* array, Allocator allocator) {
    assert(!array->allocator.allocate);
    array->allocator = allocator;
}

bucket_array_template_decl
T* BucketArrayPush(bucket_array_decl* array) {
    T* result = nullptr;
    if (!array->firstBucket) {
        array->firstBucket = (bucket_array_decl::Bucket*)array->allocator.Alloc(sizeof(bucket_array_decl::Bucket), alignof(bucket_array_decl::Bucket));
        if (array->firstBucket) {
            array->firstBucket->at = 0;
            array->firstBucket->next = nullptr;
            array->bucketCount++;
        }
    } else if (array->firstBucket->at == BucketCapacity) {
        auto newBlock = (bucket_array_decl::Bucket*)array->allocator.Alloc(sizeof(bucket_array_decl::Bucket), alignof(bucket_array_decl::Bucket));
        if (newBlock) {
            array->bucketCount++;
            newBlock->at = 0;
            newBlock->next = array->firstBucket;
            array->firstBucket = newBlock;
        }
    }

    if (array->firstBucket && array->firstBucket->at < BucketCapacity) {
        result = array->firstBucket->data + array->firstBucket->at;
        array->firstBucket->at++;
        array->entryCount++;
    }
    return result;
}

bucket_array_template_decl
void BucketArrayRemove(bucket_array_decl* array, T* entry) {
    assert(array->firstBucket && array->firstBucket->at);
    auto lastEntry = array->firstBucket->data + (array->firstBucket->at - 1);
    array->firstBucket->at--;
    array->entryCount--;
    if (entry != lastEntry) {
        *entry = *lastEntry;
    }
    if (!array->firstBucket->at) {
        auto empty = array->firstBucket;
        auto nextBlock = array->firstBucket->next;
        array->firstBucket = nextBlock;
        assert(array->bucketCount);
        assert((!array->firstBucket) ? (array->entryCount == 0) : true);
        array->allocator.Dealloc(empty);
        array->bucketCount--;
    }
}

bucket_array_template_decl
void BucketArrayClear(bucket_array_decl* array) {
    auto bucket = array->firstBucket;
    while (bucket) {
        auto bucketToDelete = bucket;
        bucket = bucket->next;
        array->allocator.Dealloc(bucketToDelete);
    }
    array->firstBucket = nullptr;
    array->bucketCount = 0;
    array->entryCount = 0;
}
