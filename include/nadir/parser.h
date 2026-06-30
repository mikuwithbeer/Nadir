#ifndef NADIR_PARSER_H
#define NADIR_PARSER_H

#include "nadir/ast.h"

/**
 * @brief Error kinds for the parser.
 */
typedef enum : nadir_u8_t {
    NADIR_PARSER_ERROR_KIND_NONE,
    NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN,
    NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF,
    NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY,
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

    nadir_list_t *tokens;
    nadir_u64_t token_index;
} nadir_parser_t;

/**
 * @brief Creates a new lexer error with the given parameters.
 */
static inline nadir_parser_error_t nadir_parser_error_new(const nadir_parser_error_kind_t id,
                                                          nadir_token_t *token) {
    return (nadir_parser_error_t){
        .kind = id,
        .token = token,
    };
}

/**
 * @brief Creates a new parser with the given tokens.
 */
nadir_parser_t *nadir_parser_new(nadir_list_t *tokens);

/**
 * @brief Peeks at the next token in the parser without advancing the token index.
 */
[[nodiscard]] nadir_token_t *nadir_parser_peek(const nadir_parser_t *parser);

/**
 * @brief Advances the parser to the next token and returns the current token.
 */
[[nodiscard]] nadir_token_t *nadir_parser_advance(nadir_parser_t *parser);

/**
 * @brief Consumes a token of the given kind from the parser.
 */
nadir_parser_error_t nadir_parser_consume(nadir_parser_t *parser,
                                          nadir_token_kind_t kind,
                                          nadir_token_t **output);

/**
 * @brief Runs the parser on the given tokens.
 */
nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser);

/**
 * @brief Frees the parser and its associated resources.
 */
void nadir_parser_free(nadir_parser_t *parser);

#endif //NADIR_PARSER_H
