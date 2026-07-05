/**
 * @file comptime.c
 * @brief The comptime implementation.
 */

#include "nadir/comptime.h"

#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_comptime_kind_t nadir_comptime_kind(const char *name) {
    if (strncmp(name, "arg", 4) == 0) {
        return NADIR_COMPTIME_KIND_ARG;
    }

    if (strncmp(name, "cast", 5) == 0) {
        return NADIR_COMPTIME_KIND_CAST;
    }

    if (strncmp(name, "abs", 4) == 0) {
        return NADIR_COMPTIME_KIND_ABS;
    }

    if (strncmp(name, "max", 4) == 0) {
        return NADIR_COMPTIME_KIND_MAX;
    }

    if (strncmp(name, "min", 4) == 0) {
        return NADIR_COMPTIME_KIND_MIN;
    }

    if (strncmp(name, "add", 4) == 0) {
        return NADIR_COMPTIME_KIND_ADD;
    }

    if (strncmp(name, "sub", 4) == 0) {
        return NADIR_COMPTIME_KIND_SUB;
    }

    if (strncmp(name, "mul", 4) == 0) {
        return NADIR_COMPTIME_KIND_MUL;
    }

    if (strncmp(name, "div", 4) == 0) {
        return NADIR_COMPTIME_KIND_DIV;
    }

    if (strncmp(name, "mod", 4) == 0) {
        return NADIR_COMPTIME_KIND_MOD;
    }

    if (strncmp(name, "bit_and", 8) == 0) {
        return NADIR_COMPTIME_KIND_BIT_AND;
    }

    if (strncmp(name, "bit_or", 7) == 0) {
        return NADIR_COMPTIME_KIND_BIT_OR;
    }

    if (strncmp(name, "bit_xor", 8) == 0) {
        return NADIR_COMPTIME_KIND_BIT_XOR;
    }

    if (strncmp(name, "bit_not", 8) == 0) {
        return NADIR_COMPTIME_KIND_BIT_NOT;
    }

    if (strncmp(name, "bit_shl", 8) == 0) {
        return NADIR_COMPTIME_KIND_BIT_SHL;
    }

    if (strncmp(name, "bit_shr", 8) == 0) {
        return NADIR_COMPTIME_KIND_BIT_SHR;
    }

    if (strncmp(name, "bit_swap_16", 12) == 0) {
        return NADIR_COMPTIME_KIND_BIT_SWAP_16;
    }

    if (strncmp(name, "bit_swap_32", 12) == 0) {
        return NADIR_COMPTIME_KIND_BIT_SWAP_32;
    }

    if (strncmp(name, "bit_swap_64", 12) == 0) {
        return NADIR_COMPTIME_KIND_BIT_SWAP_64;
    }

    return NADIR_COMPTIME_KIND_NONE;
}

nadir_compiler_error_t nadir_comptime_run(const nadir_comptime_t *comptime,
                                          const nadir_list_t *context,
                                          nadir_i128_t *result) {
    auto error = (nadir_compiler_error_t){};

    switch (comptime->kind) {
        case NADIR_COMPTIME_KIND_NONE:
            return error; // Unreachable
        case NADIR_COMPTIME_KIND_ARG: {
            if (context == nullptr) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_NULL_CONTEXT;
                return error;
            }

            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *argument = nadir_list_get(comptime->arguments, 0);

            // Argument should be a location in the context list.
            const auto argument_location = (nadir_u64_t) *argument;
            if (argument_location != *argument) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            const nadir_i128_t *context_value = nadir_list_get(context, argument_location);
            if (context_value == nullptr) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            *result = *context_value;
            break;
        }
        case NADIR_COMPTIME_KIND_CAST: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *type = nadir_list_get(comptime->arguments, 1);

            // Convert number to the specified type.
            switch (*type) {
                case NADIR_TYPE_U8:
                    *result = (nadir_u8_t) *value;
                    break;
                case NADIR_TYPE_U16:
                    *result = (nadir_u16_t) *value;
                    break;
                case NADIR_TYPE_U32:
                    *result = (nadir_u32_t) *value;
                    break;
                case NADIR_TYPE_U64:
                    *result = (nadir_u64_t) *value;
                    break;
                case NADIR_TYPE_I8:
                    *result = (nadir_i8_t) *value;
                    break;
                case NADIR_TYPE_I16:
                    *result = (nadir_i16_t) *value;
                    break;
                case NADIR_TYPE_I32:
                    *result = (nadir_i32_t) *value;
                    break;
                case NADIR_TYPE_I64:
                    *result = (nadir_i64_t) *value;
                    break;
                default:
                    error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_ARGUMENT;
                    return error;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_ABS: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            if (*value < 0) {
                *result = -*value;
            } else {
                *result = *value;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_MAX: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            if (*left > *right) {
                *result = *left;
            } else {
                *result = *right;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_MIN: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            if (*left < *right) {
                *result = *left;
            } else {
                *result = *right;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_ADD: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left + *right;
            break;
        }
        case NADIR_COMPTIME_KIND_SUB: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left - *right;
            break;
        }
        case NADIR_COMPTIME_KIND_MUL: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left * *right;
            break;
        }
        case NADIR_COMPTIME_KIND_DIV: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            // Guard against division by zero.
            if (*right == 0) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                return error;
            }

            *result = *left / *right;
            break;
        }
        case NADIR_COMPTIME_KIND_MOD: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            // Guard against division by zero.
            if (*right == 0) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                return error;
            }

            *result = *left % *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_AND: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left & *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_OR: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left | *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_XOR: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            *result = *left ^ *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_NOT: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            *result = ~*value;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_SHL: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            // Guard against shifting by an invalid amount.
            if (*right < 0 || *right > NADIR_I8_MAXIMUM) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND;
                return error;
            }

            *result = *left << *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_SHR: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            // Guard against shifting by an invalid amount.
            if (*right < 0 || *right > NADIR_I8_MAXIMUM) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND;
                return error;
            }

            *result = *left >> *right;
            break;
        }
        case NADIR_COMPTIME_KIND_BIT_SWAP_16: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            if (*value < NADIR_U16_MINIMUM || *value > NADIR_U16_MAXIMUM) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            const auto value_u16 = (nadir_u16_t) *value;
            *result = __builtin_bswap16(value_u16);

            break;
        }
        case NADIR_COMPTIME_KIND_BIT_SWAP_32: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            if (*value < NADIR_U32_MINIMUM || *value > NADIR_U32_MAXIMUM) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            const auto value_u32 = (nadir_u32_t) *value;
            *result = __builtin_bswap32(value_u32);

            break;
        }
        case NADIR_COMPTIME_KIND_BIT_SWAP_64: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            if (*value < NADIR_U64_MINIMUM || *value > NADIR_U64_MAXIMUM) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            const auto value_u64 = (nadir_u64_t) *value;
            *result = __builtin_bswap64(value_u64);

            break;
        }
    }

    return error;
}
