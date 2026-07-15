/**
 * @file comptime.c
 * @brief The comptime implementation.
 */

#include "nadir/comptime.h"

#include <string.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_comptime_kind_t nadir_comptime_kind(const char *name,
                                          const nadir_u64_t length) {
#define NADIR_COMPTIME_MATCH(literal) \
    (length == sizeof(literal) && memcmp(name, "@"literal, length) == 0)

    if (NADIR_COMPTIME_MATCH("arg")) return NADIR_COMPTIME_KIND_ARG;
    if (NADIR_COMPTIME_MATCH("cast")) return NADIR_COMPTIME_KIND_CAST;
    if (NADIR_COMPTIME_MATCH("clamp")) return NADIR_COMPTIME_KIND_CLAMP;
    if (NADIR_COMPTIME_MATCH("max")) return NADIR_COMPTIME_KIND_MAX;
    if (NADIR_COMPTIME_MATCH("min")) return NADIR_COMPTIME_KIND_MIN;
    if (NADIR_COMPTIME_MATCH("here")) return NADIR_COMPTIME_KIND_HERE;

    if (NADIR_COMPTIME_MATCH("abs")) return NADIR_COMPTIME_KIND_ABS;
    if (NADIR_COMPTIME_MATCH("neg")) return NADIR_COMPTIME_KIND_NEG;
    if (NADIR_COMPTIME_MATCH("add")) return NADIR_COMPTIME_KIND_ADD;
    if (NADIR_COMPTIME_MATCH("sub")) return NADIR_COMPTIME_KIND_SUB;
    if (NADIR_COMPTIME_MATCH("mul")) return NADIR_COMPTIME_KIND_MUL;
    if (NADIR_COMPTIME_MATCH("div")) return NADIR_COMPTIME_KIND_DIV;
    if (NADIR_COMPTIME_MATCH("mod")) return NADIR_COMPTIME_KIND_MOD;

    if (NADIR_COMPTIME_MATCH("or")) return NADIR_COMPTIME_KIND_OR;
    if (NADIR_COMPTIME_MATCH("and")) return NADIR_COMPTIME_KIND_AND;
    if (NADIR_COMPTIME_MATCH("xor")) return NADIR_COMPTIME_KIND_XOR;
    if (NADIR_COMPTIME_MATCH("shl")) return NADIR_COMPTIME_KIND_SHL;
    if (NADIR_COMPTIME_MATCH("shr")) return NADIR_COMPTIME_KIND_SHR;
    if (NADIR_COMPTIME_MATCH("not")) return NADIR_COMPTIME_KIND_NOT;
    if (NADIR_COMPTIME_MATCH("bswap")) return NADIR_COMPTIME_KIND_BSWAP;
    if (NADIR_COMPTIME_MATCH("mask")) return NADIR_COMPTIME_KIND_MASK;
    if (NADIR_COMPTIME_MATCH("insert")) return NADIR_COMPTIME_KIND_INSERT;
    if (NADIR_COMPTIME_MATCH("extract")) return NADIR_COMPTIME_KIND_EXTRACT;
    if (NADIR_COMPTIME_MATCH("rotl")) return NADIR_COMPTIME_KIND_ROTL;
    if (NADIR_COMPTIME_MATCH("rotr")) return NADIR_COMPTIME_KIND_ROTR;

    if (NADIR_COMPTIME_MATCH("assert")) return NADIR_COMPTIME_KIND_ASSERT;
    if (NADIR_COMPTIME_MATCH("if")) return NADIR_COMPTIME_KIND_IF;
    if (NADIR_COMPTIME_MATCH("eq")) return NADIR_COMPTIME_KIND_EQ;
    if (NADIR_COMPTIME_MATCH("lt")) return NADIR_COMPTIME_KIND_LT;
    if (NADIR_COMPTIME_MATCH("gt")) return NADIR_COMPTIME_KIND_GT;
    if (NADIR_COMPTIME_MATCH("le")) return NADIR_COMPTIME_KIND_LE;
    if (NADIR_COMPTIME_MATCH("ge")) return NADIR_COMPTIME_KIND_GE;
    if (NADIR_COMPTIME_MATCH("neq")) return NADIR_COMPTIME_KIND_NEQ;
    if (NADIR_COMPTIME_MATCH("lor")) return NADIR_COMPTIME_KIND_LOR;
    if (NADIR_COMPTIME_MATCH("land")) return NADIR_COMPTIME_KIND_LAND;
    if (NADIR_COMPTIME_MATCH("lnot")) return NADIR_COMPTIME_KIND_LNOT;
    if (NADIR_COMPTIME_MATCH("between")) return NADIR_COMPTIME_KIND_BETWEEN;

#undef NADIR_COMPTIME_MATCH

    return NADIR_COMPTIME_KIND_NONE;
}

nadir_compiler_error_t nadir_comptime_run(const nadir_comptime_t *comptime,
                                          const nadir_compiler_t *compiler,
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
                case 0: // U8
                    *result = (nadir_u8_t) *value;
                    break;
                case 1: // U16
                    *result = (nadir_u16_t) *value;
                    break;
                case 2: // U32
                    *result = (nadir_u32_t) *value;
                    break;
                case 3: // U64
                    *result = (nadir_u64_t) *value;
                    break;
                case 4: // I8
                    *result = (nadir_i8_t) *value;
                    break;
                case 5: // I16
                    *result = (nadir_i16_t) *value;
                    break;
                case 6: // I32
                    *result = (nadir_i32_t) *value;
                    break;
                case 7: // I64
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
        case NADIR_COMPTIME_KIND_HERE: {
            if (comptime->arguments->length != 0) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            *result = (nadir_i128_t) compiler->binary_calculation;
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
        case NADIR_COMPTIME_KIND_ADD: {
            if (comptime->arguments->length < 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *first = nadir_list_get(comptime->arguments, 0);
            nadir_i128_t value = *first;

            for (nadir_u64_t index = 1; index < comptime->arguments->length; ++index) {
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
        case NADIR_COMPTIME_KIND_AND: {
            if (comptime->arguments->length < 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *first = nadir_list_get(comptime->arguments, 0);
            nadir_i128_t value = *first;

            for (nadir_u64_t index = 1; index < comptime->arguments->length; ++index) {
                const nadir_i128_t *argument = nadir_list_get(comptime->arguments, index);
                switch (comptime->kind) {
                    case NADIR_COMPTIME_KIND_OR:
                        value |= *argument;
                        break;
                    case NADIR_COMPTIME_KIND_XOR:
                        value ^= *argument;
                        break;
                    case NADIR_COMPTIME_KIND_AND:
                        value &= *argument;
                        break;
                    default:
                        break; // Unreachable
                }
            }

            *result = value;
            break;
        }
        case NADIR_COMPTIME_KIND_SHL:
        case NADIR_COMPTIME_KIND_SHR: {
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

            if (comptime->kind == NADIR_COMPTIME_KIND_SHL) {
                *result = *left << *right;
            } else {
                *result = *left >> *right;
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
                    error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                    return error;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_MASK: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 0);

            // Guard against invalid bit width.
            if (*width < 0 || *width > 128) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            if (*width == 128) {
                *result = ~(nadir_i128_t) 0; // All bits set
            } else {
                *result = ((nadir_i128_t) 1 << *width) - 1;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_INSERT: {
            if (comptime->arguments->length != 4) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *base = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *offset = nadir_list_get(comptime->arguments, 2);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 3);

            // Guard against invalid bit width and offset.
            if (*offset < 0 || *offset > 127 || *width < 0 || *width > 128 || *offset + *width > 128) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            nadir_i128_t mask;
            if (*width == 128) {
                mask = ~(nadir_i128_t) 0; // All bits set
            } else {
                mask = ((nadir_i128_t) 1 << *width) - 1;
            }

            *result = (*base & ~(mask << *offset)) | ((*value & mask) << *offset);
            break;
        }
        case NADIR_COMPTIME_KIND_EXTRACT: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *offset = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 2);

            // Guard against invalid bit width and offset.
            if (*offset < 0 || *offset > 127 || *width < 0 || *width > 128 || *offset + *width > 128) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            nadir_i128_t mask;
            if (*width == 128) {
                mask = ~(nadir_i128_t) 0; // All bits set
            } else {
                mask = ((nadir_i128_t) 1 << *width) - 1;
            }

            *result = *value >> *offset & mask;
            break;
        }
        case NADIR_COMPTIME_KIND_ROTL:
        case NADIR_COMPTIME_KIND_ROTR: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *shift = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 2);

            // Guard against invalid bit width.
            if (*width < 1 || *width > 128) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            // Guard against negative shift values.
            if (*shift < 0) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_SHIFT_OUT_OF_BOUND;
                return error;
            }

            const auto bits = (nadir_u32_t) *width;
            const auto rotation = (nadir_u32_t) (*shift % bits);

            const auto mask = bits == 128 ? NADIR_U128_MAXIMUM : ((nadir_u128_t) 1 << bits) - 1;
            const auto normalized = (nadir_u128_t) *value & mask;

            // Early out for no-op rotations.
            if (rotation == 0) {
                *result = (nadir_i128_t) normalized;
                break;
            }

            if (comptime->kind == NADIR_COMPTIME_KIND_ROTL) {
                *result = (nadir_i128_t) ((normalized << rotation | normalized >> (bits - rotation)) & mask);
            } else {
                *result = (nadir_i128_t) ((normalized >> rotation | normalized << (bits - rotation)) & mask);
            }

            break;
        }

        case NADIR_COMPTIME_KIND_ASSERT: {
            if (comptime->arguments->length != 2) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *condition = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 1);

            if (!*condition) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ASSERTION_FAILED;
                return error;
            }

            *result = *value;
            break;
        }
        case NADIR_COMPTIME_KIND_IF: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                break;
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
                break;
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
                break;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            if (comptime->kind == NADIR_COMPTIME_KIND_LOR) {
                *result = *left || *right;
            } else {
                *result = *left && *right;
            }

            break;
        }
        case NADIR_COMPTIME_KIND_LNOT: {
            if (comptime->arguments->length != 1) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                break;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = !*value;
            break;
        }
        case NADIR_COMPTIME_KIND_BETWEEN: {
            if (comptime->arguments->length != 3) {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                break;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *minimum = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *maximum = nadir_list_get(comptime->arguments, 2);

            *result = *value >= *minimum && *value <= *maximum;
            break;
        }
    }

    return error;
}
