#ifndef NADIR_LEXER_H
#define NADIR_LEXER_H

/**
 * @file lexer.h
 * @brief The lexer interface.
 *
 * The lexer is responsible for tokenizing the input source code into
 * a list of tokens that can be processed by the parser.
 */

#include "nadir/common/list.h"
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
    NADIR_LEXER_STATE_NUMBER_BASE2,
    NADIR_LEXER_STATE_NUMBER_BASE10,
    NADIR_LEXER_STATE_NUMBER_BASE16,
    NADIR_LEXER_STATE_IDENT,
    NADIR_LEXER_STATE_COMPTIME,
    NADIR_LEXER_STATE_ADDRESS,
    NADIR_LEXER_STATE_PATH,
} nadir_lexer_state_t;

/**
 * @brief Error kinds for the lexer.
 */
typedef enum : nadir_u8_t {
    NADIR_LEXER_ERROR_KIND_NONE,
    NADIR_LEXER_ERROR_KIND_BUFFER_OVERFLOW,
    NADIR_LEXER_ERROR_KIND_OUT_OF_MEMORY,
    NADIR_LEXER_ERROR_KIND_ILLEGAL_CHARACTER,
    NADIR_LEXER_ERROR_KIND_INVALID_NUMBER,
    NADIR_LEXER_ERROR_KIND_UNEXPECTED_CHARACTER,
    NADIR_LEXER_ERROR_KIND_UNEXPECTED_STATE,
} nadir_lexer_error_kind_t;

/**
 * @brief Error structure for the lexer.
 */
typedef struct [[nodiscard]] {
    const char *path;

    nadir_u32_t line;
    nadir_u32_t column;

    char character;
    nadir_lexer_error_kind_t kind;
} nadir_lexer_error_t;

/**
 * @brief Lexer structure for the assembler.
 */
typedef struct {
    nadir_arena_t *arena;

    nadir_list_t *tokens; // List of `nadir_token_t`
    nadir_token_t token; // Temporary token for construction

    const char *source;
    const char *source_path;
    nadir_u64_t source_length;
    nadir_u64_t source_index;

    nadir_u32_t line;
    nadir_u32_t column;

    nadir_lexer_state_t state;
} nadir_lexer_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new lexer error with the given parameters.
 */
[[maybe_unused]] static inline nadir_lexer_error_t nadir_lexer_error_new(const nadir_lexer_t *lexer,
                                                                         const nadir_lexer_error_kind_t kind) {
    return (nadir_lexer_error_t){
        .kind = kind,

        .path = lexer->source_path,
        .line = lexer->line,
        .column = lexer->column,

        .character = '\0',
    };
}

/**
 * @brief Creates a new lexer with the given arena and source.
 */
[[nodiscard]] nadir_lexer_t *nadir_lexer_new(nadir_arena_t *arena,
                                             const char *path);

/**
 * @brief Collects tokens from the source and returns any lexer errors encountered.
 */
nadir_lexer_error_t nadir_lexer_collect(nadir_lexer_t *lexer);

/**
 * @brief Frees the memory allocated for the lexer.
 *
 * @warning The lexer is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_lexer_free(nadir_lexer_t *lexer);

#endif //NADIR_LEXER_H
