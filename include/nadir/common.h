#ifndef NADIR_COMMON_H
#define NADIR_COMMON_H

#include <stdint.h>

typedef uint8_t nadir_u8_t;
typedef uint16_t nadir_u16_t;
typedef uint32_t nadir_u32_t;
typedef uint64_t nadir_u64_t;

typedef int8_t nadir_i8_t;
typedef int16_t nadir_i16_t;
typedef int32_t nadir_i32_t;
typedef int64_t nadir_i64_t;

constexpr nadir_u8_t NADIR_U8_MAX = 0xFF;
constexpr nadir_u16_t NADIR_U16_MAX = 0xFFFF;
constexpr nadir_u32_t NADIR_U32_MAX = 0xFFFFFFFF;
constexpr nadir_u64_t NADIR_U64_MAX = 0xFFFFFFFFFFFFFFFF;

#endif //NADIR_COMMON_H
