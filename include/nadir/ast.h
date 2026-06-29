#ifndef NADIR_AST_H
#define NADIR_AST_H

#include "nadir/token.h"

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Expression kinds for the abstract syntax tree.
 */
typedef enum : nadir_u8_t {
    NADIR_AST_EXPRESSION_KIND_NUMBER,
    NADIR_AST_EXPRESSION_KIND_TYPE,
    NADIR_AST_EXPRESSION_KIND_MEMBER,
    NADIR_AST_EXPRESSION_KIND_BUILTIN_CALL,
    NADIR_AST_EXPRESSION_KIND_PROC_CALL
} nadir_ast_expression_kind_t;

/**
 * @brief Expression structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *token;
    nadir_ast_expression_kind_t kind;

    union {
        struct {
            nadir_token_t *object;
            nadir_token_t *field;
        } member;

        struct {
            nadir_token_t *name;
            nadir_list_t *arguments;
        } call;
    } data;
} nadir_ast_expression_t;

/**
 * @brief Constant entry structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_ast_expression_t value;
} nadir_ast_const_entry_t;

/**
 * @brief Constant declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_list_t *entries;
} nadir_ast_const_declaration_t;

/**
 * @brief Procedure declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;

    nadir_list_t *params;
    nadir_list_t *body;
} nadir_ast_procedure_declaration_t;

/**
 * @brief Declaration kinds for the abstract syntax tree.
 */
typedef enum : nadir_u8_t {
    NADIR_AST_DECLARATION_KIND_CONSTANT,
    NADIR_AST_DECLARATION_KIND_PROCEDURE
} nadir_ast_declaration_kind_t;

/**
 * @brief Declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *token;
    nadir_ast_declaration_kind_t kind;

    union {
        nadir_ast_const_declaration_t constant;
        nadir_ast_procedure_declaration_t procedure;
    } data;
} nadir_ast_declaration_t;

/**
 * @brief Abstract syntax tree structure for the assembler.
 */
typedef struct {
    nadir_list_t *declarations;
} nadir_ast_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new abstract syntax tree.
 */
[[nodiscard]] nadir_ast_t *nadir_ast_new(void);

/**
 * @brief Appends a declaration to the abstract syntax tree.
 *
 * @return false if the reallocation fails, true otherwise.
 */
[[nodiscard]] bool nadir_ast_append_declaration(const nadir_ast_t *ast,
                                                const nadir_ast_declaration_t *declaration);

/**
 * @brief Frees the memory allocated for the abstract syntax tree.
 */
void nadir_ast_free(nadir_ast_t *ast);

#endif //NADIR_AST_H
