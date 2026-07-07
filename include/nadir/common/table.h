#ifndef NADIR_COMMON_TABLE_H
#define NADIR_COMMON_TABLE_H

/**
 * @file table.h
 * @brief The table interface.
 *
 * This file defines the table structure and related constants for
 * the assembler.
 */

#include "nadir/common/number.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_TABLE_DEFAULT_CAPACITY = 1 << 6;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Generic table entry structure for the assembler and components.
 */
typedef struct {
    char *key;
    nadir_u64_t key_length;

    void *value;
    bool exists;
} nadir_table_entry_t;

/**
 * @brief Generic table structure for the assembler and components.
 */
typedef struct {
    nadir_table_entry_t *entries;

    nadir_u64_t length;
    nadir_u64_t capacity;
    nadir_u64_t size;
} nadir_table_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new table with the given value size.
 *
 * @warning Allocates memory for the table, which must be freed.
 */
[[nodiscard]] nadir_table_t *nadir_table_new(nadir_u64_t size);

/**
 * @brief Inserts a key-value pair into the table.
 *
 * @return false if the key already exists or reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_table_insert(nadir_table_t *table,
                                      const char *key,
                                      nadir_u64_t length,
                                      const void *value);

/**
 * @brief Fetches a value from the table by key.
 */
[[nodiscard]] void *nadir_table_fetch(const nadir_table_t *table,
                                      const char *key,
                                      nadir_u64_t length);

/**
 * @brief Frees the table and its values.
 */
void nadir_table_free(nadir_table_t *table);

#endif //NADIR_COMMON_TABLE_H
