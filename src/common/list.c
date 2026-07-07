/**
 * @file list.c
 * @brief The list implementation.
 */

#include "nadir/common/list.h"

#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_list_t *nadir_list_new(const nadir_u64_t size) {
    nadir_list_t *list = malloc(sizeof(nadir_list_t));
    if (list == nullptr) {
        return nullptr;
    }

    list->length = 0;
    list->capacity = NADIR_LIST_DEFAULT_CAPACITY;
    list->size = size;

    void *items = malloc(list->capacity * list->size);
    if (items == nullptr) {
        free(list);
        return nullptr;
    }

    list->items = items;
    return list;
}

bool nadir_list_append(nadir_list_t *list,
                       const void *item) {
    if (list->length >= list->capacity) {
        const auto new_capacity = list->capacity << 1; // Double the capacity
        const auto new_items = realloc(list->items, new_capacity * list->size);
        if (new_items == nullptr) {
            return false;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    // Copy the item into the list and increment the length.
    memcpy((nadir_u8_t *)list->items + (list->length++) * list->size, item, list->size);
    return true;
}

void *nadir_list_get(const nadir_list_t *list,
                     const nadir_u64_t index) {
    if (index >= list->length) {
        return nullptr;
    }

    // Return a pointer to the item at the given index.
    return (nadir_u8_t *) list->items + index * list->size;
}

void nadir_list_free(nadir_list_t *list) {
    if (list == nullptr) {
        return;
    }

    if (list->items != nullptr) {
        free(list->items);
        list->items = nullptr;
    }

    free(list);
}
