#ifndef NADIR_PARSER_H
#define NADIR_PARSER_H

/**
 * @file parser.h
 * @brief The parser interface.
 *
 * The parser is responsible for parsing the list of tokens produced
 * by the lexer into an abstract syntax tree.
 */

#include "nadir/ast.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_PARSER_ARGUMENTS_MAXIMUM = 1 << 4;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Error kinds for the parser.
 */
typedef enum : nadir_u8_t {
    NADIR_PARSER_ERROR_KIND_NONE,
    NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY,
    NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN,
    NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF,
    NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION,
    NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK,
    NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION,
    NADIR_PARSER_ERROR_KIND_MISSING_BINARY_ORIGIN,
    NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS,
    NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY,
    NADIR_PARSER_ERROR_KIND_INVALID_BINARY_ORIGIN,
} nadir_parser_error_kind_t;

/**
 * @brief Error structure for the parser.
 */
typedef struct [[nodiscard]] {
    nadir_parser_error_kind_t kind;
    nadir_token_t *token;

    nadir_token_kind_t expected;
} nadir_parser_error_t;

/**
 * @brief Parser structure for the assembler.
 */
typedef struct {
    nadir_arena_t *arena;
    nadir_ast_t *ast;

    nadir_list_t *tokens; // List of `nadir_token_t`
    nadir_u64_t token_index;

    bool seen_binary; // Flag to avoid multiple binary declarations
} nadir_parser_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new parser error with the given kind and token.
 */
[[maybe_unused]] static inline nadir_parser_error_t nadir_parser_error_new(const nadir_parser_error_kind_t kind,
                                                                           nadir_token_t *token) {
    return (nadir_parser_error_t){
        .kind = kind,
        .token = token,
    };
}

/**
 * @brief Creates a new parser with the given arena and list of tokens.
 */
nadir_parser_t *nadir_parser_new(nadir_arena_t *arena,
                                 nadir_list_t *tokens);

/**
 * @brief Generates an abstract syntax tree from the list of tokens.
 */
nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser);

/**
 * @brief Frees the memory allocated for the parser.
 *
 * @warning The parser is allocated on the arena and will be freed when the arena is freed.
 */
void nadir_parser_free(nadir_parser_t *parser);

#endif //NADIR_PARSER_H
