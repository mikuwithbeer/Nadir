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
    nadir_token_t *token;
    nadir_ast_expression_kind_t kind;

    union {
        struct {
            nadir_token_t *field;
        } member;

        struct {
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
} nadir_ast_constant_entry_t;

/**
 * @brief Constant declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;
    nadir_list_t *entries;
} nadir_ast_declaration_constant_t;

/**
 * @brief Procedure declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *name;

    nadir_list_t *parameters;
    nadir_list_t *statements;
} nadir_ast_declaration_procedure_t;

/**
 * @brief Binary declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_u64_t origin;
    nadir_list_t *statements;
} nadir_ast_declaration_binary_t;

/**
 * @brief Declaration structure for the abstract syntax tree.
 */
typedef struct {
    nadir_token_t *token;
    nadir_ast_declaration_kind_t kind;

    union {
        nadir_ast_declaration_constant_t constant;
        nadir_ast_declaration_procedure_t procedure;
        nadir_ast_declaration_binary_t binary;
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
 * @brief Frees the memory allocated for the abstract syntax tree.
 */
void nadir_ast_free(nadir_ast_t *ast);

#endif //NADIR_AST_H
