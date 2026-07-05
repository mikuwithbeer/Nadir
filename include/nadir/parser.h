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

constexpr auto NADIR_PARSER_ARGUMENTS_MAXIMUM = 1 << 4; // 16 arguments per procedure

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
    NADIR_PARSER_ERROR_KIND_MISSING_SEMICOLON,
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
} nadir_parser_error_t;

/**
 * @brief Parser structure for the assembler.
 */
typedef struct {
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
static inline nadir_parser_error_t nadir_parser_error_new(const nadir_parser_error_kind_t id,
                                                          nadir_token_t *token) {
    return (nadir_parser_error_t){
        .kind = id,
        .token = token,
    };
}

/**
 * @brief Creates a new parser with the given list of tokens.
 *
 * @warning Allocates memory for the parser and its associated resources.
 */
nadir_parser_t *nadir_parser_new(nadir_list_t *tokens);

/**
 * @brief Generates an abstract syntax tree from the list of tokens.
 */
nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser);

/**
 * @brief Frees the memory allocated for the parser and its associated resources.
 */
void nadir_parser_free(nadir_parser_t *parser);

#endif //NADIR_PARSER_H
