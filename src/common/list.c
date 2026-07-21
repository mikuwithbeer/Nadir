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

    auto const new_items = nadir_arena_allocate(list->arena, capacity * list->size);
    if (new_items == nullptr) {
        return false;
    }

    if (list->items != nullptr) {
        // Copy existing items to the new memory block.
        memcpy(new_items, list->items, list->length * list->size);
    }

    list->items = new_items;
    list->capacity = capacity;

    return true;
}

bool nadir_list_append(nadir_list_t *list,
                       const void *item) {
    if (list->length >= list->capacity) {
        auto new_capacity = list->capacity << 1; // Double the capacity
        if (new_capacity == 0) {
            new_capacity = NADIR_LIST_DEFAULT_CAPACITY;
        }

        if (!nadir_list_reserve(list, new_capacity)) {
            return false;
        }
    }

    auto const destination = (nadir_u8_t *) list->items + list->length * list->size;
    memcpy(destination, item, list->size);

    ++list->length;
    return true;
}

bool nadir_list_fill(nadir_list_t *list,
                     const void *item,
                     const nadir_u64_t count) {
    // Job is already done :)
    if (count == 0) {
        return true;
    }

    auto const required_capacity = list->length + count;
    if (required_capacity > list->capacity) {
        auto new_capacity = list->capacity << 1; // Double the capacity
        if (new_capacity == 0) {
            new_capacity = NADIR_LIST_DEFAULT_CAPACITY;
        }

        // Increase the capacity until it fits.
        while (new_capacity < required_capacity) {
            new_capacity <<= 1; // Double the capacity
        }

        // Reserve the new memory block.
        if (!nadir_list_reserve(list, new_capacity)) {
            return false;
        }
    }

    // We manually copy the first item into the buffer.
    // Using this item as the reference to clone all the others.
    auto const destination = (nadir_u8_t *) list->items + list->length * list->size;
    memcpy(destination, item, list->size);

    auto copy_count = (nadir_u64_t) 1;
    auto copy_bytes = list->size;

    // Instead of copying the item one by one, we copy the entire block we just wrote.
    while (copy_count < count) {
        auto const remaining = count - copy_count;

        auto chunk = remaining;
        if (copy_count < remaining) {
            chunk = copy_count;
        }

        memcpy(destination + copy_bytes, destination, chunk * list->size);

        copy_count += chunk;
        copy_bytes += chunk * list->size;
    }

    list->length += count;
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
