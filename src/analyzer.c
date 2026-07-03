#include "nadir/analyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

nadir_analyzer_error_t nadir_analyzer_run_constant(const nadir_analyzer_t *analyzer,
                                                   const nadir_ast_declaration_constant_t *declaration);

nadir_analyzer_error_t nadir_analyzer_run_procedure(const nadir_analyzer_t *analyzer,
                                                    const nadir_ast_declaration_procedure_t *declaration);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_analyzer_t *nadir_analyzer_new(nadir_ast_t *ast) {
    nadir_analyzer_t *analyzer = malloc(sizeof(nadir_analyzer_t));
    if (analyzer == nullptr) {
        return nullptr;
    }

    const auto constants = nadir_table_new(sizeof(nadir_analyzer_constant_t));
    if (constants == nullptr) {
        free(analyzer);
        return nullptr;
    }

    const auto procedures = nadir_table_new(sizeof(nadir_analyzer_procedure_t));
    if (procedures == nullptr) {
        nadir_table_free(constants);
        free(analyzer);
        return nullptr;
    }

    analyzer->ast = ast;
    analyzer->constants = constants;
    analyzer->procedures = procedures;

    return analyzer;
}

nadir_analyzer_error_t nadir_analyzer_run(const nadir_analyzer_t *analyzer) {
    auto error = (nadir_analyzer_error_t){};

    if (analyzer->ast->declarations->length == 0) {
        return nadir_analyzer_error_new(NADIR_ANALYZER_ERROR_KIND_EMPTY, nullptr);
    }

    for (nadir_u64_t index = 0; index < analyzer->ast->declarations->length; ++index) {
        const auto declaration = (nadir_ast_declaration_t *) nadir_list_get(analyzer->ast->declarations, index);
        switch (declaration->kind) {
            case NADIR_AST_DECLARATION_KIND_CONSTANT:
                error = nadir_analyzer_run_constant(analyzer, &declaration->data.constant);
                break;
            case NADIR_AST_DECLARATION_KIND_PROCEDURE:
                error = nadir_analyzer_run_procedure(analyzer, &declaration->data.procedure);
                break;
            default:
                break;
        }

        if (error.kind != NADIR_ANALYZER_ERROR_KIND_NONE) {
            return error;
        }
    }

    return error;
}

void nadir_analyzer_free(nadir_analyzer_t *analyzer) {
    if (analyzer == nullptr) {
        return;
    }

    nadir_table_free(analyzer->constants);
    nadir_table_free(analyzer->procedures);
    free(analyzer);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

nadir_analyzer_error_t nadir_analyzer_run_constant(const nadir_analyzer_t *analyzer,
                                                   const nadir_ast_declaration_constant_t *declaration) {
    const auto object = declaration->name->value;
    for (nadir_u64_t index = 0; index < declaration->entries->length; ++index) {
        const auto entry = (nadir_ast_constant_entry_t *) nadir_list_get(declaration->entries, index);
        const auto field = entry->name->value;

        auto constant = (nadir_analyzer_constant_t){
            .token = entry->name,
            .value = &entry->value,
        };

        char member[NADIR_STRING_MAXIMUM] = {0};
        sprintf(member, "%s.%s", object, field);

        if (!nadir_table_insert(analyzer->constants, member, &constant)) {
            return nadir_analyzer_error_new(NADIR_ANALYZER_ERROR_KIND_OUT_OF_MEMORY, entry->name);
        }
    }

    return (nadir_analyzer_error_t){};
}

nadir_analyzer_error_t nadir_analyzer_run_procedure(const nadir_analyzer_t *analyzer,
                                                    const nadir_ast_declaration_procedure_t *declaration) {
    auto procedure = (nadir_analyzer_procedure_t){
        .token = declaration->name,
        .parameters = declaration->parameters,
        .statements = declaration->statements,
    };

    const auto name = declaration->name->value;
    if (!nadir_table_insert(analyzer->procedures, name, &procedure)) {
        return nadir_analyzer_error_new(NADIR_ANALYZER_ERROR_KIND_OUT_OF_MEMORY, declaration->name);
    }

    return (nadir_analyzer_error_t){};
}
