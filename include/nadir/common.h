#ifndef NADIR_COMMON_H
#define NADIR_COMMON_H

#include <stdint.h>

typedef uint8_t nadir_u8_t;
typedef uint16_t nadir_u16_t;
typedef uint32_t nadir_u32_t;
typedef uint64_t nadir_u64_t;
typedef unsigned _BitInt(128) nadir_u128_t;

typedef int8_t nadir_i8_t;
typedef int16_t nadir_i16_t;
typedef int32_t nadir_i32_t;
typedef int64_t nadir_i64_t;
typedef _BitInt(128) nadir_i128_t;

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr nadir_u8_t NADIR_U8_MAX = 0xFF;
constexpr nadir_u16_t NADIR_U16_MAX = 0xFFFF;
constexpr nadir_u32_t NADIR_U32_MAX = 0xFFFFFFFF;
constexpr nadir_u64_t NADIR_U64_MAX = 0xFFFFFFFFFFFFFFFF;
constexpr nadir_u128_t NADIR_U128_MAX = ~(nadir_u128_t) 0;

constexpr nadir_i128_t NADIR_I128_MAX = NADIR_U128_MAX >> 1;
constexpr nadir_i128_t NADIR_I128_MIN = -NADIR_I128_MAX - 1;

constexpr nadir_u64_t NADIR_LIST_DEFAULT_CAPACITY = 1 << 10;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Generic list structure for the assembler and components.
 */
typedef struct {
    void **items;

    nadir_u64_t length;
    nadir_u64_t capacity;
    nadir_u64_t size;
} nadir_list_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Converts a string to a 128-bit signed integer.
 *
 * @return true if the conversion was successful, false otherwise.
 */
[[nodiscard]] bool nadir_common_string_to_i128(const char *input,
                                               nadir_i128_t *value);

/**
 * @brief Creates a new list with the given capacity.
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

#endif //NADIR_COMMON_H
