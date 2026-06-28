#include "nadir/lexer.h"

#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_lexer_error_t nadir_lexer_collect_default(nadir_lexer_t *lexer,
                                                       char character,
                                                       bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_comment(nadir_lexer_t *lexer,
                                                       char character);

static nadir_lexer_error_t nadir_lexer_collect_number(nadir_lexer_t *lexer,
                                                      char character,
                                                      bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_ident(nadir_lexer_t *lexer,
                                                     char character,
                                                     bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer);

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

nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer) {
    auto error = (nadir_lexer_error_t){};

    nadir_u64_t source_index = 0;
    while (source_index < lexer->source_length) {
        const char character = lexer->source[source_index];
        bool recollect = false;

        switch (lexer->state) {
            case NADIR_LEXER_STATE_DEFAULT:
                error = nadir_lexer_collect_default(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_COMMENT:
                error = nadir_lexer_collect_comment(lexer, character);
                break;
            case NADIR_LEXER_STATE_NUMBER:
                error = nadir_lexer_collect_number(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_IDENT:
                error = nadir_lexer_collect_ident(lexer, character, &recollect);
                break;
        }

        // If an error occurred, return it immediately.
        if (error.id != NADIR_LEXER_ERROR_ID_NONE) {
            return error;
        }

        // Do not advance the source index.
        if (recollect) {
            continue;
        }

        if (character == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }

        source_index++;
    }

    error = nadir_lexer_collect_eof(lexer);
    return error;
}

void nadir_lexer_free(nadir_lexer_t *lexer) {
    if (lexer == nullptr) {
        return;
    }

    nadir_token_list_free(lexer->token_list);
    free(lexer);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_lexer_error_t nadir_lexer_collect_default(nadir_lexer_t *lexer,
                                                       const char character,
                                                       bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    // Ignore whitespace characters.
    if (nadir_token_value_whitespace(character)) {
        return error;
    }

    // Initialize the temporary token.
    lexer->temporary_token = nadir_token_new(NADIR_TOKEN_ID_EOF, lexer->line, lexer->column);

    // Check for comment.
    if (character == NADIR_TOKEN_VALUE_COMMENT) {
        lexer->state = NADIR_LEXER_STATE_COMMENT;
        return error;
    }

    // Check for base 10 number.
    if (nadir_token_value_digit(character) || character == '-' || character == '+') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_NUMBER;
        lexer->state = NADIR_LEXER_STATE_NUMBER;

        *recollect = true;
        return error;
    }

    // Check for identifier.
    if (nadir_token_value_alpha(character)) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_IDENT;
        lexer->state = NADIR_LEXER_STATE_IDENT;

        *recollect = true;
        return error;
    }

    // Check for single-character tokens.
    if (character == NADIR_TOKEN_VALUE_LEFT_BRACE) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_BRACE;
    } else if (character == NADIR_TOKEN_VALUE_RIGHT_BRACE) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_BRACE;
    } else if (character == NADIR_TOKEN_VALUE_LEFT_PAREN) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_PAREN;
    } else if (character == NADIR_TOKEN_VALUE_RIGHT_PAREN) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_PAREN;
    } else if (character == NADIR_TOKEN_VALUE_EQUAL) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_EQUAL;
    } else if (character == NADIR_TOKEN_VALUE_COMMA) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_COMMA;
    } else if (character == NADIR_TOKEN_VALUE_DOT) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_DOT;
    } else if (character == NADIR_TOKEN_VALUE_SEMICOLON) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_SEMICOLON;
    } else {
        error.id = NADIR_LEXER_ERROR_ID_UNKNOWN_CHARACTER;
        error.specific.character = character;

        return error;
    }

    if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
        error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_comment(nadir_lexer_t *lexer,
                                                       const char character) {
    // Ignore all characters until a newline is found.
    if (character == '\n') {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;
    }

    return (nadir_lexer_error_t){};
}

static nadir_lexer_error_t nadir_lexer_collect_number(nadir_lexer_t *lexer,
                                                      const char character,
                                                      bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the number.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // 40 digits is the maximum length for a 128-bit signed integer in base 10 representation.
        if (lexer->temporary_token.value_length > 40) {
            error.id = NADIR_LEXER_ERROR_ID_NUMBER_TOO_LONG;
            return error;
        }

        // Convert the number string to a 128-bit signed integer.
        if (!nadir_common_string_to_i128(lexer->temporary_token.value, &lexer->temporary_token.specific.number)) {
            error.id = NADIR_LEXER_ERROR_ID_INVALID_NUMBER;
            return error;
        }

        if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
            error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Ignore underscores in numbers.
    if (character == '_') {
        return error;
    }

    // Check for valid characters.
    if (!nadir_token_value_digit(character) &&
        (lexer->temporary_token.value_length != 0 || (character != '-' && character != '+'))) {
        error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
        error.specific.character = character;

        return error;
    }

    if (!nadir_token_append(&lexer->temporary_token, character)) {
        error.id = NADIR_LEXER_ERROR_ID_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_ident(nadir_lexer_t *lexer,
                                                     const char character,
                                                     bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the number.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check for keywords.
        if (strncmp(lexer->temporary_token.value, "const", 6) == 0) {
            lexer->temporary_token.id = NADIR_TOKEN_ID_CONST;
        } else if (strncmp(lexer->temporary_token.value, "instruction", 12) == 0) {
            lexer->temporary_token.id = NADIR_TOKEN_ID_INSTRUCTION;
        } else if (strncmp(lexer->temporary_token.value, "binary", 7) == 0) {
            lexer->temporary_token.id = NADIR_TOKEN_ID_BINARY;
        }

        if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
            error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!nadir_token_value_alpha(character) && !nadir_token_value_digit(character)) {
        error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
        error.specific.character = character;

        return error;
    }

    if (!nadir_token_append(&lexer->temporary_token, character)) {
        error.id = NADIR_LEXER_ERROR_ID_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    // If the lexer is in a state other than default, it means that there is an unfinished token.
    if (lexer->state != NADIR_LEXER_STATE_DEFAULT) {
        error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_STATE;
        return error;
    }

    // Append the EOF token.
    lexer->temporary_token = nadir_token_new(NADIR_TOKEN_ID_EOF, lexer->line, lexer->column);
    if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
        error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
    }

    return error;
}
