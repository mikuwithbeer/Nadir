#ifndef NADIR_COMMON_ARENA_H
#define NADIR_COMMON_ARENA_H

/**
 * @file arena.h
 * @brief The arena interface.
 *
 * This file defines the arena allocator structure and related
 * constants for the assembler.
 */

#include "nadir/common/number.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_ARENA_DEFAULT_CAPACITY = 1 << 4;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Arena packet structure for the assembler and components.
 */
typedef struct nadir_arena_packet_t {
    struct nadir_arena_packet_t *next;
    nadir_u64_t capacity;
    nadir_u64_t offset;

    nadir_u8_t value[]; // Flexible array member
} nadir_arena_packet_t;

/**
 * @brief Arena structure for the assembler and components.
 */
typedef struct {
    nadir_arena_packet_t *head;
    nadir_arena_packet_t *current;
    nadir_u64_t default_capacity;
} nadir_arena_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Initializes the arena with the given default capacity.
 */
[[nodiscard]] bool nadir_arena_init(nadir_arena_t *arena,
                                    nadir_u64_t default_capacity);

/**
 * @brief Allocates memory from the arena.
 */
[[nodiscard]] void *nadir_arena_allocate(nadir_arena_t *arena,
                                         nadir_u64_t size);

/**
 * @brief Resets the arena's allocation offset to zero.
 *
 * @warning All previously allocated memory will be considered
 * invalid after this operation. Accessing any memory allocated
 * before the reset will lead to undefined behavior.
 */
void nadir_arena_reset(nadir_arena_t *arena);

/**
 * @brief Frees all memory associated with the arena, including all packets.
 */
void nadir_arena_free(nadir_arena_t *arena);

#endif //NADIR_COMMON_ARENA_H
