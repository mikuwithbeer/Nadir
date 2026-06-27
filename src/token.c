#include "nadir/token.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_token_t nadir_token_new(const nadir_token_id_t id,
                              const nadir_u64_t line,
                              const nadir_u64_t column) {
    nadir_token_t token = {};

    token.id = id;
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

bool nadir_token_list_append(nadir_token_list_t *token_list,
                             const nadir_token_t token) {
    if (token_list->token_count >= token_list->token_capacity) {
        const auto new_capacity = token_list->token_capacity << 1;
        const auto new_tokens = realloc(token_list->tokens, new_capacity * sizeof(nadir_token_t));
        if (new_tokens == nullptr) {
            return false;
        }

        token_list->tokens = new_tokens;
        token_list->token_capacity = new_capacity;
    }

    token_list->tokens[token_list->token_count++] = token;
    return true;
}

nadir_token_list_t *nadir_token_list_new(const nadir_u64_t initial_capacity) {
    nadir_token_list_t *token_list = malloc(sizeof(nadir_token_list_t));
    if (token_list == nullptr) {
        return nullptr;
    }

    const auto tokens = malloc(initial_capacity * sizeof(nadir_token_t));
    if (tokens == nullptr) {
        free(token_list);
        return nullptr;
    }

    token_list->tokens = tokens;
    token_list->token_capacity = initial_capacity;
    token_list->token_count = 0;

    return token_list;
}

void nadir_token_list_free(nadir_token_list_t *token_list) {
    if (token_list == nullptr) {
        return;
    }

    free(token_list->tokens);
    free(token_list);
}
