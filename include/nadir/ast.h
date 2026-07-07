#ifndef NADIR_AST_H
#define NADIR_AST_H

/**
 * @file ast.h
 * @brief The abstract syntax tree interface.
 *
 * Tree representation of the abstract syntactic structure of source code.
 * Each node of the tree denotes a construct occurring in the source code.
 */

#include "nadir/common/list.h"
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
    NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL,
    NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL,
    NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS,
    NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS,
} nadir_ast_expression_kind_t;

/**
 * @brief Declaration kinds for the abstract syntax tree.
 */
typedef enum : nadir_u8_t {
    NADIR_AST_DECLARATION_KIND_CONSTANT,
    NADIR_AST_DECLARATION_KIND_PROCEDURE,
    NADIR_AST_DECLARATION_KIND_BINARY,
} nadir_ast_declaration_kind_t;

/**
 * @brief Expression structure for the abstract syntax tree.
 */
typedef struct {
    nadir_ast_expression_kind_t kind;
    nadir_token_t *token;

    union {
        // Member field access.
        struct {
            nadir_token_t *field;
        } member;

        // Procedure and comptime arguments.
        struct {
            nadir_list_t *arguments; // List of `nadir_ast_expression_t`
        } call;
    };
} nadir_ast_expression_t;

/**
 * @brief Constant entry structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_ast_expression_t value;
} nadir_ast_declaration_constant_entry_t;

/**
 * @brief Constant declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_list_t *entries; // List of `nadir_ast_declaration_constant_entry_t`
} nadir_ast_declaration_constant_t;

/**
 * @brief Procedure declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_list_t *parameters; // List of `nadir_token_kind_t`
    nadir_list_t *statements; // List of `nadir_ast_expression_t`
} nadir_ast_declaration_procedure_t;

/**
 * @brief Binary declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_u64_t origin;
    nadir_list_t *statements; // List of `nadir_ast_expression_t`
} nadir_ast_declaration_binary_t;

/**
 * @brief Declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_ast_declaration_kind_t kind;
    nadir_token_t *token;

    union {
        nadir_ast_declaration_constant_t constant;
        nadir_ast_declaration_procedure_t procedure;
        nadir_ast_declaration_binary_t binary;
    };
} nadir_ast_declaration_t;

/**
 * @brief Abstract syntax tree structure for the assembler.
 */
typedef struct {
    nadir_list_t *declarations; // List of `nadir_ast_declaration_t`
} nadir_ast_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new abstract syntax tree.
 *
 * @warning Allocates memory for the abstract syntax tree, which must be freed.
 */
[[nodiscard]] nadir_ast_t *nadir_ast_new(void);

/**
 * @brief Frees the abstract syntax tree and its associated resources.
 */
void nadir_ast_free(nadir_ast_t *ast);

#endif //NADIR_AST_H
