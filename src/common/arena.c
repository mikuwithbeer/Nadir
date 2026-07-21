/**
 * @file arena.c
 * @brief The arena implementation.
 */

#include "nadir/common/arena.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_arena_packet_t *nadir_arena_allocate_packet(nadir_u64_t capacity);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

bool nadir_arena_init(nadir_arena_t *arena,
                      const nadir_u64_t default_capacity) {
    auto const packet = nadir_arena_allocate_packet(default_capacity);
    if (packet == nullptr) {
        return false;
    }

    arena->head = packet;
    arena->current = arena->head;
    arena->default_capacity = default_capacity;
    return true;
}

void *nadir_arena_allocate(nadir_arena_t *arena,
                           const nadir_u64_t size) {
    // Align the allocation to a 16-byte boundary.
    auto const current_pointer = (nadir_u64_t) (arena->current->value + arena->current->offset);
    auto const aligned_pointer = (current_pointer + 15) & ~(nadir_u64_t) 15;
    auto const padding_pointer = aligned_pointer - current_pointer;

    if (arena->current->offset + padding_pointer + size > arena->current->capacity) {
        auto new_capacity = arena->default_capacity;
        if (new_capacity < size + 16) {
            // Make sure the requested allocation still fits after alignment.
            new_capacity = size + 16;
        }

        auto const new_packet = nadir_arena_allocate_packet(new_capacity);
        if (new_packet == nullptr) {
            return nullptr;
        }

        arena->current->next = new_packet;
        arena->current = new_packet;

        // Align the allocation to a 16-byte boundary.
        auto const new_current_pointer = (nadir_u64_t) arena->current->value;
        auto const new_aligned_pointer = (new_current_pointer + 15) & ~(nadir_u64_t) 15;
        auto const new_padding_pointer = new_aligned_pointer - new_current_pointer;

        arena->current->offset = new_padding_pointer + size;
        return (void *) new_aligned_pointer;
    }

    arena->current->offset += padding_pointer + size;
    return (void *) aligned_pointer;
}

void nadir_arena_reset(nadir_arena_t *arena) {
    auto packet = arena->head;
    while (packet != nullptr) {
        packet->offset = 0; // Reset the offset for reuse
        packet = packet->next;
    }

    // Start back at the first package.
    arena->current = arena->head;
}

void nadir_arena_free(nadir_arena_t *arena) {
    auto packet = arena->head;
    while (packet != nullptr) {
        auto const next = packet->next;
        free(packet);
        packet = next;
    }

    arena->head = nullptr;
    arena->current = nullptr;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_arena_packet_t *nadir_arena_allocate_packet(const nadir_u64_t capacity) {
    // Flexible array to avoid multiple allocation.
    nadir_arena_packet_t *packet = malloc(sizeof(nadir_arena_packet_t) + capacity);
    if (packet == nullptr) {
        return nullptr;
    }

    packet->next = nullptr;
    packet->capacity = capacity;
    packet->offset = 0;

    return packet;
}
