#ifndef NADIR_COMMON_H
#define NADIR_COMMON_H

/**
 * @file common.h
 * @brief The common interface.
 *
 * This file defines common types, constants, and data structures used
 * throughout the assembler and its components.
 */

#include <stdint.h>

typedef uint8_t nadir_u8_t; // 8-bit unsigned integer
typedef uint16_t nadir_u16_t; // 16-bit unsigned integer
typedef uint32_t nadir_u32_t; // 32-bit unsigned integer
typedef uint64_t nadir_u64_t; // 64-bit unsigned integer
typedef unsigned _BitInt(128) nadir_u128_t; // 128-bit unsigned integer

typedef int8_t nadir_i8_t; // 8-bit signed integer
typedef int16_t nadir_i16_t; // 16-bit signed integer
typedef int32_t nadir_i32_t; // 32-bit signed integer
typedef int64_t nadir_i64_t; // 64-bit signed integer
typedef _BitInt(128) nadir_i128_t; // 128-bit signed integer

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr nadir_u8_t NADIR_U8_MAXIMUM = 0xFF; // 2^8 - 1
constexpr nadir_u8_t NADIR_U8_MINIMUM = 0;

constexpr nadir_u16_t NADIR_U16_MAXIMUM = 0xFFFF; // 2^16 - 1
constexpr nadir_u16_t NADIR_U16_MINIMUM = 0;

constexpr nadir_u32_t NADIR_U32_MAXIMUM = 0xFFFFFFFFU; // 2^32 - 1
constexpr nadir_u32_t NADIR_U32_MINIMUM = 0;

constexpr nadir_u64_t NADIR_U64_MAXIMUM = 0xFFFFFFFFFFFFFFFFULL; // 2^64 - 1
constexpr nadir_u64_t NADIR_U64_MINIMUM = 0;

constexpr nadir_u128_t NADIR_U128_MAXIMUM = ~(nadir_u128_t) 0; // 2^128 - 1
constexpr nadir_u128_t NADIR_U128_MINIMUM = 0;

constexpr nadir_i8_t NADIR_I8_MAXIMUM = NADIR_U8_MAXIMUM >> 1; // 2^7 - 1
constexpr nadir_i8_t NADIR_I8_MINIMUM = -NADIR_I8_MAXIMUM - 1; // -2^7

constexpr nadir_i16_t NADIR_I16_MAXIMUM = NADIR_U16_MAXIMUM >> 1; // 2^15 - 1
constexpr nadir_i16_t NADIR_I16_MINIMUM = -NADIR_I16_MAXIMUM - 1; // -2^15

constexpr nadir_i32_t NADIR_I32_MAXIMUM = NADIR_U32_MAXIMUM >> 1; // 2^31 - 1
constexpr nadir_i32_t NADIR_I32_MINIMUM = -NADIR_I32_MAXIMUM - 1; // -2^31

constexpr nadir_i64_t NADIR_I64_MAXIMUM = NADIR_U64_MAXIMUM >> 1; // 2^63 - 1
constexpr nadir_i64_t NADIR_I64_MINIMUM = -NADIR_I64_MAXIMUM - 1; // -2^63

constexpr nadir_i128_t NADIR_I128_MAXIMUM = NADIR_U128_MAXIMUM >> 1; // 2^127 - 1
constexpr nadir_i128_t NADIR_I128_MINIMUM = -NADIR_I128_MAXIMUM - 1; // -2^127

constexpr auto NADIR_STRING_MAXIMUM = 0xFF + 1; // 255 + 1 for null terminator
constexpr auto NADIR_LIST_DEFAULT_CAPACITY = 1 << 4;
constexpr auto NADIR_TABLE_DEFAULT_CAPACITY = NADIR_LIST_DEFAULT_CAPACITY;
constexpr auto NADIR_STACK_MAXIMUM = 1 << 10;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Type kinds for the assembler and components.
 */
typedef enum : nadir_u8_t {
    NADIR_TYPE_U8,
    NADIR_TYPE_U16,
    NADIR_TYPE_U32,
    NADIR_TYPE_U64,
    NADIR_TYPE_I8,
    NADIR_TYPE_I16,
    NADIR_TYPE_I32,
    NADIR_TYPE_I64,
} nadir_type_t;

/**
 * @brief Generic list structure for the assembler and components.
 */
typedef struct {
    void *items;

    nadir_u64_t length;
    nadir_u64_t capacity;
    nadir_u64_t size; // Size of each item in the list
} nadir_list_t;

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
    nadir_u64_t size; // Size of each value in the table
} nadir_table_t;

/**
 * @brief Generic stack structure for the assembler and components.
 */
typedef struct {
    nadir_i128_t data[NADIR_STACK_MAXIMUM];
    nadir_u64_t length;
} nadir_stack_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Compares two strings for equality.
 */
[[nodiscard]] bool nadir_string_compare(const char *left,
                                        const char *right,
                                        nadir_u64_t left_length,
                                        nadir_u64_t right_length);

/**
 * @brief Decodes a base-10 string into a 128-bit signed integer.
 */
[[nodiscard]] bool nadir_i128_decode_base10(const char *input,
                                            nadir_u64_t length,
                                            nadir_i128_t *value);

/**
 * @brief Decodes a base-16 string into a 128-bit signed integer.
 */
[[nodiscard]] bool nadir_i128_decode_base16(const char *input,
                                            nadir_u64_t length,
                                            nadir_i128_t *value);

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
 * @brief Gets an item from the list at the given index.
 */
[[nodiscard]] void *nadir_list_get(const nadir_list_t *list,
                                   nadir_u64_t index);

/**
 * @brief Frees the list and its items.
 */
void nadir_list_free(nadir_list_t *list);

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

/**
 * @brief Creates a new stack.
 *
 * @warning Allocates memory for the stack, which must be freed.
 */
[[nodiscard]] nadir_stack_t *nadir_stack_new(void);

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
 */
void nadir_stack_free(nadir_stack_t *stack);

#endif //NADIR_COMMON_H
