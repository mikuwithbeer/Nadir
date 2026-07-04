#ifndef NADIR_TOKEN_H
#define NADIR_TOKEN_H

#include "nadir/common.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_TOKEN_VALUE_MAXIMUM = (1 << 6) + 1;

constexpr auto NADIR_TOKEN_VALUE_COMMENT = '#';
constexpr auto NADIR_TOKEN_VALUE_COMPTIME = '@';
constexpr auto NADIR_TOKEN_VALUE_STORE_ADDRESS = '<';
constexpr auto NADIR_TOKEN_VALUE_LOAD_ADDRESS = '>';
constexpr auto NADIR_TOKEN_VALUE_LEFT_BRACE = '{';
constexpr auto NADIR_TOKEN_VALUE_RIGHT_BRACE = '}';
constexpr auto NADIR_TOKEN_VALUE_LEFT_PAREN = '(';
constexpr auto NADIR_TOKEN_VALUE_RIGHT_PAREN = ')';
constexpr auto NADIR_TOKEN_VALUE_EQUAL = '=';
constexpr auto NADIR_TOKEN_VALUE_COMMA = ',';
constexpr auto NADIR_TOKEN_VALUE_DOT = '.';
constexpr auto NADIR_TOKEN_VALUE_SEMICOLON = ';';

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Token types for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_TOKEN_KIND_NUMBER,
    NADIR_TOKEN_KIND_IDENT,
    NADIR_TOKEN_KIND_COMPTIME,
    NADIR_TOKEN_KIND_STORE_ADDRESS,
    NADIR_TOKEN_KIND_LOAD_ADDRESS,

    NADIR_TOKEN_KIND_LEFT_BRACE,
    NADIR_TOKEN_KIND_RIGHT_BRACE,
    NADIR_TOKEN_KIND_LEFT_PAREN,
    NADIR_TOKEN_KIND_RIGHT_PAREN,
    NADIR_TOKEN_KIND_EQUAL,
    NADIR_TOKEN_KIND_COMMA,
    NADIR_TOKEN_KIND_DOT,
    NADIR_TOKEN_KIND_SEMICOLON,

    NADIR_TOKEN_KIND_CONSTANT,
    NADIR_TOKEN_KIND_PROCEDURE,
    NADIR_TOKEN_KIND_BINARY,

    NADIR_TOKEN_KIND_TYPE_U8,
    NADIR_TOKEN_KIND_TYPE_U16,
    NADIR_TOKEN_KIND_TYPE_U32,
    NADIR_TOKEN_KIND_TYPE_U64,
    NADIR_TOKEN_KIND_TYPE_I8,
    NADIR_TOKEN_KIND_TYPE_I16,
    NADIR_TOKEN_KIND_TYPE_I32,
    NADIR_TOKEN_KIND_TYPE_I64,

    NADIR_TOKEN_KIND_EOF = 0xFF
} nadir_token_kind_t;

/**
 * @brief Token structure for the assembler.
 */
typedef struct {
    nadir_token_kind_t kind;

    nadir_u64_t line;
    nadir_u64_t column;

    char value[NADIR_TOKEN_VALUE_MAXIMUM];
    nadir_u64_t value_length;

    // Extra data for the token, depending on token type.
    union {
        nadir_i128_t number;
    } specific;
} nadir_token_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new token with the given parameters.
 */
[[nodiscard]] nadir_token_t nadir_token_new(nadir_token_kind_t kind,
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
 * @brief Checks if a character is a whitespace character.
 */
static inline bool nadir_token_value_whitespace(const char character) {
    return character == ' ' || character == '\n' || character == '\t' || character == '\r';
}

/**
 * @brief Checks if a character is an uppercase alphabetic character with underscore (A-Z, _).
 */
static inline bool nadir_token_value_upper_underscore(const char character) {
    return (character >= 'A' && character <= 'Z') || character == '_';
}

/**
 * @brief Checks if a character is a lowercase alphabetic character with underscore (a-z, _).
 */
static inline bool nadir_token_value_lower_underscore(const char character) {
    return (character >= 'a' && character <= 'z') || character == '_';
}

/**
 * @brief Checks if a character is an alphabetic character (a-z, A-Z).
 */
static inline bool nadir_token_value_alpha(const char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

/**
 * @brief Checks if a character is a digit (0-9).
 */
static inline bool nadir_token_value_digit(const char character) {
    return character >= '0' && character <= '9';
}

/**
 * @brief Checks if a character is a single-character token.
 */
static inline bool nadir_token_value_single(const char character) {
    return character == NADIR_TOKEN_VALUE_COMMENT ||
           character == NADIR_TOKEN_VALUE_LEFT_BRACE ||
           character == NADIR_TOKEN_VALUE_RIGHT_BRACE ||
           character == NADIR_TOKEN_VALUE_LEFT_PAREN ||
           character == NADIR_TOKEN_VALUE_RIGHT_PAREN ||
           character == NADIR_TOKEN_VALUE_EQUAL ||
           character == NADIR_TOKEN_VALUE_COMMA ||
           character == NADIR_TOKEN_VALUE_DOT ||
           character == NADIR_TOKEN_VALUE_SEMICOLON;
}

/**
 * @brief Checks if a token kind is a type token.
 */
static inline bool nadir_token_is_type(const nadir_token_kind_t kind) {
    return kind >= NADIR_TOKEN_KIND_TYPE_U8 && kind <= NADIR_TOKEN_KIND_TYPE_I64;
}

#endif //NADIR_TOKEN_H
