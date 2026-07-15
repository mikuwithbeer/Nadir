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
    const auto packet = nadir_arena_allocate_packet(default_capacity);
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
    const auto alignment = size + 7 & ~(nadir_u64_t) 7; // Align to 8 bytes

    // Check if the current packet has enough space for the requested allocation.
    if (arena->current->offset + alignment > arena->current->capacity) {
        auto new_capacity = arena->default_capacity;
        if (new_capacity < alignment) {
            new_capacity = alignment;
        }

        const auto new_packet = nadir_arena_allocate_packet(new_capacity);
        if (new_packet == nullptr) {
            return nullptr;
        }

        // Link the new packet and advance.
        arena->current->next = new_packet;
        arena->current = new_packet;
    }


    const auto pointer = arena->current->value + arena->current->offset;
    arena->current->offset += alignment; // Bump the offset

    return pointer;
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
        const auto next = packet->next;
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
    nadir_arena_packet_t *packet = malloc(sizeof(nadir_arena_packet_t) + capacity);
    if (packet == nullptr) {
        return nullptr;
    }

    packet->next = nullptr;
    packet->capacity = capacity;
    packet->offset = 0;

    return packet;
}
