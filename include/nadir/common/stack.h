#ifndef NADIR_COMMON_STACK_H
#define NADIR_COMMON_STACK_H

/**
 * @file stack.h
 * @brief The stack interface.
 *
 * This file defines the stack structure and related constants for
 * the assembler.
 */

#include "nadir/common/arena.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_STACK_MAXIMUM = 1 << 10;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Generic stack structure for the assembler and components.
 */
typedef struct {
    nadir_arena_t *arena;
    nadir_u64_t length;

    nadir_i128_t data[NADIR_STACK_MAXIMUM];
} nadir_stack_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new stack with the given arena.
 */
[[nodiscard]] nadir_stack_t *nadir_stack_new(nadir_arena_t *arena);

/**
 * @brief Pushes a value onto the stack.
 */
[[nodiscard]] bool nadir_stack_push(nadir_stack_t *stack,
                                    nadir_i128_t value);

/**
 * @brief Pops a value from the stack.
 *
 * @return false if the stack is empty, true otherwise.
 */
[[nodiscard]] bool nadir_stack_pop(nadir_stack_t *stack,
                                   nadir_i128_t *value);

/**
 * @brief Frees the stack.
 *
 * @warning The stack is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_stack_free(nadir_stack_t *stack);

#endif //NADIR_COMMON_STACK_H
