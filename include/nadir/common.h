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

constexpr auto NADIR_STRING_MAXIMUM = 0xFF + 1;
constexpr auto NADIR_LIST_DEFAULT_CAPACITY = 1 << 6;
constexpr auto NADIR_TABLE_DEFAULT_CAPACITY = NADIR_LIST_DEFAULT_CAPACITY;
constexpr auto NADIR_STACK_MAXIMUM = 1 << 10;

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

/**
 * @brief Table entry structure for the table structure.
 */
typedef struct {
    char key[NADIR_STRING_MAXIMUM];
    void *value;
    bool is_used;
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
 * @brief Decodes a string into a 128-bit signed integer.
 */
[[nodiscard]] bool nadir_i128_decode(const char *input,
                                     nadir_i128_t *value);

/**
 * @brief Creates a new list with the given data size.
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
 * @brief Creates a new table with the given data size.
 *
 * @warning Allocates memory for the table, which must be freed.
 */
[[nodiscard]] nadir_table_t *nadir_table_new(nadir_u64_t size);

/**
 * @brief Inserts a key-value pair into the table.
 */
[[nodiscard]] bool nadir_table_insert(nadir_table_t *table,
                                      const char *key,
                                      const void *value);

/**
 * @brief Fetches a value from the table by key.
 */
[[nodiscard]] void *nadir_table_fetch(const nadir_table_t *table,
                                      const char *key);

/**
 * @brief Frees the table and its items.
 */
void nadir_table_free(nadir_table_t *table);

/**
 * @brief Creates a new stack.
 */
[[nodiscard]] nadir_stack_t nadir_stack_new(void);

/**
 * @brief Pushes a value onto the stack.
 */
[[nodiscard]] bool nadir_stack_push(nadir_stack_t *stack,
                                    nadir_i128_t value);

/**
 * @brief Pops a value from the stack.
 */
[[nodiscard]] bool nadir_stack_pop(nadir_stack_t *stack,
                                   nadir_i128_t *value);

#endif //NADIR_COMMON_H
