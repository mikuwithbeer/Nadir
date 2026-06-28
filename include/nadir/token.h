#ifndef NADIR_TOKEN_H
#define NADIR_TOKEN_H

#include "nadir/common.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr nadir_u8_t NADIR_TOKEN_BUFFER_SIZE = 1 << 7;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Token types for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_TOKEN_ID_NUMBER,
    NADIR_TOKEN_ID_IDENT,

    NADIR_TOKEN_ID_LEFT_BRACE,
    NADIR_TOKEN_ID_RIGHT_BRACE,

    NADIR_TOKEN_ID_LEFT_PAREN,
    NADIR_TOKEN_ID_RIGHT_PAREN,

    NADIR_TOKEN_ID_EQUAL,
    NADIR_TOKEN_ID_COMMA,

    NADIR_TOKEN_ID_MAP,
    NADIR_TOKEN_ID_INSTRUCTION,

    NADIR_TOKEN_ID_EOF = 0xFF
} nadir_token_id_t;

/**
 * @brief Token structure for the assembler.
 */
typedef struct {
    nadir_token_id_t id;

    nadir_u64_t line;
    nadir_u64_t column;

    char value[NADIR_TOKEN_BUFFER_SIZE];
    nadir_u64_t value_length;

    // Extra data for the token, depending on token type.
    union {
        nadir_u64_t number;
    } specific;
} nadir_token_t;

/**
 * @brief List of tokens for the assembler.
 */
typedef struct {
    nadir_token_t *tokens;

    nadir_u64_t token_count;
    nadir_u64_t token_capacity;
} nadir_token_list_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new token with the given parameters.
 */
[[nodiscard]] nadir_token_t nadir_token_new(nadir_token_id_t id,
                                            nadir_u64_t line,
                                            nadir_u64_t column);

/**
 * @brief Appends a character to the token's value.
 *
 * @return false if the buffer is full, true otherwise.
 */
[[nodiscard]] bool nadir_token_append(nadir_token_t *token,
                                      char character);

/**
 * @brief Creates a new token list with the given initial capacity.
 *
 * @warning Allocates memory for the token list, which must be freed.
 */
[[nodiscard]] nadir_token_list_t *nadir_token_list_new(nadir_u64_t initial_capacity);

/**
 * @brief Appends a token to the token list.
 *
 * @return false if the reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_token_list_append(nadir_token_list_t *token_list,
                                           nadir_token_t token);

/**
 * @brief Frees the memory allocated for the token list.
 */
void nadir_token_list_free(nadir_token_list_t *token_list);

#endif //NADIR_TOKEN_H
