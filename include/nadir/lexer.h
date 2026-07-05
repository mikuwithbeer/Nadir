#ifndef NADIR_LEXER_H
#define NADIR_LEXER_H

/**
 * @file lexer.h
 * @brief The lexer interface.
 *
 * The lexer is responsible for tokenizing the input source code into
 * a list of tokens that can be processed by the parser.
 */

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_LEXER_NUMBER_BASE10_MAXIMUM = 41; // 40 digits + sign
constexpr auto NADIR_LEXER_NUMBER_BASE16_MAXIMUM = (1 << 5) + 1; // 32 digits + sign

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
    nadir_u64_t line;
    nadir_u64_t column;

    nadir_lexer_error_kind_t kind;
    char character;
} nadir_lexer_error_t;

/**
 * @brief Lexer structure for the assembler.
 */
typedef struct {
    nadir_list_t *tokens; // List of `nadir_token_t`
    nadir_token_t token; // Temporary token for construction

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
    return (nadir_lexer_error_t){
        .line = line,
        .column = column,

        .kind = kind,
        .character = '\0',
    };
}

/**
 * @brief Creates a new lexer with the given source and length.
 *
 * @warning Allocates memory for the lexer and its associated resources.
 */
[[nodiscard]] nadir_lexer_t *nadir_lexer_new(const char *source,
                                             nadir_u64_t source_length);

/**
 * @brief Collects tokens from the source and returns any lexer errors encountered.
 */
nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer);

/**
 * @brief Frees the lexer and its associated resources.
 */
void nadir_lexer_free(nadir_lexer_t *lexer);

#endif //NADIR_LEXER_H
