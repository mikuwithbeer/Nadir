/**
 * @file lexer.c
 * @brief The lexer implementation.
 */

#include "nadir/lexer.h"

#include <stdio.h>
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

static nadir_lexer_error_t nadir_lexer_collect_path(nadir_lexer_t *lexer,
                                                    char character);

static nadir_lexer_error_t nadir_lexer_collect_eof(nadir_lexer_t *lexer);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_lexer_t *nadir_lexer_new(nadir_arena_t *arena,
                               const char *source_path) {
    nadir_lexer_t *lexer = nadir_arena_allocate(arena, sizeof(nadir_lexer_t));
    if (lexer == nullptr) {
        return nullptr;
    }

    lexer->arena = arena;

    const auto source_file = fopen(source_path, "rb");
    if (source_file == nullptr) {
        return nullptr;
    }

    fseek(source_file, 0, SEEK_END);
    const auto source_size = ftell(source_file); // End of the file to determine the size
    fseek(source_file, 0, SEEK_SET);

    if (source_size <= 0) {
        fclose(source_file);
        return nullptr;
    }

    char *source_buffer = nadir_arena_allocate(arena, (nadir_u64_t) source_size + 1);
    if (source_buffer == nullptr) {
        fclose(source_file);
        return nullptr;
    }

    const auto source_length = fread(source_buffer, sizeof(char), (nadir_u64_t) source_size, source_file);
    source_buffer[source_length] = '\0'; // Null-terminate the buffer for safety

    if ((nadir_u64_t) source_size != source_length) {
        fclose(source_file);
        return nullptr;
    }

    lexer->source = source_buffer;
    lexer->source_path = source_path;
    lexer->source_length = source_length;
    lexer->source_index = 0;

    const auto tokens = nadir_list_new(arena, sizeof(nadir_token_t));
    if (tokens == nullptr) {
        return nullptr;
    }

    // Reserve space for tokens based on the source length.
    if (!nadir_list_reserve(tokens, source_length / 2)) {
        return nullptr;
    }

    lexer->token = (nadir_token_t){};
    lexer->tokens = tokens;

    lexer->line = 1;
    lexer->column = 1;
    lexer->state = NADIR_LEXER_STATE_DEFAULT;

    return lexer;
}

nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer) {
    auto error = (nadir_lexer_error_t){};

    // State machine that transitions between different states based on the characters it encounters in the source code.
    while (lexer->source_index < lexer->source_length) {
        const char character = lexer->source[lexer->source_index];
        bool recollect = false; // Determines whether current character should be reprocessed in the next iteration

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
            case NADIR_LEXER_STATE_PATH:
                error = nadir_lexer_collect_path(lexer, character);
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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);
    if (nadir_token_value_whitespace(character)) {
        return error;
    }

    if (character == NADIR_TOKEN_VALUE_COMMENT) {
        lexer->state = NADIR_LEXER_STATE_COMMENT;
        return error;
    }

    lexer->token = nadir_token_new(lexer->source_path, NADIR_TOKEN_KIND_EOF, lexer->line, lexer->column);

    // Start the token value with the current character position.
    nadir_token_start(&lexer->token, lexer->source, lexer->source_index);

    switch (character) {
        case NADIR_TOKEN_VALUE_LEFT_BRACE:
            lexer->token.kind = NADIR_TOKEN_KIND_LEFT_BRACE;
            break;
        case NADIR_TOKEN_VALUE_RIGHT_BRACE:
            lexer->token.kind = NADIR_TOKEN_KIND_RIGHT_BRACE;
            break;
        case NADIR_TOKEN_VALUE_LEFT_PAREN:
            lexer->token.kind = NADIR_TOKEN_KIND_LEFT_PAREN;
            break;
        case NADIR_TOKEN_VALUE_RIGHT_PAREN:
            lexer->token.kind = NADIR_TOKEN_KIND_RIGHT_PAREN;
            break;
        case NADIR_TOKEN_VALUE_EQUAL:
            lexer->token.kind = NADIR_TOKEN_KIND_EQUAL;
            break;
        case NADIR_TOKEN_VALUE_COMMA:
            lexer->token.kind = NADIR_TOKEN_KIND_COMMA;
            break;
        case NADIR_TOKEN_VALUE_DOT:
            lexer->token.kind = NADIR_TOKEN_KIND_DOT;
            break;
        case NADIR_TOKEN_VALUE_SEMICOLON:
            lexer->token.kind = NADIR_TOKEN_KIND_SEMICOLON;
            break;
        case NADIR_TOKEN_VALUE_LEFT_BRACKET:
            lexer->token.kind = NADIR_TOKEN_KIND_PATH;
            lexer->state = NADIR_LEXER_STATE_PATH;

            if (!nadir_token_increment(&lexer->token)) {
                error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            }

            return error;
        case NADIR_TOKEN_VALUE_HEXADECIMAL:
            lexer->token.kind = NADIR_TOKEN_KIND_NUMBER;
            lexer->state = NADIR_LEXER_STATE_NUMBER_BASE16;

            if (!nadir_token_increment(&lexer->token)) {
                error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            }

            return error;
        case NADIR_TOKEN_VALUE_COMPTIME:
            lexer->token.kind = NADIR_TOKEN_KIND_COMPTIME;
            lexer->state = NADIR_LEXER_STATE_COMPTIME;

            if (!nadir_token_increment(&lexer->token)) {
                error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            }

            return error;
        case NADIR_TOKEN_VALUE_STORE_ADDRESS:
            lexer->token.kind = NADIR_TOKEN_KIND_STORE_ADDRESS;
            lexer->state = NADIR_LEXER_STATE_ADDRESS;

            if (!nadir_token_increment(&lexer->token)) {
                error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            }

            return error;
        case NADIR_TOKEN_VALUE_LOAD_ADDRESS:
            lexer->token.kind = NADIR_TOKEN_KIND_LOAD_ADDRESS;
            lexer->state = NADIR_LEXER_STATE_ADDRESS;

            if (!nadir_token_increment(&lexer->token)) {
                error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            }

            return error;
        default:
            break;
    }

    if (nadir_token_value_digit(character) || nadir_token_value_sign(character)) {
        lexer->token.kind = NADIR_TOKEN_KIND_NUMBER;
        lexer->state = NADIR_LEXER_STATE_NUMBER_BASE10;

        *recollect = true;
        return error;
    }

    if (nadir_token_value_uppercase(character) ||
        nadir_token_value_lowercase(character) ||
        nadir_token_value_underscore(character)) {
        lexer->token.kind = NADIR_TOKEN_KIND_IDENT;
        lexer->state = NADIR_LEXER_STATE_IDENT;

        *recollect = true;
        return error;
    }

    // Immediately finalize single-character tokens.
    if (nadir_token_value_single(character)) {
        if (!nadir_token_increment(&lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW;
            return error;
        }

        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        return error;
    }

    // Fallback for unrecognized characters.
    error.kind = NADIR_LEXER_ERROR_KIND_UNKNOWN_CHARACTER;
    error.character = character;
    return error;
}

static nadir_lexer_error_t nadir_lexer_collect_comment(nadir_lexer_t *lexer,
                                                       const char character) {
    // Comments are ignored until a newline is encountered.
    if (character == '\n') {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;
    }

    return (nadir_lexer_error_t){};
}

static nadir_lexer_error_t nadir_lexer_collect_number_base10(nadir_lexer_t *lexer,
                                                             const char character,
                                                             bool *recollect) {
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

    // Check for whitespace or single-character tokens to end the number.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the number is not empty or just a sign.
        if (lexer->token.string.count == 1 && nadir_token_value_sign(lexer->token.string.value[0])) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

    // Check for whitespace or single-character tokens to end the identifier.
    if (nadir_token_value_whitespace(character) || nadir_token_value_single(character)) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

#define NADIR_LEXER_IDENT_MATCH(literal) \
    (lexer->token.string.count == (sizeof(literal) - 1) && memcmp(lexer->token.string.value, literal, lexer->token.string.count) == 0)

        // Reserved keywords and types are also valid identifiers, so we check for them here.
        if (NADIR_LEXER_IDENT_MATCH("constant")) lexer->token.kind = NADIR_TOKEN_KIND_CONSTANT;
        else if (NADIR_LEXER_IDENT_MATCH("procedure")) lexer->token.kind = NADIR_TOKEN_KIND_PROCEDURE;
        else if (NADIR_LEXER_IDENT_MATCH("binary")) lexer->token.kind = NADIR_TOKEN_KIND_BINARY;
        else if (NADIR_LEXER_IDENT_MATCH("include")) lexer->token.kind = NADIR_TOKEN_KIND_INCLUDE;
        else if (NADIR_LEXER_IDENT_MATCH("until")) lexer->token.kind = NADIR_TOKEN_KIND_UNTIL;
        else if (NADIR_LEXER_IDENT_MATCH("repeat")) lexer->token.kind = NADIR_TOKEN_KIND_REPEAT;
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
        !nadir_token_value_underscore(character)) {
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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

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

static nadir_lexer_error_t nadir_lexer_collect_path(nadir_lexer_t *lexer,
                                                    const char character) {
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

    // Check for the closing bracket to end the path.
    if (character == NADIR_TOKEN_VALUE_RIGHT_BRACKET) {
        lexer->state = NADIR_LEXER_STATE_DEFAULT;

        // Check if the path is not empty.
        if (lexer->token.string.count == 1) {
            error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER;
            error.character = character;

            return error;
        }

        // Remove the brackets.
        ++lexer->token.string.value;
        --lexer->token.string.count;

        if (!nadir_list_append(lexer->tokens, &lexer->token)) {
            error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
        }

        return error;
    }

    if (!nadir_token_value_path(character)) {
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
    auto error = nadir_lexer_error_new(lexer, NADIR_LEXER_ERROR_KIND_NONE);

    // Grammatically, the lexer should be in the default state when it reaches EOF.
    if (lexer->state != NADIR_LEXER_STATE_DEFAULT) {
        error.kind = NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE;
        return error;
    }

    // Create an EOF token to signify the end of the token stream.
    lexer->token = nadir_token_new(lexer->source_path, NADIR_TOKEN_KIND_EOF, lexer->line, lexer->column);
    if (!nadir_list_append(lexer->tokens, &lexer->token)) {
        error.kind = NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY;
    }

    return error;
}
