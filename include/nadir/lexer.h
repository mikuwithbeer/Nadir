#ifndef NADIR_LEXER_H
#define NADIR_LEXER_H

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr nadir_u8_t NADIR_LEXER_DEFAULT_TOKEN_LIST_CAPACITY = 1 << 6;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Lexer states for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_LEXER_STATE_DEFAULT = 0,
    NADIR_LEXER_STATE_COMMENT,
    NADIR_LEXER_STATE_NUMBER,
    NADIR_LEXER_STATE_IDENT,
} nadir_lexer_state_t;

/**
 * @brief Error IDs for the lexer.
 */
typedef enum : nadir_u8_t {
    NADIR_LEXER_ERROR_ID_NONE = 0,
    NADIR_LEXER_ERROR_ID_UNEXPECTED_CHARACTER,
} nadir_lexer_error_id_t;

/**
 * @brief Error structure for the lexer.
 */
typedef struct {
    nadir_lexer_error_id_t id;

    nadir_u64_t line;
    nadir_u64_t column;

    union {
        char unexpected_character;
    } specific;
} nadir_lexer_error_t;

/**
 * @brief Lexer structure for the assembler.
 */
typedef struct {
    nadir_token_list_t *token_list;
    nadir_token_t temporary_token;

    const char *source;
    nadir_u64_t source_length;

    nadir_u64_t line;
    nadir_u64_t column;
    nadir_lexer_state_t state;
} nadir_lexer_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new lexer with the given source code.
 *
 * @warning Allocates memory for the lexer, which must be freed.
 */
nadir_lexer_t *nadir_lexer_new(const char *source,
                               nadir_u64_t source_length);

/**
 * @brief Runs the lexer on the source code and populates the token list.
 */
nadir_lexer_error_t nadir_lexer_run(nadir_lexer_t *lexer);

/**
 * @brief Frees the memory allocated for the lexer.
 */
void nadir_lexer_free(nadir_lexer_t *lexer);

#endif //NADIR_LEXER_H
