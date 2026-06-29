#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_token_t nadir_token_new(const nadir_token_kind_t kind,
                              const nadir_u64_t line,
                              const nadir_u64_t column) {
    nadir_token_t token = {};

    token.kind = kind;
    token.line = line;
    token.column = column;

    return token;
}

bool nadir_token_append(nadir_token_t *token,
                        const char character) {
    if (token->value_length < NADIR_TOKEN_BUFFER_SIZE) {
        token->value[token->value_length++] = character;
        return true;
    }

    return false;
}
