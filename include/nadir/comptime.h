#ifndef NADIR_COMPTIME_H
#define NADIR_COMPTIME_H

#include "nadir/common.h"

/**
 * @brief Comptime kinds for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_COMPTIME_KIND_NONE,
    NADIR_COMPTIME_KIND_AT,
    NADIR_COMPTIME_KIND_CAST,

    NADIR_COMPTIME_KIND_ABS,
    NADIR_COMPTIME_KIND_MAX,
    NADIR_COMPTIME_KIND_MIN,

    NADIR_COMPTIME_KIND_ADD,
    NADIR_COMPTIME_KIND_SUB,
    NADIR_COMPTIME_KIND_MUL,
    NADIR_COMPTIME_KIND_DIV,
    NADIR_COMPTIME_KIND_MOD,

    NADIR_COMPTIME_KIND_BIT_AND,
    NADIR_COMPTIME_KIND_BIT_OR,
    NADIR_COMPTIME_KIND_BIT_XOR,
    NADIR_COMPTIME_KIND_BIT_NOT,
    NADIR_COMPTIME_KIND_BIT_SHL,
    NADIR_COMPTIME_KIND_BIT_SHR,
} nadir_comptime_kind_t;

/**
 * @brief Comptime structure for the assembler.
 */
typedef struct {
    nadir_comptime_kind_t kind;
    nadir_list_t *arguments;
} nadir_comptime_t;

/**
 * @brief Returns the comptime kind for the given name.
 */
[[nodiscard]] nadir_comptime_kind_t nadir_comptime_kind(const char *name);

/**
 * @brief Runs the comptime operation and writes the result.
 */
[[nodiscard]] bool nadir_comptime_run(const nadir_comptime_t *comptime,
                                      const nadir_list_t *context,
                                      nadir_i128_t *result);

#endif //NADIR_COMPTIME_H
