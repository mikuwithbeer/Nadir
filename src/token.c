/**
 * @file token.c
 * @brief The token implementation.
 */

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_token_t nadir_token_new(const char *path,
                              const nadir_token_kind_t kind,
                              const nadir_u32_t line,
                              const nadir_u32_t column) {
    return (nadir_token_t){
        .path = path,
        .kind = kind,

        .column = column,
        .line = line
    };
}

void nadir_token_start(nadir_token_t *token,
                       const char *source,
                       const nadir_u64_t index) {
    token->string.value = source + index;
    token->string.count = 0;
}

bool nadir_token_increment(nadir_token_t *token) {
    if (token->string.count < NADIR_TOKEN_VALUE_MAXIMUM - 1) {
        ++token->string.count;
        return true;
    }

    return false;
}
