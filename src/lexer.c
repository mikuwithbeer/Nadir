#include "nadir/lexer.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_lexer_t *nadir_lexer_new(const char *source,
                               const nadir_u64_t source_length) {
    nadir_lexer_t *lexer = malloc(sizeof(nadir_lexer_t));
    if (lexer == nullptr) {
        return nullptr;
    }

    const auto token_list = nadir_token_list_new(NADIR_LEXER_DEFAULT_TOKEN_LIST_CAPACITY);
    if (token_list == nullptr) {
        free(lexer);
        return nullptr;
    }

    lexer->token_list = token_list;

    lexer->source = source;
    lexer->source_length = source_length;

    lexer->line = 1;
    lexer->column = 1;
    lexer->state = NADIR_LEXER_STATE_DEFAULT;

    return lexer;
}

nadir_lexer_error_t nadir_lexer_run(nadir_lexer_t *lexer) {
    nadir_lexer_error_t error = {
        .id = NADIR_LEXER_ERROR_ID_NONE,
    };

    nadir_u64_t source_index = 0;

    while (source_index < lexer->source_length) {
        const char character = lexer->source[source_index];

        switch (lexer->state) {
            case NADIR_LEXER_STATE_DEFAULT:
                lexer->temporary_token = (nadir_token_t){}; // Initialize to zeroed state
                lexer->temporary_token.line = lexer->line;
                lexer->temporary_token.column = lexer->column;

                if (character == ';') {
                    lexer->state = NADIR_LEXER_STATE_COMMENT;
                } else if (character >= '0' && character <= '9') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_NUMBER;
                    lexer->state = NADIR_LEXER_STATE_NUMBER;
                    continue;
                } else if ((character >= 'a' && character <= 'z') ||
                           (character >= 'A' && character <= 'Z') ||
                           character == '_') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_IDENT;
                    lexer->state = NADIR_LEXER_STATE_IDENT;
                    continue;
                } else if (character == '{') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_BRACE;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                } else if (character == '}') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_BRACE;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                } else if (character == '(') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_PAREN;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                } else if (character == ')') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_PAREN;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                } else if (character == '=') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_EQUAL;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                } else if (character == ',') {
                    lexer->temporary_token.id = NADIR_TOKEN_ID_COMMA;
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                }

                break;
            case NADIR_LEXER_STATE_COMMENT:
                if (character == '\n') {
                    lexer->state = NADIR_LEXER_STATE_DEFAULT;
                }

                break;
            case NADIR_LEXER_STATE_NUMBER:
                if (character == ' ' || character == '\n' || character == '\t') {
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                    lexer->state = NADIR_LEXER_STATE_DEFAULT;
                    continue;
                }

                nadir_token_append(&lexer->temporary_token, character);

                if (!(character >= '0' && character <= '9')) {
                    error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
                    error.line = lexer->line;
                    error.column = lexer->column;
                    error.specific.unexpected_character = character;

                    return error;
                }

                break;
            case NADIR_LEXER_STATE_IDENT:
                if (character == ' ' || character == '\n' || character == '\t') {
                    nadir_token_list_append(lexer->token_list, lexer->temporary_token);
                    lexer->state = NADIR_LEXER_STATE_DEFAULT;
                    continue;
                }

                nadir_token_append(&lexer->temporary_token, character);

                if (!((character >= 'a' && character <= 'z') ||
                      (character >= 'A' && character <= 'Z') ||
                      character == '_')) {
                    error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
                    error.line = lexer->line;
                    error.column = lexer->column;
                    error.specific.unexpected_character = character;

                    return error;
                }

                break;
        }

        if (character == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }

        source_index++;
    }

    if (lexer->state == NADIR_LEXER_STATE_NUMBER || lexer->state == NADIR_LEXER_STATE_IDENT) {
        nadir_token_list_append(lexer->token_list, lexer->temporary_token);
    }

    lexer->temporary_token = (nadir_token_t){};
    lexer->temporary_token.id = NADIR_TOKEN_ID_EOF;
    lexer->temporary_token.line = lexer->line;
    lexer->temporary_token.column = lexer->column;

    nadir_token_list_append(lexer->token_list, lexer->temporary_token);

    return error;
}

void nadir_lexer_free(nadir_lexer_t *lexer) {
    if (lexer == nullptr) {
        return;
    }

    nadir_token_list_free(lexer->token_list);
    free(lexer);
}
