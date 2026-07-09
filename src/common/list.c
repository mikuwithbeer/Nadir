/**
 * @file list.c
 * @brief The list implementation.
 */

#include "nadir/common/list.h"

#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_list_t *nadir_list_new(nadir_arena_t *arena,
                             const nadir_u64_t size) {
    nadir_list_t *list = nadir_arena_allocate(arena, sizeof(nadir_list_t));
    if (list == nullptr) {
        return nullptr;
    }

    list->arena = arena;
    list->items = nullptr; // Will be allocated on first appending

    list->length = 0;
    list->capacity = 0;
    list->size = size;

    return list;
}

bool nadir_list_reserve(nadir_list_t *list,
                        const nadir_u64_t capacity) {
    if (capacity <= list->capacity) {
        return true;
    }

    const auto new_items = nadir_arena_allocate(list->arena, capacity * list->size);
    if (new_items == nullptr) {
        return false;
    }

    if (list->items != nullptr) {
        memcpy(new_items, list->items, list->length * list->size);
    }

    list->items = new_items;
    list->capacity = capacity;

    return true;
}

bool nadir_list_append(nadir_list_t *list,
                       const void *item) {
    if (list->length >= list->capacity) {
        auto new_capacity = list->capacity << 1;
        if (new_capacity == 0) {
            new_capacity = NADIR_LIST_DEFAULT_CAPACITY;
        }

        if (!nadir_list_reserve(list, new_capacity)) {
            return false;
        }
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

    // Return a pointer to the item at the given index.
    return (nadir_u8_t *) list->items + index * list->size;
}

void nadir_list_free(nadir_list_t *list) {
    if (list == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    list->items = nullptr;
    list->length = 0;
    list->capacity = 0;
}
