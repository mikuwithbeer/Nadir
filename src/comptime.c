/**
 * @file comptime.c
 * @brief The comptime implementation.
 */

#include "nadir/comptime.h"

#include <string.h>
#include <stddef.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_comptime_kind_t nadir_comptime_kind(const char *name,
                                          const nadir_u64_t length) {
    switch (length) {
        case 3:
            if (memcmp(name, "@if", length) == 0) return NADIR_COMPTIME_KIND_IF;
            if (memcmp(name, "@eq", length) == 0) return NADIR_COMPTIME_KIND_EQ;
            if (memcmp(name, "@lt", length) == 0) return NADIR_COMPTIME_KIND_LT;
            if (memcmp(name, "@gt", length) == 0) return NADIR_COMPTIME_KIND_GT;
            if (memcmp(name, "@le", length) == 0) return NADIR_COMPTIME_KIND_LE;
            if (memcmp(name, "@ge", length) == 0) return NADIR_COMPTIME_KIND_GE;
            if (memcmp(name, "@or", length) == 0) return NADIR_COMPTIME_KIND_OR;
            return NADIR_COMPTIME_KIND_NONE;
        case 4:
            if (memcmp(name, "@arg", length) == 0) return NADIR_COMPTIME_KIND_ARG;
            if (memcmp(name, "@abs", length) == 0) return NADIR_COMPTIME_KIND_ABS;
            if (memcmp(name, "@neg", length) == 0) return NADIR_COMPTIME_KIND_NEG;
            if (memcmp(name, "@add", length) == 0) return NADIR_COMPTIME_KIND_ADD;
            if (memcmp(name, "@sub", length) == 0) return NADIR_COMPTIME_KIND_SUB;
            if (memcmp(name, "@mul", length) == 0) return NADIR_COMPTIME_KIND_MUL;
            if (memcmp(name, "@div", length) == 0) return NADIR_COMPTIME_KIND_DIV;
            if (memcmp(name, "@mod", length) == 0) return NADIR_COMPTIME_KIND_MOD;
            if (memcmp(name, "@and", length) == 0) return NADIR_COMPTIME_KIND_AND;
            if (memcmp(name, "@xor", length) == 0) return NADIR_COMPTIME_KIND_XOR;
            if (memcmp(name, "@shl", length) == 0) return NADIR_COMPTIME_KIND_SHL;
            if (memcmp(name, "@shr", length) == 0) return NADIR_COMPTIME_KIND_SHR;
            if (memcmp(name, "@not", length) == 0) return NADIR_COMPTIME_KIND_NOT;
            if (memcmp(name, "@neq", length) == 0) return NADIR_COMPTIME_KIND_NEQ;
            if (memcmp(name, "@lor", length) == 0) return NADIR_COMPTIME_KIND_LOR;
            if (memcmp(name, "@max", length) == 0) return NADIR_COMPTIME_KIND_MAX;
            if (memcmp(name, "@min", length) == 0) return NADIR_COMPTIME_KIND_MIN;
            return NADIR_COMPTIME_KIND_NONE;
        case 5:
            if (memcmp(name, "@cast", length) == 0) return NADIR_COMPTIME_KIND_CAST;
            if (memcmp(name, "@here", length) == 0) return NADIR_COMPTIME_KIND_HERE;
            if (memcmp(name, "@mask", length) == 0) return NADIR_COMPTIME_KIND_MASK;
            if (memcmp(name, "@rotl", length) == 0) return NADIR_COMPTIME_KIND_ROTL;
            if (memcmp(name, "@rotr", length) == 0) return NADIR_COMPTIME_KIND_ROTR;
            if (memcmp(name, "@land", length) == 0) return NADIR_COMPTIME_KIND_LAND;
            if (memcmp(name, "@lnot", length) == 0) return NADIR_COMPTIME_KIND_LNOT;
            return NADIR_COMPTIME_KIND_NONE;
        case 6:
            if (memcmp(name, "@clamp", length) == 0) return NADIR_COMPTIME_KIND_CLAMP;
            if (memcmp(name, "@bswap", length) == 0) return NADIR_COMPTIME_KIND_BSWAP;
            return NADIR_COMPTIME_KIND_NONE;
        case 7:
            if (memcmp(name, "@insert", length) == 0) return NADIR_COMPTIME_KIND_INSERT;
            if (memcmp(name, "@assert", length) == 0) return NADIR_COMPTIME_KIND_ASSERT;
            if (memcmp(name, "@popcnt", length) == 0) return NADIR_COMPTIME_KIND_POPCNT;
            return NADIR_COMPTIME_KIND_NONE;
        case 8:
            if (memcmp(name, "@extract", length) == 0) return NADIR_COMPTIME_KIND_EXTRACT;
            if (memcmp(name, "@between", length) == 0) return NADIR_COMPTIME_KIND_BETWEEN;
            return NADIR_COMPTIME_KIND_NONE;
        default:
            return NADIR_COMPTIME_KIND_NONE;
    }
}

nadir_compiler_error_t nadir_comptime_run(const nadir_comptime_t *comptime,
                                          const nadir_compiler_t *compiler,
                                          const nadir_context_t *context,
                                          nadir_i128_t *result) {
    auto error = (nadir_compiler_error_t){};

    switch (comptime->kind) {
        case NADIR_COMPTIME_KIND_ARG: {
            if (context == nullptr) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_NULL_CONTEXT;
                return error;
            }

            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *argument = nadir_list_get(comptime->arguments, 0);

            // Argument should be a location in the context list.
            const auto argument_location = (nadir_u64_t) *argument;
            if (argument_location != *argument) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            if (argument_location >= context->length) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                return error;
            }

            *result = context->value[argument_location];
            break;
        }
        case NADIR_COMPTIME_KIND_CAST: {
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 3) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 0) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            *result = (nadir_i128_t) compiler->binary_calculation;
            break;
        }

        case NADIR_COMPTIME_KIND_ABS: {
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = -*value;
            break;
        }
        case NADIR_COMPTIME_KIND_ADD: {
            if (comptime->arguments->length < 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
                    if (*right == 0) [[clang::unlikely]] {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                        return error;
                    }

                    *result = *left / *right;
                    break;
                case NADIR_COMPTIME_KIND_MOD:
                    // Guard against division by zero.
                    if (*right == 0) [[clang::unlikely]] {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_DIVISION_BY_ZERO;
                        return error;
                    }

                    *result = *left % *right;
                    break;
                default:
                    unreachable();
            }

            break;
        }

        case NADIR_COMPTIME_KIND_OR:
        case NADIR_COMPTIME_KIND_XOR:
        case NADIR_COMPTIME_KIND_AND: {
            if (comptime->arguments->length < 2) [[clang::unlikely]] {
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
                        unreachable();
                }
            }

            *result = value;
            break;
        }
        case NADIR_COMPTIME_KIND_SHL:
        case NADIR_COMPTIME_KIND_SHR: {
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *left = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *right = nadir_list_get(comptime->arguments, 1);

            // Guard against shifting by an invalid amount.
            if (*right < 0 || *right > NADIR_I8_MAXIMUM) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = ~*value;
            break;
        }
        case NADIR_COMPTIME_KIND_POPCNT: {
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_u128_t *value = nadir_list_get(comptime->arguments, 0);

            const auto lower = (nadir_u64_t) *value;
            const auto upper = (nadir_u64_t) (*value >> 64);

            *result = __builtin_popcountll(lower) + __builtin_popcountll(upper);
            break;
        }
        case NADIR_COMPTIME_KIND_BSWAP: {
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 1);

            switch (*width) {
                case 16:
                    if (*value < NADIR_U16_MINIMUM || *value > NADIR_U16_MAXIMUM) [[clang::unlikely]] {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = __builtin_bswap16((nadir_u16_t) *value);
                    break;
                case 32:
                    if (*value < NADIR_U32_MINIMUM || *value > NADIR_U32_MAXIMUM) [[clang::unlikely]] {
                        error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_OUT_OF_BOUND;
                        return error;
                    }

                    *result = __builtin_bswap32((nadir_u32_t) *value);
                    break;
                case 64:
                    if (*value < NADIR_U64_MINIMUM || *value > NADIR_U64_MAXIMUM) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 0);

            // Guard against invalid bit width.
            if (*width < 0 || *width > 128) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 4) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *base = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *offset = nadir_list_get(comptime->arguments, 2);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 3);

            // Guard against invalid bit width and offset.
            if (*offset < 0
                || *offset > 127
                || *width < 0
                || *width > 128
                || *offset + *width > 128) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 3) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *offset = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 2);

            // Guard against invalid bit width and offset.
            if (*offset < 0
                || *offset > 127
                || *width < 0
                || *width > 128
                || *offset + *width > 128) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            nadir_i128_t mask;
            if (*width == 128) [[clang::unlikely]] {
                mask = ~(nadir_i128_t) 0; // All bits set
            } else {
                mask = ((nadir_i128_t) 1 << *width) - 1;
            }

            *result = *value >> *offset & mask;
            break;
        }
        case NADIR_COMPTIME_KIND_ROTL:
        case NADIR_COMPTIME_KIND_ROTR: {
            if (comptime->arguments->length != 3) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                return error;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *shift = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *width = nadir_list_get(comptime->arguments, 2);

            // Guard against invalid bit width.
            if (*width < 1 || *width > 128) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_INVALID_BIT_WIDTH;
                return error;
            }

            // Guard against negative shift values.
            if (*shift < 0) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 3) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
                    unreachable();
            }

            break;
        }
        case NADIR_COMPTIME_KIND_LOR:
        case NADIR_COMPTIME_KIND_LAND: {
            if (comptime->arguments->length != 2) [[clang::unlikely]] {
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
            if (comptime->arguments->length != 1) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                break;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);

            *result = !*value;
            break;
        }
        case NADIR_COMPTIME_KIND_BETWEEN: {
            if (comptime->arguments->length != 3) [[clang::unlikely]] {
                error.kind = NADIR_COMPILER_ERROR_KIND_COMPTIME_ARGUMENT_COUNT_MISMATCH;
                break;
            }

            const nadir_i128_t *value = nadir_list_get(comptime->arguments, 0);
            const nadir_i128_t *minimum = nadir_list_get(comptime->arguments, 1);
            const nadir_i128_t *maximum = nadir_list_get(comptime->arguments, 2);

            *result = *value >= *minimum && *value <= *maximum;
            break;
        }
        case NADIR_COMPTIME_KIND_NONE:
            unreachable();
    }

    return error;
}
