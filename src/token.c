#include "nadir/token.h"

nadir_token_t nadir_token_new(nadir_token_id_t id,
                              nadir_u64_t line,
                              nadir_u64_t column) {
    nadir_token_t token = {};

    token.id = id;
    token.line = line;
    token.column = column;

    return token;
}

bool nadir_token_append(nadir_token_t *token,
                        char character) {
    if (token->value_length < NADIR_TOKEN_BUFFER_SIZE) {
        token->value[token->value_length++] = character;
        return true;
    }

    return false;
}
