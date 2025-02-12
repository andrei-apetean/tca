#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct arena_allocator arena_allocator;
typedef struct stack_allocator stack_allocator;
typedef struct pool_allocator  pool_allocator;
typedef struct ll_allocator    ll_allocator;

arena_allocator* arena_allocator_new(size_t size);
void  arena_allocator_placement_new(arena_allocator** allocator, void* data,
                                    size_t size);
void* arena_allocator_alloc(arena_allocator* allocator, size_t size,
                            size_t alignment);
void  arena_allocator_free(arena_allocator* allocator, size_t size);
void  arena_allocator_reset(arena_allocator* allocator);
void  arena_allocator_destroy(arena_allocator* allocator);

// #define MEM_ALLOC_IMPLEMENTATION
#ifdef TCA_IMPLEMENTATION
// implementation based on this article
// http://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

typedef struct arena_allocator {
    void*  data;
    size_t capacity;
    size_t offset;
} arena_allocator;

typedef struct stack_allocator {
    int a;
} stack_allocator;

typedef struct pool_allocator {
    int a;
} pool_allocator;

typedef struct ll_allocator {
    int a;
} ll_allocator;

static bool is_power_of_two(uintptr_t ptr) { return ptr && !(ptr & (ptr - 1)); }

static uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t mask = alignment - 1;
    return (ptr + mask) & ~mask;
}

arena_allocator* arena_allocator_new(size_t size) {
    size_t           total = size + sizeof(arena_allocator);
    arena_allocator* allocator = (arena_allocator*)malloc(total);
    if (!allocator) {
        return NULL;
    }
    allocator->capacity = size;
    allocator->offset = 0;
    allocator->data =
        memset((uint8_t*)allocator + sizeof(arena_allocator), 0, size);
    return allocator;
}

void arena_allocator_placement_new(arena_allocator** allocator, void* data,
                                   size_t size) {
    assert(allocator != NULL && "Allocator cannot be null!\n");
    assert(data != NULL && "Allocator buffer cannot be null!\n");
    *allocator = (arena_allocator*)data;
    (*allocator)->capacity = size - sizeof(arena_allocator);
    (*allocator)->offset = 0;
    (*allocator)->data =
        memset((uint8_t*)*allocator + sizeof(arena_allocator), 0, size);
}

void* arena_allocator_alloc(arena_allocator* allocator, size_t size, size_t alignment) {
    assert(allocator != NULL && "Allocator cannot be null!\n");
    assert(is_power_of_two(alignment) && "Alignment must be a power of two!\n");

    uintptr_t base = (uintptr_t)allocator->data + allocator->offset;
    uintptr_t aligned_offset = align_forward(base, alignment) - (uintptr_t)allocator->data;

    if (aligned_offset + size > allocator->capacity) {
        return NULL;  // Not enough space
    }

    void* ptr = (uint8_t*)allocator->data + aligned_offset;
    allocator->offset = aligned_offset + size;  // Update offset properly

    return ptr;
}

void arena_allocator_free(arena_allocator* allocator, size_t size) {
    assert(allocator != NULL && "Allocator cannot be null!\n");
    allocator->offset =
        (allocator->offset >= size) ? (allocator->offset - size) : 0;
}

void arena_allocator_reset(arena_allocator* allocator) {
    assert(allocator != NULL && "Allocator cannot be null!\n");
    allocator->offset = 0;
}

void arena_allocator_destroy(arena_allocator* allocator) {
    assert(allocator != NULL && "Allocator cannot be null!\n");
    free(allocator);
}

#endif  // MEM_ALLOC_IMPLEMENTATION
