#ifndef NADIR_TOKEN_H
#define NADIR_TOKEN_H

/**
 * @file token.h
 * @brief The token interface.
 *
 * This file defines the token structure and related constants for
 * the assembler.
 */

#include "nadir/common/number.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_TOKEN_DEFAULT_MAXIMUM = 1 << 6;
constexpr auto NADIR_TOKEN_NUMBER_MAXIMUM = 0x30;
constexpr auto NADIR_TOKEN_PATH_MAXIMUM = 1 << 9;

constexpr auto NADIR_TOKEN_VALUE_COMMENT = '#';
constexpr auto NADIR_TOKEN_VALUE_COMPTIME = '@';
constexpr auto NADIR_TOKEN_VALUE_HEXADECIMAL = '$';
constexpr auto NADIR_TOKEN_VALUE_STORE_ADDRESS = '<';
constexpr auto NADIR_TOKEN_VALUE_LOAD_ADDRESS = '>';
constexpr auto NADIR_TOKEN_VALUE_LEFT_BRACE = '{';
constexpr auto NADIR_TOKEN_VALUE_RIGHT_BRACE = '}';
constexpr auto NADIR_TOKEN_VALUE_LEFT_PAREN = '(';
constexpr auto NADIR_TOKEN_VALUE_RIGHT_PAREN = ')';
constexpr auto NADIR_TOKEN_VALUE_LEFT_BRACKET = '[';
constexpr auto NADIR_TOKEN_VALUE_RIGHT_BRACKET = ']';
constexpr auto NADIR_TOKEN_VALUE_EQUAL = '=';
constexpr auto NADIR_TOKEN_VALUE_COMMA = ',';
constexpr auto NADIR_TOKEN_VALUE_DOT = '.';
constexpr auto NADIR_TOKEN_VALUE_SEMICOLON = ';';

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Token kinds for the assembler.
 */
typedef enum : nadir_u8_t {
    NADIR_TOKEN_KIND_NUMBER,
    NADIR_TOKEN_KIND_IDENT,
    NADIR_TOKEN_KIND_PATH,
    NADIR_TOKEN_KIND_COMPTIME,
    NADIR_TOKEN_KIND_STORE_ADDRESS,
    NADIR_TOKEN_KIND_LOAD_ADDRESS,
    NADIR_TOKEN_KIND_UNTIL,
    NADIR_TOKEN_KIND_REPEAT,

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
    NADIR_TOKEN_KIND_INCLUDE,

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
    union {
        nadir_i128_t number;

        struct {
            const char *value;
            nadir_u64_t count;
        } string;
    };

    const char *path;

    nadir_u32_t line;
    nadir_u32_t column;

    nadir_token_kind_t kind;
} nadir_token_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new token with the given parameters.
 */
[[nodiscard]] nadir_token_t nadir_token_new(const char *path,
                                            nadir_token_kind_t kind,
                                            nadir_u32_t line,
                                            nadir_u32_t column);

/**
 * @brief Initializes the token string value and count.
 */
void nadir_token_start(nadir_token_t *token,
                       const char *source,
                       nadir_u64_t index);

/**
 * @brief Increments the token string length.
 *
 * @return false if it reached the maximum limit, true otherwise.
 */
[[nodiscard]] bool nadir_token_increment(nadir_token_t *token);

/**
 * @brief Checks if a character is a whitespace character (' ', '\\n', '\\t', '\\r').
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_whitespace(const char character) {
    return character == ' ' || character == '\n' || character == '\t' || character == '\r';
}

/**
 * @brief Checks if a character is an uppercase alphabetic character (A-Z).
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_uppercase(const char character) {
    return character >= 'A' && character <= 'Z';
}

/**
 * @brief Checks if a character is a lowercase alphabetic character (a-z).
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_lowercase(const char character) {
    return character >= 'a' && character <= 'z';
}

/**
 * @brief Checks if a character is an underscore ('_').
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_underscore(const char character) {
    return character == '_';
}

/**
 * @brief Checks if a character is a digit (0-9).
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_digit(const char character) {
    return character >= '0' && character <= '9';
}

/**
 * @brief Checks if a character is a hexadecimal (0-9, A-F, a-f).
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_hexadecimal(const char character) {
    return (character >= '0' && character <= '9') ||
           (character >= 'A' && character <= 'F') ||
           (character >= 'a' && character <= 'f');
}

/**
 * @brief Checks if a character is a sign character ('+', '-').
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_sign(const char character) {
    return character == '+' || character == '-';
}

/**
 * @brief Checks if a character is valid in a path character.
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_path(const char character) {
    return (character >= '0' && character <= '9') ||
           (character >= 'A' && character <= 'Z') ||
           (character >= 'a' && character <= 'z') ||
           character == '_' ||
           character == '-' ||
           character == '.' ||
           character == ':' ||
           character == ' ' ||
           character == '/' ||
           character == '\\';
}

/**
 * @brief Checks if a character is a single-character token.
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_single(const char character) {
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
 * @brief Checks if a token kind is a type token ('u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32', 'i64').
 */
[[nodiscard, maybe_unused]] static inline bool nadir_token_value_type(const nadir_token_kind_t kind) {
    return kind >= NADIR_TOKEN_KIND_TYPE_U8 && kind <= NADIR_TOKEN_KIND_TYPE_I64;
}

#endif //NADIR_TOKEN_H
