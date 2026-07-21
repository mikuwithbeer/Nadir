#ifndef NADIR_COMMON_LIST_H
#define NADIR_COMMON_LIST_H

/**
 * @file list.h
 * @brief The list interface.
 *
 * This file defines the list structure and related constants for
 * the assembler.
 */

#include "nadir/common/arena.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_LIST_DEFAULT_CAPACITY = 1 << 2;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Generic list structure for the assembler and components.
 */
typedef struct {
    nadir_arena_t *arena;
    void *items;

    nadir_u64_t length;
    nadir_u64_t capacity;
    nadir_u64_t size;
} nadir_list_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new list with the given arena and item size.
 */
[[nodiscard]] nadir_list_t *nadir_list_new(nadir_arena_t *arena,
                                           nadir_u64_t size);

/**
 * @brief Reserves space for the list to hold at least the given capacity.
 *
 * @return false if the allocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_list_reserve(nadir_list_t *list, uint64_t capacity);

/**
 * @brief Appends an item to the list, reallocating if necessary.
 *
 * @return false if the reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_list_append(nadir_list_t *list,
                                     const void *item);

/**
 * @brief Fills the list with the given item and count.
 *
 * @return false if the reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_list_fill(nadir_list_t *list,
                                   const void *item,
                                   nadir_u64_t count);

/**
 * @brief Gets an item from the list at the given index.
 */
[[nodiscard]] void *nadir_list_get(const nadir_list_t *list,
                                   nadir_u64_t index);

/**
 * @brief Frees the list and its items.
 *
 * @warning The list is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_list_free(nadir_list_t *list);

#endif //NADIR_COMMON_LIST_H
