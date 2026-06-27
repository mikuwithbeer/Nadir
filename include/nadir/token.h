#ifndef NADIR_TOKEN_H
#define NADIR_TOKEN_H

#include "nadir/common.h"

constexpr nadir_u8_t NADIR_TOKEN_BUFFER_SIZE = 1 << 7;

/**
 * @brief Token types for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_TOKEN_ID_ERROR = 0,

    NADIR_TOKEN_ID_MAP,
    NADIR_TOKEN_ID_INSTRUCTION,

    NADIR_TOKEN_ID_IDENT,
    NADIR_TOKEN_ID_NUMBER,

    NADIR_TOKEN_ID_LEFT_BRACE,
    NADIR_TOKEN_ID_RIGHT_BRACE,

    NADIR_TOKEN_ID_LEFT_PAREN,
    NADIR_TOKEN_ID_RIGHT_PAREN,

    NADIR_TOKEN_ID_EQUAL,
    NADIR_TOKEN_ID_COMMA,

    NADIR_TOKEN_ID_EOF = 0xFF
} nadir_token_id_t;

/**
 * @brief Error types for the token.
 */
typedef enum : nadir_u8_t {
    NADIR_TOKEN_ERROR_ID_UNEXPECTED_CHARACTER = 1,
    NADIR_TOKEN_ERROR_ID_INVALID_NUMBER,
} nadir_token_error_id_t;

/**
 * @brief Error structure for the token.
 */
typedef struct {
    nadir_token_error_id_t id;

    // Human-readable message for the error.
    char message[NADIR_TOKEN_BUFFER_SIZE];
} nadir_token_error_t;

/**
 * @brief Token structure for the assembler.
 */
typedef struct {
    nadir_token_id_t id;

    nadir_u64_t line;
    nadir_u64_t column;

    char value[NADIR_TOKEN_BUFFER_SIZE];
    nadir_u64_t value_length;

    // Extra data for the token, depending on the token type.
    union {
        nadir_u64_t number;
        nadir_token_error_id_t error;
    } specific;
} nadir_token_t;

/**
 * @brief Creates a new token with the given parameters.
 */
nadir_token_t nadir_token_new(nadir_token_id_t id,
                              nadir_u64_t line,
                              nadir_u64_t column);

/**
 * @brief Appends a character to the token's value.
 *
 * @return false if the buffer is full, true otherwise.
 */
bool nadir_token_append(nadir_token_t *token,
                        char character);

#endif //NADIR_TOKEN_H
