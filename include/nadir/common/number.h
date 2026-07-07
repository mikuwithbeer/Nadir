#ifndef NADIR_COMMON_NUMBER_H
#define NADIR_COMMON_NUMBER_H

/**
 * @file number.h
 * @brief The number interface.
 */

#include <stdint.h>

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

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

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

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

#endif //NADIR_COMMON_NUMBER_H
