#ifndef NADIR_COMMON_LIST_H
#define NADIR_COMMON_LIST_H

/**
 * @file list.h
 * @brief The list interface.
 *
 * This file defines the list structure and related constants for
 * the assembler.
 */

#include "nadir/common/number.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_LIST_DEFAULT_CAPACITY = 1 << 4;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Generic list structure for the assembler and components.
 */
typedef struct {
    void *items;

    nadir_u64_t length;
    nadir_u64_t capacity;
    nadir_u64_t size;
} nadir_list_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new list with the given item size.
 *
 * @warning Allocates memory for the list, which must be freed.
 */
[[nodiscard]] nadir_list_t *nadir_list_new(nadir_u64_t size);

/**
 * @brief Appends an item to the list.
 *
 * @return false if the reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_list_append(nadir_list_t *list,
                                     const void *item);

/**
 * @brief Fetches an item from the list by index.
 */
[[nodiscard]] void *nadir_list_get(const nadir_list_t *list,
                                   nadir_u64_t index);

/**
 * @brief Frees the list and its items.
 */
void nadir_list_free(nadir_list_t *list);

#endif //NADIR_COMMON_LIST_H
