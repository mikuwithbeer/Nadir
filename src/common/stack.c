/**
 * @file stack.c
 * @brief The stack implementation.
 */

#include "nadir/common/stack.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_stack_t *nadir_stack_new(void) {
    const auto stack = calloc(1, sizeof(nadir_stack_t));
    if (stack == nullptr) {
        return nullptr;
    }

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

    free(stack);
}
