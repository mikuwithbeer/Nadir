#include "nadir/common.h"

#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

bool nadir_common_string_to_i128(const char *input,
                                 nadir_i128_t *value) {
    auto result = (typeof(*value)) 0;
    auto negative = false;

    // Check for sign.
    if (*input == '-') {
        negative = true;
        input++;
    } else if (*input == '+') {
        input++;
    }

    // Loop through each character in the string.
    while (*input >= '0' && *input <= '9') {
        const auto digit = *input - '0';

        if (negative) {
            // Check for underflow.
            if (result < (NADIR_I128_MIN + digit) / 10) {
                return false;
            }

            result = result * 10 - digit;
        } else {
            // Check for overflow.
            if (result > (NADIR_I128_MAX - digit) / 10) {
                return false;
            }

            result = result * 10 + digit;
        }

        ++input;
    }

    // Check if we reached the end of the string.
    if (*input != '\0') {
        return false;
    }

    *value = result;
    return true;
}

nadir_list_t *nadir_list_new(const nadir_u64_t size) {
    nadir_list_t *list = malloc(sizeof(nadir_list_t));
    if (list == nullptr) {
        return nullptr;
    }

    list->items = malloc(NADIR_LIST_DEFAULT_CAPACITY * size);
    if (list->items == nullptr) {
        free(list);
        return nullptr;
    }

    list->length = 0;
    list->capacity = NADIR_LIST_DEFAULT_CAPACITY;
    list->size = size;

    return list;
}

bool nadir_list_append(nadir_list_t *list,
                       const void *item) {
    if (list->length >= list->capacity) {
        const auto new_capacity = list->capacity << 1;
        const auto new_items = realloc(list->items, new_capacity * list->size);
        if (new_items == nullptr) {
            return false;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    memcpy((nadir_u8_t *)list->items + list->length * list->size, item, list->size);
    ++list->length;

    return true;
}

void *nadir_list_get(const nadir_list_t *list,
                     const nadir_u64_t index) {
    if (index >= list->length) {
        return nullptr;
    }

    return (nadir_u8_t *) list->items + index * list->size;
}

void nadir_list_free(nadir_list_t *list) {
    if (list == nullptr) {
        return;
    }

    free(list->items);
    free(list);
}
