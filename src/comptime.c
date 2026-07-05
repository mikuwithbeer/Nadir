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
    if (strncmp(name, "arg", 4) == 0) return NADIR_COMPTIME_KIND_ARG;
    if (strncmp(name, "cast", 5) == 0) return NADIR_COMPTIME_KIND_CAST;
    if (strncmp(name, "clamp", 6) == 0) return NADIR_COMPTIME_KIND_CLAMP;

    if (strncmp(name, "abs", 4) == 0) return NADIR_COMPTIME_KIND_ABS;
    if (strncmp(name, "neg", 4) == 0) return NADIR_COMPTIME_KIND_NEG;
    if (strncmp(name, "max", 4) == 0) return NADIR_COMPTIME_KIND_MAX;
    if (strncmp(name, "min", 4) == 0) return NADIR_COMPTIME_KIND_MIN;

    if (strncmp(name, "add", 4) == 0) return NADIR_COMPTIME_KIND_ADD;
    if (strncmp(name, "sub", 4) == 0) return NADIR_COMPTIME_KIND_SUB;
    if (strncmp(name, "mul", 4) == 0) return NADIR_COMPTIME_KIND_MUL;
    if (strncmp(name, "div", 4) == 0) return NADIR_COMPTIME_KIND_DIV;
    if (strncmp(name, "mod", 4) == 0) return NADIR_COMPTIME_KIND_MOD;

    if (strncmp(name, "or", 3) == 0) return NADIR_COMPTIME_KIND_OR;
    if (strncmp(name, "and", 4) == 0) return NADIR_COMPTIME_KIND_AND;
    if (strncmp(name, "xor", 4) == 0) return NADIR_COMPTIME_KIND_XOR;
    if (strncmp(name, "shl", 4) == 0) return NADIR_COMPTIME_KIND_SHL;
    if (strncmp(name, "shr", 4) == 0) return NADIR_COMPTIME_KIND_SHR;
    if (strncmp(name, "not", 4) == 0) return NADIR_COMPTIME_KIND_NOT;
    if (strncmp(name, "bswap", 6) == 0) return NADIR_COMPTIME_KIND_BSWAP;

    if (strncmp(name, "if", 3) == 0) return NADIR_COMPTIME_KIND_IF;
    if (strncmp(name, "eq", 3) == 0) return NADIR_COMPTIME_KIND_EQ;
    if (strncmp(name, "lt", 3) == 0) return NADIR_COMPTIME_KIND_LT;
    if (strncmp(name, "gt", 3) == 0) return NADIR_COMPTIME_KIND_GT;
    if (strncmp(name, "le", 3) == 0) return NADIR_COMPTIME_KIND_LE;
    if (strncmp(name, "ge", 3) == 0) return NADIR_COMPTIME_KIND_GE;
    if (strncmp(name, "neq", 4) == 0) return NADIR_COMPTIME_KIND_NEQ;

    if (strncmp(name, "lor", 4) == 0) return NADIR_COMPTIME_KIND_LOR;
    if (strncmp(name, "land", 5) == 0) return NADIR_COMPTIME_KIND_LAND;
    if (strncmp(name, "lnot", 5) == 0) return NADIR_COMPTIME_KIND_LNOT;

    return NADIR_COMPTIME_KIND_NONE;
}

nadir_compiler_error_t nadir_comptime_run(const nadir_comptime_t *comptime,
                                          const nadir_list_t *context,
                                          nadir_i128_t *result) {
    auto error = (nadir_compiler_error_t){};

    switch (comptime->kind) {
        case NADIR_COMPTIME_KIND_NONE:
            break; // Unreachable
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
        case NADIR_COMPTIME_KIND_CLAMP: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            const nadir_i128_t *minimum = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *maximum = nadir_list_get(comptime->arguments, 2);

            if (*value < *minimum) {
                *result = *minimum;
            } else if (*value > *maximum) {
                *result = *maximum;
            } else {
                *result = *value;
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
        case NADIR_COMPTIME_KIND_NEG: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = -*value;
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
            if (comptime->arguments->length < 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            nadir_i128_t value = 0;
            for (nadir_u64_t index = 0; index < comptime->arguments->length; ++index) {
                const nadir_i128_t *argument = nadir_list_get(comptime->arguments, index);
                value += *argument;
            }

            *result = value;
            break;
        }
        case NADIR_COMPTIME_KIND_SUB:
        case NADIR_COMPTIME_KIND_MUL:
        case NADIR_COMPTIME_KIND_DIV:
        case NADIR_COMPTIME_KIND_MOD: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            switch (comptime->kind) {
                case NADIR_COMPTIME_KIND_SUB:
                    *result = *left - *right;
                    break;
                case NADIR_COMPTIME_KIND_MUL:
                    *result = *left * *right;
                    break;
                case NADIR_COMPTIME_KIND_DIV:
                    // Guard against division by zero.
                    if (*right == 0) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                        return error;
                    }

                    *result = *left / *right;
                    break;
                case NADIR_COMPTIME_KIND_MOD:
                    // Guard against division by zero.
                    if (*right == 0) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                        return error;
                    }

                    *result = *left % *right;
                    break;
                default:
                    break; // Unreachable
            }

            break;
        }

        case NADIR_COMPTIME_KIND_OR:
        case NADIR_COMPTIME_KIND_XOR:
        case NADIR_COMPTIME_KIND_AND:
        case NADIR_COMPTIME_KIND_SHL:
        case NADIR_COMPTIME_KIND_SHR: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            switch (comptime->kind) {
                case NADIR_COMPTIME_KIND_OR:
                    *result = *left | *right;
                    break;
                case NADIR_COMPTIME_KIND_XOR:
                    *result = *left ^ *right;
                    break;
                case NADIR_COMPTIME_KIND_AND:
                    *result = *left & *right;
                    break;
                case NADIR_COMPTIME_KIND_SHL:
                    // Guard against shifting by an invalid amount.
                    if (*right < 0 || *right > NADIR_I8_MAXIMUM) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = *left << *right;
                    break;
                case NADIR_COMPTIME_KIND_SHR:
                    // Guard against shifting by an invalid amount.
                    if (*right < 0 || *right > NADIR_I8_MAXIMUM) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = *left >> *right;
                    break;
                default:
                    break; // Unreachable
            }

            break;
        }
        case NADIR_COMPTIME_KIND_NOT: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = ~*value;
            break;
        }
        case NADIR_COMPTIME_KIND_BSWAP: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 1);

            switch (*width) {
                case 16:
                    if (*value < NADIR_U16_MINIMUM || *value > NADIR_U16_MAXIMUM) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = __builtin_bswap16((nadir_u16_t) *value);
                    break;
                case 32:
                    if (*value < NADIR_U32_MINIMUM || *value > NADIR_U32_MAXIMUM) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = __builtin_bswap32((nadir_u32_t) *value);
                    break;
                case 64:
                    if (*value < NADIR_U64_MINIMUM || *value > NADIR_U64_MAXIMUM) {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = __builtin_bswap64((nadir_u64_t) *value);
                    break;
                default:
                    error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_ARGUMENT;
                    return error;
            }

            break;
        }

        case NADIR_COMPTIME_KIND_IF: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
            }

            const nadir_i128_t *condition = nadir_list_get(comptime->arguments, 0);

            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *wrong = nadir_list_get(comptime->arguments, 2);

            if (*condition) {
                *result = *right;
            } else {
                *result = *wrong;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_EQ:
        case NADIR_COMPTIME_KIND_LT:
        case NADIR_COMPTIME_KIND_GT:
        case NADIR_COMPTIME_KIND_LE:
        case NADIR_COMPTIME_KIND_GE:
        case NADIR_COMPTIME_KIND_NEQ: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            switch (comptime->kind) {
                case NADIR_COMPTIME_KIND_EQ:
                    *result = *left == *right;
                    break;
                case NADIR_COMPTIME_KIND_LT:
                    *result = *left < *right;
                    break;
                case NADIR_COMPTIME_KIND_GT:
                    *result = *left > *right;
                    break;
                case NADIR_COMPTIME_KIND_LE:
                    *result = *left <= *right;
                    break;
                case NADIR_COMPTIME_KIND_GE:
                    *result = *left >= *right;
                    break;
                case NADIR_COMPTIME_KIND_NEQ:
                    *result = *left != *right;
                    break;
                default:
                    break; // Unreachable
            }

            break;
        }

        case NADIR_COMPTIME_KIND_LOR:
        case NADIR_COMPTIME_KIND_LAND: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            switch (comptime->kind) {
                case NADIR_COMPTIME_KIND_LOR:
                    *result = *left || *right;
                    break;
                case NADIR_COMPTIME_KIND_LAND:
                    *result = *left && *right;
                    break;
                default:
                    break; // Unreachable
            }

            break;
        }
        case NADIR_COMPTIME_KIND_LNOT: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = !*value;
            break;
        }
    }

    return error;
}
