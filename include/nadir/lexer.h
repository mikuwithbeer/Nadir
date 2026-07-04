#ifndef NADIR_LEXER_H
#define NADIR_LEXER_H

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Lexer states for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_LEXER_STATE_DEFAULT,
    NADIR_LEXER_STATE_COMMENT,
    NADIR_LEXER_STATE_NUMBER_BASE10,
    NADIR_LEXER_STATE_NUMBER_BASE16,
    NADIR_LEXER_STATE_IDENT,
    NADIR_LEXER_STATE_COMPTIME,
    NADIR_LEXER_STATE_ADDRESS,
} nadir_lexer_state_t;

/**
 * @brief Error kinds for the lexer.
 */
typedef enum [[nodiscard]] : nadir_u8_t {
    NADIR_LEXER_ERROR_KIND_NONE,
    NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW,
    NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY,
    NADIR_LEXER_ERROR_KIND_UNKNOWN_CHARACTER,
    NADIR_LEXER_ERROR_KIND_NUMBER_TOO_LONG,
    NADIR_LEXER_ERROR_KIND_INVALID_NUMBER,
    NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER,
    NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE,
} nadir_lexer_error_kind_t;

/**
 * @brief Error structure for the lexer.
 */
typedef struct [[nodiscard]] {
    nadir_lexer_error_kind_t kind;

    nadir_u64_t line;
    nadir_u64_t column;

    union {
        char character;
    } specific;
} nadir_lexer_error_t;

/**
 * @brief Lexer structure for the assembler.
 */
typedef struct {
    nadir_list_t *tokens;
    nadir_token_t token;

    const char *source;
    nadir_u64_t source_length;
    nadir_u64_t source_index;

    nadir_u64_t line;
    nadir_u64_t column;

    nadir_lexer_state_t state;
} nadir_lexer_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new lexer error with the given parameters.
 */
static inline nadir_lexer_error_t nadir_lexer_error_new(const nadir_lexer_error_kind_t kind,
                                                        const nadir_u64_t line,
                                                        const nadir_u64_t column) {
    auto error = (nadir_lexer_error_t){};

    error.kind = kind;
    error.line = line;
    error.column = column;

    return error;
}

/**
 * @brief Creates a new lexer with the given source code.
 *
 * @warning Allocates memory for the lexer, which must be freed.
 */
[[nodiscard]] nadir_lexer_t *nadir_lexer_new(const char *source,
                                             nadir_u64_t source_length);

/**
 * @brief Runs the lexer on the source code and collects tokens.
 */
nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer);

/**
 * @brief Frees the memory allocated for the lexer.
 */
void nadir_lexer_free(nadir_lexer_t *lexer);

#endif //NADIR_LEXER_H
