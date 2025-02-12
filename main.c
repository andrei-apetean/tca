#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define TCA_IMPLEMENTATION
#include "tca.h"

void test_arena_allocator_basic_allocation() {
    printf("Test: Basic Allocation\n");
    arena_allocator* arena = arena_allocator_new(1024);
    assert(arena != NULL && "Arena allocation failed!");
    
    void* mem1 = arena_allocator_alloc(arena, 256, 8);
    assert(mem1 != NULL && "Memory allocation failed!");
    
    void* mem2 = arena_allocator_alloc(arena, 512, 16);
    assert(mem2 != NULL && "Second allocation failed!");
    
    assert((uintptr_t)mem2 > (uintptr_t)mem1 && "Memory blocks should be sequential!");

    arena_allocator_destroy(arena);
    printf("Passed Basic Allocation\n");
}

void test_arena_allocator_alignment() {
    printf("Test: Alignment\n");
    arena_allocator* arena = arena_allocator_new(1024);
    
    void* mem1 = arena_allocator_alloc(arena, 100, 16);
    void* mem2 = arena_allocator_alloc(arena, 200, 32);
    
    assert(((uintptr_t)mem1 % 16) == 0 && "Alignment error: mem1 is not 16-byte aligned!");
    assert(((uintptr_t)mem2 % 32) == 0 && "Alignment error: mem2 is not 32-byte aligned!");

    arena_allocator_destroy(arena);
    printf("Passed Alignment Test\n");
}

void test_arena_allocator_out_of_memory() {
    printf("Test: Out of Memory Handling\n");
    arena_allocator* arena = arena_allocator_new(512);
    
    void* mem1 = arena_allocator_alloc(arena, 400, 8);
    assert(mem1 != NULL && "Memory allocation failed unexpectedly!");
    
    void* mem2 = arena_allocator_alloc(arena, 200, 8); // Should fail
    assert(mem2 == NULL && "Out-of-memory check failed!");

    arena_allocator_destroy(arena);
    printf("Passed Out of Memory Test\n");
}

void test_arena_allocator_reset() {
    printf("Test: Reset Functionality\n");
    arena_allocator* arena = arena_allocator_new(512);
    
    void* mem1 = arena_allocator_alloc(arena, 256, 8);
    assert(mem1 != NULL);

    arena_allocator_reset(arena);  // Should reset offset to 0
    
    void* mem2 = arena_allocator_alloc(arena, 512, 8); // Should succeed
    assert(mem2 != NULL && "Reset failed: should allow reusing memory!");

    arena_allocator_destroy(arena);
    printf("Passed Reset Test\n");
}

void test_arena_allocator_placement_new() {
    // Allocate a large block of memory for arena_allocator and data
    size_t total_size = 1024;
    void* memory_block = malloc(total_size);  // Raw memory block

    // Now, create an arena_allocator using placement new
    arena_allocator* allocator = NULL;
    arena_allocator_placement_new(&allocator, memory_block, total_size);

    // Check if allocator is properly initialized
    assert(allocator != NULL && "Allocator should not be NULL after placement new!");
    assert(allocator->capacity == total_size - sizeof(arena_allocator) && 
           "Allocator capacity not set correctly!");
    assert(allocator->offset == 0 && "Allocator offset should be 0 initially!");
    assert(allocator->data != NULL && "Allocator data pointer should not be NULL!");

    // Check if the data memory is zeroed out (as per `memset`)
    for (size_t i = 0; i < allocator->capacity; ++i) {
        assert(((uint8_t*)allocator->data)[i] == 0 && "Data area is not zeroed!");
    }

    // Clean up the memory block
    free(memory_block);

    printf("Passed Placement New Test\n");
}

void test_arena_allocator_free_behavior() {
    printf("Test: Free Behavior\n");
    arena_allocator* arena = arena_allocator_new(512);
    
    void* mem1 = arena_allocator_alloc(arena, 256, 8);
    assert(mem1 != NULL);
    
    arena_allocator_free(arena, 256);
    assert(arena->offset == 0 && "Free did not correctly shrink the offset!");

    void* mem2 = arena_allocator_alloc(arena, 512, 8);
    assert(mem2 != NULL);

    arena_allocator_destroy(arena);
    printf("Passed Free Behavior Test\n");
}

int main() {
    test_arena_allocator_basic_allocation();
    test_arena_allocator_alignment();
    test_arena_allocator_out_of_memory();
    test_arena_allocator_reset();
    test_arena_allocator_free_behavior();
    test_arena_allocator_placement_new();
    printf("\nAll tests passed successfully!\n");
    return 0;
}
