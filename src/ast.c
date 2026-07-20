/**
 * @file ast.c
 * @brief The abstract syntax tree implementation.
 */

#include "nadir/ast.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_ast_t *nadir_ast_new(nadir_arena_t *arena) {
    nadir_ast_t *ast = nadir_arena_allocate(arena, sizeof(nadir_ast_t));
    if (ast == nullptr) {
        return nullptr;
    }

    ast->arena = arena;

    auto const declarations = nadir_list_new(arena, sizeof(nadir_ast_declaration_t));
    if (declarations == nullptr) {
        return nullptr;
    }

    ast->declarations = declarations;
    return ast;
}

void nadir_ast_free(nadir_ast_t *ast) {
    if (ast == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    ast->declarations = nullptr;
}
