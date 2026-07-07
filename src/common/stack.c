/**
 * @file stack.c
 * @brief The stack implementation.
 */

#include "nadir/common/stack.h"

#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_stack_t *nadir_stack_new(nadir_arena_t *arena) {
    nadir_stack_t *stack = nadir_arena_allocate(arena, sizeof(nadir_stack_t));
    if (stack == nullptr) {
        return nullptr;
    }

    stack->arena = arena;
    stack->length = 0;

    return stack;
}

bool nadir_stack_push(nadir_stack_t *stack,
                      const nadir_i128_t value) {
    if (stack->length >= NADIR_STACK_MAXIMUM) {
        return false;
    }

    stack->data[stack->length++] = value;
    return true;
}

bool nadir_stack_pop(nadir_stack_t *stack,
                     nadir_i128_t *value) {
    if (stack->length == 0) {
        return false;
    }

    if (value) {
        *value = stack->data[--stack->length];
    } else {
        --stack->length;
    }

    return true;
}

void nadir_stack_free(nadir_stack_t *stack) {
    if (stack == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    memset(stack->data, 0, sizeof(stack->data));
    stack->length = 0;
}
