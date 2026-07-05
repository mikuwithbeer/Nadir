/**
 * @file ast.c
 * @brief The abstract syntax tree implementation.
 */

#include "nadir/ast.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static void nadir_ast_free_expression(const nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_ast_t *nadir_ast_new(void) {
    nadir_ast_t *ast = malloc(sizeof(nadir_ast_t));
    if (ast == nullptr) {
        return nullptr;
    }

    const auto declarations = nadir_list_new(sizeof(nadir_ast_declaration_t));
    if (declarations == nullptr) {
        free(ast);
        return nullptr;
    }

    ast->declarations = declarations;
    return ast;
}

void nadir_ast_free(nadir_ast_t *ast) {
    if (ast == nullptr) {
        return;
    }

    if (ast->declarations != nullptr) {
        // Free each declaration in the abstract syntax tree.
        for (nadir_u64_t index = 0; index < ast->declarations->length; ++index) {
            const auto declaration = (nadir_ast_declaration_t *) nadir_list_get(ast->declarations, index);

            if (declaration->kind == NADIR_AST_DECLARATION_KIND_CONSTANT) {
                const auto entries = declaration->constant.entries;
                for (nadir_u64_t inner = 0; inner < entries->length; ++inner) {
                    const auto entry = (nadir_ast_declaration_constant_entry_t *) nadir_list_get(entries, inner);
                    nadir_ast_free_expression(&entry->value);
                }

                nadir_list_free(entries);
            } else if (declaration->kind == NADIR_AST_DECLARATION_KIND_PROCEDURE) {
                nadir_list_free(declaration->procedure.parameters);

                const auto statements = declaration->procedure.statements;
                for (nadir_u64_t inner = 0; inner < statements->length; ++inner) {
                    const auto statement = (nadir_ast_expression_t *) nadir_list_get(statements, inner);
                    nadir_ast_free_expression(statement);
                }

                nadir_list_free(statements);
            } else if (declaration->kind == NADIR_AST_DECLARATION_KIND_BINARY) {
                const auto statements = declaration->binary.statements;
                for (nadir_u64_t inner = 0; inner < statements->length; ++inner) {
                    const auto statement = (nadir_ast_expression_t *) nadir_list_get(statements, inner);
                    nadir_ast_free_expression(statement);
                }

                nadir_list_free(statements);
            }
        }

        nadir_list_free(ast->declarations);
        ast->declarations = nullptr;
    }

    free(ast);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static void nadir_ast_free_expression(const nadir_ast_expression_t *expression) {
    if (expression == nullptr) {
        return;
    }

    if (expression->kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
        expression->kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
        const auto arguments = expression->call.arguments;
        if (arguments != nullptr) {
            for (nadir_u64_t index = 0; index < arguments->length; ++index) {
                const auto argument = nadir_list_get(arguments, index);
                nadir_ast_free_expression(argument);
            }

            nadir_list_free(expression->call.arguments);
        }
    }
}
