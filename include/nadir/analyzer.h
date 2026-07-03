#ifndef NADIR_ANALYZER_H
#define NADIR_ANALYZER_H

#include "nadir/ast.h"

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Error kinds for the analyzer.
 */
typedef enum [[nodiscard]] : nadir_u8_t {
    NADIR_ANALYZER_ERROR_KIND_NONE,
    NADIR_ANALYZER_ERROR_KIND_EMPTY,
    NADIR_ANALYZER_ERROR_KIND_OUT_OF_MEMORY,
    NADIR_ANALYZER_ERROR_KIND_TABLE_FAILED,
} nadir_analyzer_error_kind_t;

/**
 * @brief Error structure for the analyzer.
 */
typedef struct [[nodiscard]] {
    nadir_analyzer_error_kind_t kind;
    nadir_token_t *token;
} nadir_analyzer_error_t;

/**
 * @brief Analyzer structure for a constant declaration.
 */
typedef struct {
    nadir_token_t *token;

    nadir_ast_expression_t *value;
} nadir_analyzer_constant_t;

/**
 * @brief Analyzer structure for a procedure declaration.
 */
typedef struct {
    nadir_token_t *token;

    nadir_list_t *parameters;
    nadir_list_t *statements;
} nadir_analyzer_procedure_t;

/**
 * @brief Analyzer structure for the assembler.
 */
typedef struct {
    nadir_ast_t *ast;

    nadir_table_t *constants;
    nadir_table_t *procedures;
} nadir_analyzer_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new analyzer error with the given parameters.
 */
static inline nadir_analyzer_error_t nadir_analyzer_error_new(const nadir_analyzer_error_kind_t kind,
                                                              nadir_token_t *token) {
    return (nadir_analyzer_error_t){
        .kind = kind,
        .token = token,
    };
}

/**
 * @brief Creates a new analyzer with the given abstract syntax tree.
 */
[[nodiscard]] nadir_analyzer_t *nadir_analyzer_new(nadir_ast_t *ast);

/**
 * @brief Runs the analyzer on the abstract syntax tree.
 */
nadir_analyzer_error_t nadir_analyzer_run(const nadir_analyzer_t *analyzer);

/**
 * @brief Frees the analyzer and its resources.
 */
void nadir_analyzer_free(nadir_analyzer_t *analyzer);

#endif //NADIR_ANALYZER_H
