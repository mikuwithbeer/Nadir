#include "nadir/ast.h"

#include <stdlib.h>

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
    // TODO: This function leaks memory, will be fixed after fully implemented parser.
    if (ast == nullptr) {
        return;
    }

    nadir_list_free(ast->declarations);
    free(ast);
}
