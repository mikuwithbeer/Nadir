#include "nadir/comptime.h"

#include <string.h>

nadir_comptime_kind_t nadir_comptime_kind(const char *name) {
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

    return NADIR_COMPTIME_KIND_NONE;
}

bool nadir_comptime_run(const nadir_comptime_t *comptime,
                        nadir_i128_t *result) {
    switch (comptime->kind) {
        case NADIR_COMPTIME_KIND_ADD: {
            // Check if there are exactly two arguments for addition.
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left + *right;
            break;
        }
        case NADIR_COMPTIME_KIND_SUB: {
            // Check if there are exactly two arguments for subtraction.
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left - *right;
            break;
        }
        case NADIR_COMPTIME_KIND_MUL: {
            // Check if there are exactly two arguments for multiplication.
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left * *right;
            break;
        }
        case NADIR_COMPTIME_KIND_DIV: {
            // Check if there are exactly two arguments for division.
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left / *right;
            break;
        }
        default:
            return false;
    }

    return true;
}
