#include "nadir/comptime.h"

#include <string.h>

nadir_comptime_kind_t nadir_comptime_kind(const char *name) {
    if (strncmp(name, "at", 3) == 0) {
        return NADIR_COMPTIME_KIND_AT;
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

    return NADIR_COMPTIME_KIND_NONE;
}

bool nadir_comptime_run(const nadir_comptime_t *comptime,
                        const nadir_list_t *context,
                        nadir_i128_t *result) {
    switch (comptime->kind) {
        case NADIR_COMPTIME_KIND_NONE:
            return false;
        case NADIR_COMPTIME_KIND_AT: {
            if (context == nullptr) {
                return false;
            }

            if (comptime->arguments->length != 1) {
                return false;
            }

            const auto value = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto position = (nadir_u64_t) *value;

            if (position != *value) {
                return false;
            }

            const auto response = (nadir_i128_t *) nadir_list_get(context, position);
            if (response == nullptr) {
                return false;
            }

            *result = *response;
            break;
        }
        case NADIR_COMPTIME_KIND_ADD: {
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left + *right;
            break;
        }
        case NADIR_COMPTIME_KIND_SUB: {
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left - *right;
            break;
        }
        case NADIR_COMPTIME_KIND_MUL: {
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left * *right;
            break;
        }
        case NADIR_COMPTIME_KIND_DIV: {
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left / *right;
            break;
        }
        case NADIR_COMPTIME_KIND_MOD: {
            if (comptime->arguments->length != 2) {
                return false;
            }

            const auto left = (nadir_i128_t *) nadir_list_get(comptime->arguments, 0);
            const auto right = (nadir_i128_t *) nadir_list_get(comptime->arguments, 1);

            *result = *left % *right;
            break;
        }
    }

    return true;
}
