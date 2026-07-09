/**
 * @file lexer.c
 * @brief The lexer implementation.
 */

#include "nadir/lexer.h"

#include <string.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_lexer_error_t nadir_lexer_collect_default(nadir_lexer_t *lexer,
                                                       char character,
                                                       bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_comment(nadir_lexer_t *lexer,
                                                       char character);

static nadir_lexer_error_t nadir_lexer_collect_number_base10(nadir_lexer_t *lexer,
                                                             char character,
                                                             bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_number_base16(nadir_lexer_t *lexer,
                                                             char character,
                                                             bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_ident(nadir_lexer_t *lexer,
                                                     char character,
                                                     bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_comptime(nadir_lexer_t *lexer,
                                                        char character,
                                                        bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_address(nadir_lexer_t *lexer,
                                                       char character,
                                                       bool *recollect);

static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_lexer_t *nadir_lexer_new(nadir_arena_t *arena,
                               const char *source,
                               const nadir_u64_t source_length) {
    nadir_lexer_t *lexer = nadir_arena_allocate(arena, sizeof(nadir_lexer_t));
    if (lexer == nullptr) {
        return nullptr;
    }

    lexer->arena = arena;
    lexer->tokens = nullptr;

    const auto tokens = nadir_list_new(arena, sizeof(nadir_token_t));
    if (tokens == nullptr) {
        return nullptr;
    }

    if (!nadir_list_reserve(tokens, source_length / 2)) {
        return nullptr;
    }

    lexer->tokens = tokens;
    lexer->token = (nadir_token_t){};

    lexer->source = source;
    lexer->source_length = source_length;
    lexer->source_index = 0;

    lexer->line = 1;
    lexer->column = 1;
    lexer->state = NADIR_LEXER_STATE_DEFAULT;

    return lexer;
}

nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer) {
    auto error = (nadir_lexer_error_t){};

    while (lexer->source_index < lexer->source_length) {
        const char character = lexer->source[lexer->source_index];
        bool recollect = false;

        // Process the character based on the current lexer state.
        switch (lexer->state) {
            case NADIR_LEXER_STATE_DEFAULT:
                error = nadir_lexer_collect_default(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_COMMENT:
                error = nadir_lexer_collect_comment(lexer, character);
                break;
            case NADIR_LEXER_STATE_NUMBER_BASE10:
                error = nadir_lexer_collect_number_base10(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_NUMBER_BASE16:
                error = nadir_lexer_collect_number_base16(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_IDENT:
                error = nadir_lexer_collect_ident(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_COMPTIME:
                error = nadir_lexer_collect_comptime(lexer, character, &recollect);
                break;
            case NADIR_LEXER_STATE_ADDRESS:
                error = nadir_lexer_collect_address(lexer, character, &recollect);
                break;
        }

        // If an error occurred, return it immediately.
        if (error.kind != NADIR_LEXER_ERROR_KIND_NONE) {
            return error;
        }

        // Do not advance the source index.
        if (recollect) {
            continue;
        }

        // Update the line and column numbers based on the character.
        if (character == '\n') {
            ++lexer->line;
            lexer->column = 1;
        } else if (character != '\r') {
            ++lexer->column;
        }

        ++lexer->source_index;
    }

    error = nadir_lexer_collect_eof(lexer);
    return error;
}

void nadir_lexer_free(nadir_lexer_t *lexer) {
    if (lexer == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    lexer->tokens = nullptr;
    lexer->source = nullptr;
    lexer->line = 0;
    lexer->column = 0;
    lexer->state = NADIR_LEXER_STATE_DEFAULT;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_lexer_error_t nadir_lexer_collect_default(nadir_lexer_t *lexer,
                                                       const char character,
                                                       bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Ignore whitespace characters.
    if (nadir_token_value_whitespace(character)) {
        return error;
    }

    // Check for comment.
    if (character == NADIR_TOKEN_VALUE_COMMENT) {
        lexer->state = NADIR_LEXER_STATE_COMMENT;
        return error;
    }

    lexer->token = nadir_token_new(NADIR_TOKEN_KIND_EOF, lexer->line, lexer->column);
    nadir_token_start(&lexer->token, lexer->source, lexer->source_index);

    // Check for base 10 number.
    if (nadir_token_value_digit(character) ||
        nadir_token_value_sign(character)) {
        lexer->token.kind = NADIR_TOKEN_KIND_NUMBER;
        lexer->state = NADIR_LEXER_STATE_NUMBER_BASE10;

        *recollect = true;
        return error;
    }

    // Check for base 16 number.
    if (character == NADIR_TOKEN_VALUE_HEXADECIMAL) {
        lexer->token.kind = NADIR_TOKEN_KIND_NUMBER;
        lexer->state = NADIR_LEXER_STATE_NUMBER_BASE16;

        if (!nadir_token_increment(&lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
        }

        return error;
    }

    // Check for identifier.
    if (nadir_token_value_uppercase(character) ||
        nadir_token_value_lowercase(character) ||
        nadir_token_value_underscore(character)) {
        lexer->token.kind = NADIR_TOKEN_KIND_IDENT;
        lexer->state = NADIR_LEXER_STATE_IDENT;

        *recollect = true;
        return error;
    }

    // Check for compile-time procedures.
    if (character == NADIR_TOKEN_VALUE_COMPTIME) {
        lexer->token.kind = NADIR_TOKEN_KIND_COMPTIME;
        lexer->state = NADIR_LEXER_STATE_COMPTIME;

        if (!nadir_token_increment(&lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
        }

        return error;
    }

    // Check for store address.
    if (character == NADIR_TOKEN_VALUE_STORE_ADDRESS) {
        lexer->token.kind = NADIR_TOKEN_KIND_STORE_ADDRESS;
        lexer->state = NADIR_LEXER_STATE_ADDRESS;

        if (!nadir_token_increment(&lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
        }

        return error;
    }

    // Check for load address.
    if (character == NADIR_TOKEN_VALUE_LOAD_ADDRESS) {
        lexer->token.kind = NADIR_TOKEN_KIND_LOAD_ADDRESS;
        lexer->state = NADIR_LEXER_STATE_ADDRESS;

        if (!nadir_token_increment(&lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
        }

        return error;
    }

    // Check for single-character tokens.
    if (character == NADIR_TOKEN_VALUE_LEFT_BRACE) {
        lexer->token.kind = NADIR_TOKEN_KIND_LEFT_BRACE;
    } else if (character == NADIR_TOKEN_VALUE_RIGHT_BRACE) {
        lexer->token.kind = NADIR_TOKEN_KIND_RIGHT_BRACE;
    } else if (character == NADIR_TOKEN_VALUE_LEFT_PAREN) {
        lexer->token.kind = NADIR_TOKEN_KIND_LEFT_PAREN;
    } else if (character == NADIR_TOKEN_VALUE_RIGHT_PAREN) {
        lexer->token.kind = NADIR_TOKEN_KIND_RIGHT_PAREN;
    } else if (character == NADIR_TOKEN_VALUE_EQUAL) {
        lexer->token.kind = NADIR_TOKEN_KIND_EQUAL;
    } else if (character == NADIR_TOKEN_VALUE_COMMA) {
        lexer->token.kind = NADIR_TOKEN_KIND_COMMA;
    } else if (character == NADIR_TOKEN_VALUE_DOT) {
        lexer->token.kind = NADIR_TOKEN_KIND_DOT;
    } else if (character == NADIR_TOKEN_VALUE_SEMICOLON) {
        lexer->token.kind = NADIR_TOKEN_KIND_SEMICOLON;
    } else {
        error.kind = NADIR_LEXER_ERROR_KIND_UNKNOWN_CHARACTER;
        error.character = character;
        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
        return error;
    }

    if (!nadir_list_append(lexer->tokens, &lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
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

static nadir_lexer_error_t nadir_lexer_collect_number_base10(nadir_lexer_t *lexer,
                                                             const char character,
                                                             bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the number.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the number is not empty or just a sign.
        if (lexer->token.string.count == 1 && nadir_token_value_sign(lexer->token.string.value[0])) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

            return error;
        }

        if (lexer->token.string.count > NADIR_LEXER_NUMBER_BASE10_MAXIMUM) {
            error.kind = NADIR_LEXER_ERROR_KIND_NUMBER_TOO_LONG;
            return error;
        }

        nadir_i128_t number;
        if (!nadir_i128_decode_base10(lexer->token.string.value, lexer->token.string.count, &number)) {
            error.kind = NADIR_LEXER_ERROR_KIND_INVALID_NUMBER;
            return error;
        }

        lexer->token.number = number;
        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!(nadir_token_value_digit(character) ||
          nadir_token_value_underscore(character) ||
          (lexer->token.string.count == 0 && nadir_token_value_sign(character)))) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
        error.character = character;

        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_number_base16(nadir_lexer_t *lexer,
                                                             const char character,
                                                             bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the hexadecimal number.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the number is not empty or just a sign.
        if (lexer->token.string.count == 1 ||
            (lexer->token.string.count == 2 && nadir_token_value_sign(lexer->token.string.value[1]))) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

            return error;
        }

        if (lexer->token.string.count > NADIR_LEXER_NUMBER_BASE16_MAXIMUM) {
            error.kind = NADIR_LEXER_ERROR_KIND_NUMBER_TOO_LONG;
            return error;
        }

        nadir_i128_t number;
        if (!nadir_i128_decode_base16(lexer->token.string.value, lexer->token.string.count, &number)) {
            error.kind = NADIR_LEXER_ERROR_KIND_INVALID_NUMBER;
            return error;
        }

        lexer->token.number = number;
        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!(nadir_token_value_hexadecimal(character) ||
          nadir_token_value_underscore(character) ||
          (lexer->token.string.count == 1 && nadir_token_value_sign(character)))) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
        error.character = character;

        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_ident(nadir_lexer_t *lexer,
                                                     const char character,
                                                     bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the identifier.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

#define NADIR_LEXER_IDENT_MATCH(literal) \
    (lexer->token.string.count == (sizeof(literal) - 1) && memcmp(lexer->token.string.value, literal, lexer->token.string.count) == 0)

        if (NADIR_LEXER_IDENT_MATCH("constant")) lexer->token.kind = NADIR_TOKEN_KIND_CONSTANT;
        else if (NADIR_LEXER_IDENT_MATCH("procedure")) lexer->token.kind = NADIR_TOKEN_KIND_PROCEDURE;
        else if (NADIR_LEXER_IDENT_MATCH("binary")) lexer->token.kind = NADIR_TOKEN_KIND_BINARY;
        else if (NADIR_LEXER_IDENT_MATCH("u8")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_U8;
        else if (NADIR_LEXER_IDENT_MATCH("u16")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_U16;
        else if (NADIR_LEXER_IDENT_MATCH("u32")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_U32;
        else if (NADIR_LEXER_IDENT_MATCH("u64")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_U64;
        else if (NADIR_LEXER_IDENT_MATCH("i8")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_I8;
        else if (NADIR_LEXER_IDENT_MATCH("i16")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_I16;
        else if (NADIR_LEXER_IDENT_MATCH("i32")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_I32;
        else if (NADIR_LEXER_IDENT_MATCH("i64")) lexer->token.kind = NADIR_TOKEN_KIND_TYPE_I64;
        else lexer->token.kind = NADIR_TOKEN_KIND_IDENT;

#undef NADIR_LEXER_IDENT_MATCH

        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!nadir_token_value_uppercase(character) &&
        !nadir_token_value_lowercase(character) &&
        !nadir_token_value_digit(character) &&
        !nadir_token_value_underscore(character)
    ) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
        error.character = character;

        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
    }

    return error;
}


static nadir_lexer_error_t nadir_lexer_collect_comptime(nadir_lexer_t *lexer,
                                                        const char character,
                                                        bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the builtin.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the builtin is not empty.
        if (lexer->token.string.count == 1) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

            return error;
        }

        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!nadir_token_value_lowercase(character)) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
        error.character = character;

        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_address(nadir_lexer_t *lexer,
                                                       const char character,
                                                       bool *recollect) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Check for whitespace or single-character tokens to end the address.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the address is not empty.
        if (lexer->token.string.count == 1) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

            return error;
        }

        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        *recollect = true;
        return error;
    }

    // Check for valid characters.
    if (!nadir_token_value_uppercase(character) && !nadir_token_value_underscore(character)) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
        error.character = character;

        return error;
    }

    if (!nadir_token_increment(&lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
    }

    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer) {
    auto error = nadir_lexer_error_new(NADIR_LEXER_ERROR_KIND_NONE, lexer->line, lexer->column);

    // Grammatically, the lexer should be in the default state when it reaches EOF.
    if (lexer->state != NADIR_LEXER_STATE_DEFAULT) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE;
        return error;
    }

    // Create an EOF token.
    lexer->token = nadir_token_new(NADIR_TOKEN_KIND_EOF, lexer->line, lexer->column);

    if (!nadir_list_append(lexer->tokens, &lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
    }

    return error;
}
