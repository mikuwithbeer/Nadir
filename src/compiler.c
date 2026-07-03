#include "nadir/compiler.h"
#include "nadir/comptime.h"

#include <stdio.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                       const nadir_ast_declaration_constant_t *declaration);

nadir_compiler_error_t nadir_compiler_prepare_procedure(nadir_compiler_t *compiler,
                                                        const nadir_ast_declaration_procedure_t *declaration);

nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                     const nadir_ast_declaration_binary_t *declaration);

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_ast_expression_t *expression);

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_compiler_t *nadir_compiler_new(nadir_ast_t *ast) {
    nadir_compiler_t *compiler = malloc(sizeof(nadir_compiler_t));
    if (compiler == nullptr) {
        return nullptr;
    }

    const auto constants = nadir_table_new(sizeof(nadir_compiler_constant_t));
    if (constants == nullptr) {
        free(compiler);
        return nullptr;
    }

    const auto addresses = nadir_table_new(sizeof(nadir_u64_t));
    if (addresses == nullptr) {
        nadir_table_free(constants);
        free(compiler);
        return nullptr;
    }

    const auto procedures = nadir_table_new(sizeof(nadir_compiler_procedure_t));
    if (procedures == nullptr) {
        nadir_table_free(addresses);
        nadir_table_free(constants);
        free(compiler);
        return nullptr;
    }

    const auto output = nadir_list_new(sizeof(nadir_u8_t));
    if (output == nullptr) {
        nadir_table_free(procedures);
        nadir_table_free(addresses);
        nadir_table_free(constants);
        free(compiler);
        return nullptr;
    }

    compiler->ast = ast;

    compiler->constants = constants;
    compiler->addresses = addresses;
    compiler->procedures = procedures;

    compiler->output = output;
    compiler->expected = 0;

    compiler->stack = nadir_stack_new(); // Initialize the stack
    return compiler;
}

nadir_compiler_error_t nadir_compiler_prepare(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};

    // Check if the abstract syntax tree has any declarations.
    if (compiler->ast->declarations->length == 0) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_EMPTY, nullptr);
    }

    // Iterate through the declarations in the abstract syntax tree and prepare constants and procedures.
    nadir_u64_t location = 0;
    for (nadir_u64_t index = 0; index < compiler->ast->declarations->length; ++index) {
        const auto declaration = (nadir_ast_declaration_t *) nadir_list_get(compiler->ast->declarations, index);
        switch (declaration->kind) {
            case NADIR_AST_DECLARATION_KIND_CONSTANT:
                error = nadir_compiler_prepare_constant(compiler, &declaration->data.constant);
                break;
            case NADIR_AST_DECLARATION_KIND_PROCEDURE:
                error = nadir_compiler_prepare_procedure(compiler, &declaration->data.procedure);
                break;
            case NADIR_AST_DECLARATION_KIND_BINARY:
                location = index;
                break;
        }

        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }
    }

    // Prepare the binary declaration after all constants and procedures have been prepared.
    const auto binary = (nadir_ast_declaration_t *) nadir_list_get(compiler->ast->declarations, location);
    error = nadir_compiler_prepare_binary(compiler, &binary->data.binary);

    return error;
}

nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};
    return error;
}

void nadir_compiler_free(nadir_compiler_t *compiler) {
    if (compiler == nullptr) {
        return;
    }

    nadir_list_free(compiler->output);

    nadir_table_free(compiler->constants);
    nadir_table_free(compiler->addresses);
    nadir_table_free(compiler->procedures);

    free(compiler);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                       const nadir_ast_declaration_constant_t *declaration) {
    auto error = (nadir_compiler_error_t){};

    const auto first = declaration->name->value;
    for (nadir_u64_t index = 0; index < declaration->entries->length; ++index) {
        const auto const_entry = (nadir_ast_constant_entry_t *) nadir_list_get(declaration->entries, index);
        const auto second = const_entry->name->value;

        // Evaluate the constant entry's value.
        error = nadir_compiler_evaluate(compiler, &const_entry->value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Pop the evaluated value from the stack.
        nadir_i128_t value;
        if (!nadir_stack_pop(&compiler->stack, &value)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, const_entry->name);
            break;
        }

        // Create a unique key for the constant entry.
        char member_key[NADIR_STRING_MAXIMUM] = {};
        sprintf(member_key, "%s.%s", first, second);

        const auto constant = (nadir_compiler_constant_t){
            .token = const_entry->name,
            .value = value,
        };

        if (!nadir_table_insert(compiler->constants, member_key, &constant)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, const_entry->name);
        }
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_prepare_procedure(nadir_compiler_t *compiler,
                                                        const nadir_ast_declaration_procedure_t *declaration) {
    const auto procedure = (nadir_compiler_procedure_t){
        .token = declaration->name,
        .parameters = declaration->parameters,
        .statements = declaration->statements,
    };

    const auto name = declaration->name->value;
    if (!nadir_table_insert(compiler->procedures, name, &procedure)) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, declaration->name);
    }

    return (nadir_compiler_error_t){};
}

nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                     const nadir_ast_declaration_binary_t *declaration) {
    for (nadir_u64_t index = 0; index < declaration->statements->length; ++index) {
        const auto statement = (nadir_ast_expression_t *) nadir_list_get(declaration->statements, index);

        if (statement->kind == NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS) {
            const auto address_name = statement->token->value;
            if (!nadir_table_insert(compiler->addresses, address_name, &compiler->expected)) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TABLE_FAILED, statement->token);
            }

            continue;
        }

        const auto procedure_name = statement->token->value;
        const auto procedure = (nadir_compiler_procedure_t *) nadir_table_fetch(compiler->procedures, procedure_name);
        if (procedure == nullptr) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
        }

        compiler->expected += procedure->statements->length;
    }

    return (nadir_compiler_error_t){};
}

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_ast_expression_t *expression) {
    auto error = (nadir_compiler_error_t){};

    // Evaluate the expression based on its kind.
    switch (expression->kind) {
        case NADIR_AST_EXPRESSION_KIND_NUMBER:
            if (!nadir_stack_push(&compiler->stack, expression->token->specific.number)) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
            }

            break;
        case NADIR_AST_EXPRESSION_KIND_TYPE:
            if (!nadir_stack_push(&compiler->stack, expression->token->kind - NADIR_TOKEN_KIND_TYPE_U8)) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
            }

            break;
        case NADIR_AST_EXPRESSION_KIND_MEMBER:
            // Constructing a unique key.
            char member_key[NADIR_STRING_MAXIMUM] = {};
            sprintf(member_key, "%s.%s", expression->token->value, expression->data.member.field->value);

            // Fetching the constant from the constant table.
            const auto result = (nadir_compiler_constant_t *) nadir_table_fetch(compiler->constants, member_key);
            if (result == nullptr) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT, expression->token);
                break;
            }

            if (!nadir_stack_push(&compiler->stack, result->value)) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
            }

            break;
        case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            error = nadir_compiler_evaluate_comptime(compiler, expression);
            break;
        case NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL:
            break;
        case NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS:
            break;
        case NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS:
            break;
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_ast_expression_t *expression) {
    auto error = (nadir_compiler_error_t){};

    // Should be freed if an error occurs before the arguments are used in the compile-time evaluation.
    const auto arguments = nadir_list_new(sizeof(nadir_i128_t));
    if (arguments == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
    }

    // Evaluate each argument of the compile-time call expression.
    for (nadir_u64_t index = 0; index < expression->data.call.arguments->length; ++index) {
        const auto argument = (nadir_ast_expression_t *) nadir_list_get(expression->data.call.arguments, index);

        // Evaluate the argument expression.
        error = nadir_compiler_evaluate(compiler, argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            nadir_list_free(arguments);
            return error;
        }

        // Pop the evaluated argument value from the stack.
        nadir_i128_t value;
        if (!nadir_stack_pop(&compiler->stack, &value)) {
            nadir_list_free(arguments);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
        }

        if (!nadir_list_append(arguments, &value)) {
            nadir_list_free(arguments);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        }
    }

    // Determine the compile-time kind based on the expression's token value.
    const auto kind = nadir_comptime_kind(expression->token->value);
    if (kind == NADIR_COMPTIME_KIND_NONE) {
        nadir_list_free(arguments);
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME, expression->token);
    }

    const auto comptime = (nadir_comptime_t){
        .kind = kind,
        .arguments = arguments,
    };

    // Evaluate the compile-time procedure with the given arguments.
    nadir_i128_t result;
    if (!nadir_comptime_run(&comptime, &result)) {
        nadir_list_free(arguments);
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_COMPTIME_FAILED, expression->token);
    }

    if (!nadir_stack_push(&compiler->stack, result)) {
        nadir_list_free(arguments);
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
    }

    nadir_list_free(arguments); // Free the argument list
    return error;
}
