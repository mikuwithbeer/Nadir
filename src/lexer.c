#include "nadir/lexer.h"

#include <stdlib.h>

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

        if (error.id != NADIR_LEXER_ERROR_ID_NONE) {
            return error;
        }

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

    if (character == ' ' || character == '\n' || character == '\t' || character == '\r') {
        return error;
    }

    lexer->temporary_token = nadir_token_new(NADIR_TOKEN_ID_EOF, lexer->line, lexer->column);

    if (character == ';') {
        lexer->state = NADIR_LEXER_STATE_COMMENT;
        return error;
    }

    if (character >= '0' && character <= '9' || character == '_') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_NUMBER;
        lexer->state = NADIR_LEXER_STATE_NUMBER;

        *recollect = true;
        return error;
    }

    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z')) {
        lexer->temporary_token.id = NADIR_TOKEN_ID_IDENT;
        lexer->state = NADIR_LEXER_STATE_IDENT;

        *recollect = true;
        return error;
    }

    if (character == '{') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_BRACE;
    } else if (character == '}') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_BRACE;
    } else if (character == '(') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_LEFT_PAREN;
    } else if (character == ')') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_RIGHT_PAREN;
    } else if (character == '=') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_EQUAL;
    } else if (character == ',') {
        lexer->temporary_token.id = NADIR_TOKEN_ID_COMMA;
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
    if (character == '\n') {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;
    }

    return nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);
}

static nadir_lexer_error_t nadir_lexer_collect_number(nadir_lexer_t *lexer,
                                                      const char character,
                                                      bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    if (character == ' ' || character == '\n' || character == '\t' || character == '\r') {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
            error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    if (character == '_') {
        return error;
    }

    if (!nadir_token_append(&lexer->temporary_token, character)) {
        error.id = NADIR_LEXER_ERROR_ID_BUFFER_OVERFLOW;
        return error;
    }

    if (character < '0' || character > '9') {
        error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
        error.specific.character = character;
    }

    return error;
}


static nadir_lexer_error_t nadir_lexer_collect_ident(nadir_lexer_t *lexer,
                                                     const char character,
                                                     bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    if (character == ' ' || character == '\n' || character == '\t' || character == '\r') {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
            error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    if (!nadir_token_append(&lexer->temporary_token, character)) {
        error.id = NADIR_LEXER_ERROR_ID_BUFFER_OVERFLOW;
        return error;
    }

    if ((character < 'a' || character > 'z') &&
        (character < 'A' || character > 'Z')) {
        error.id = NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER;
        error.specific.character = character;
    }

    return error;
}


static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_ID_NONE, lexer->line, lexer->column);

    if (lexer->state == NADIR_LEXER_STATE_NUMBER || lexer->state == NADIR_LEXER_STATE_IDENT) {
        if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
            error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
            return error;
        }
    }

    lexer->temporary_token = nadir_token_new(NADIR_TOKEN_ID_EOF, lexer->line, lexer->column);
    if (!nadir_token_list_append(lexer->token_list, lexer->temporary_token)) {
        error.id = NADIR_LEXER_ERROR_ID_OUT_OF_MEMORY;
    }

    return error;
}
