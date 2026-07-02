#include "nadir/ast.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static void nadir_ast_free_expression(nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_ast_t *nadir_ast_new(void) {
    nadir_ast_t *ast = malloc(sizeof(nadir_ast_t));
    if (ast == nullptr) {
        return nullptr;
    }

    ast->declarations = nadir_list_new(sizeof(nadir_ast_declaration_t));
    if (ast->declarations == nullptr) {
        free(ast);
        return nullptr;
    }

    return ast;
}

void nadir_ast_free(nadir_ast_t *ast) {
    if (ast == nullptr) {
        return;
    }

    for (nadir_u64_t index = 0; index < ast->declarations->length; ++index) {
        const auto declaration = (nadir_ast_declaration_t *) nadir_list_get(ast->declarations, index);

        if (declaration->kind == NADIR_AST_DECLARATION_KIND_CONSTANT) {
            const auto entries = declaration->data.constant.entries;
            for (nadir_u64_t inner = 0; inner < entries->length; ++inner) {
                const auto entry = (nadir_ast_const_entry_t *) nadir_list_get(entries, inner);
                nadir_ast_free_expression(&entry->value);
            }

            nadir_list_free(entries);
        } else if (declaration->kind == NADIR_AST_DECLARATION_KIND_PROCEDURE) {
            nadir_list_free(declaration->data.procedure.parameters);

            const auto statements = declaration->data.procedure.statements;
            for (nadir_u64_t inner = 0; inner < statements->length; ++inner) {
                const auto statement = (nadir_ast_expression_t *) nadir_list_get(statements, inner);
                nadir_ast_free_expression(statement);
            }

            nadir_list_free(statements);
        }
    }

    nadir_list_free(ast->declarations);
    free(ast);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static void nadir_ast_free_expression(nadir_ast_expression_t *expression) {
    if (!expression) {
        return;
    }

    if (expression->kind == NADIR_AST_EXPRESSION_KIND_BUILTIN_CALL ||
        expression->kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
        const auto arguments = expression->data.call.arguments;
        if (arguments != nullptr) {
            for (nadir_u64_t index = 0; index < arguments->length; ++index) {
                const auto argument = nadir_list_get(arguments, index);
                nadir_ast_free_expression(argument);
            }

            nadir_list_free(expression->data.call.arguments);
        }
    }
}
