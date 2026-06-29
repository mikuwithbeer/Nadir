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

bool nadir_ast_append_declaration(const nadir_ast_t *ast,
                                  const nadir_ast_declaration_t *declaration) {
    return nadir_list_append(ast->declarations, declaration);
}

void nadir_ast_free(nadir_ast_t *ast) {
    // TODO: this leaks memory, will be fixed after fully implemented parser.

    if (ast == nullptr) {
        return;
    }

    nadir_list_free(ast->declarations);
    free(ast);
}
