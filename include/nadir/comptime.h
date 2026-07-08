#ifndef NADIR_COMPTIME_H
#define NADIR_COMPTIME_H

/**
 * @file comptime.h
 * @brief The comptime interface.
 *
 * The comptime interface provides functionality for evaluating
 * compile-time procedures in the assembler.
 */

#include "nadir/compiler.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_COMPILER_STRING_MAXIMUM = 0x82;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Comptime kinds for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_COMPTIME_KIND_NONE,

    NADIR_COMPTIME_KIND_ARG,
    NADIR_COMPTIME_KIND_CAST,
    NADIR_COMPTIME_KIND_CLAMP,
    NADIR_COMPTIME_KIND_ASSERT,

    NADIR_COMPTIME_KIND_ABS,
    NADIR_COMPTIME_KIND_NEG,
    NADIR_COMPTIME_KIND_MAX,
    NADIR_COMPTIME_KIND_MIN,

    NADIR_COMPTIME_KIND_ADD,
    NADIR_COMPTIME_KIND_SUB,
    NADIR_COMPTIME_KIND_MUL,
    NADIR_COMPTIME_KIND_DIV,
    NADIR_COMPTIME_KIND_MOD,

    NADIR_COMPTIME_KIND_OR,
    NADIR_COMPTIME_KIND_XOR,
    NADIR_COMPTIME_KIND_AND,
    NADIR_COMPTIME_KIND_SHL,
    NADIR_COMPTIME_KIND_SHR,
    NADIR_COMPTIME_KIND_NOT,
    NADIR_COMPTIME_KIND_BSWAP,

    NADIR_COMPTIME_KIND_IF,
    NADIR_COMPTIME_KIND_EQ,
    NADIR_COMPTIME_KIND_LT,
    NADIR_COMPTIME_KIND_GT,
    NADIR_COMPTIME_KIND_LE,
    NADIR_COMPTIME_KIND_GE,
    NADIR_COMPTIME_KIND_NEQ,

    NADIR_COMPTIME_KIND_LOR,
    NADIR_COMPTIME_KIND_LAND,
    NADIR_COMPTIME_KIND_LNOT,
} nadir_comptime_kind_t;

/**
 * @brief Comptime structure for the assembler.
 */
typedef struct {
    nadir_comptime_kind_t kind;
    nadir_list_t *arguments; // List of `nadir_i128_t`
} nadir_comptime_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Determines the comptime kind based on the given name.
 */
[[nodiscard]] nadir_comptime_kind_t nadir_comptime_kind(const char *name,
                                                        nadir_u64_t length);

/**
 * @brief Evaluates the given comptime expression with the provided context.
 */
[[nodiscard]] nadir_compiler_error_t nadir_comptime_run(const nadir_comptime_t *comptime,
                                                        const nadir_list_t *context,
                                                        nadir_i128_t *result);

#endif //NADIR_COMPTIME_H
