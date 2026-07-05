/**
 * @file compiler.c
 * @brief The compiler implementation.
 */

#include "nadir/comptime.h"

#include <stdio.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                       const nadir_ast_declaration_constant_t *declaration);

nadir_compiler_error_t nadir_compiler_prepare_procedure(const nadir_compiler_t *compiler,
                                                        const nadir_ast_declaration_procedure_t *declaration);

nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                     const nadir_ast_declaration_binary_t *declaration);

nadir_compiler_error_t nadir_compiler_run_procedure(nadir_compiler_t *compiler,
                                                    const nadir_ast_expression_t *expression,
                                                    const nadir_compiler_procedure_t *procedure);

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_list_t *context,
                                               const nadir_ast_expression_t *expression);

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_list_t *context,
                                                        const nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Inline Functions                                             < //
// [--------------------------------------------------------------] //

static inline nadir_compiler_error_t nadir_compiler_stack_push(const nadir_compiler_t *compiler,
                                                               const nadir_i128_t value,
                                                               nadir_token_t *token) {
    if (!nadir_stack_push(compiler->stack, value)) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, token);
    }

    return (nadir_compiler_error_t){};
}

static inline nadir_compiler_error_t nadir_compiler_stack_pop(const nadir_compiler_t *compiler,
                                                              nadir_i128_t *value,
                                                              nadir_token_t *token) {
    if (!nadir_stack_pop(compiler->stack, value)) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, token);
    }

    return (nadir_compiler_error_t){};
}

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_compiler_t *nadir_compiler_new(nadir_ast_t *ast) {
    nadir_compiler_t *compiler = malloc(sizeof(nadir_compiler_t));
    if (compiler == nullptr) {
        return nullptr;
    }

    const auto addresses = nadir_table_new(sizeof(nadir_u64_t));
    if (addresses == nullptr) {
        free(compiler);
        return nullptr;
    }

    const auto constants = nadir_table_new(sizeof(nadir_compiler_constant_t));
    if (constants == nullptr) {
        nadir_table_free(addresses);
        free(compiler);
        return nullptr;
    }

    const auto procedures = nadir_table_new(sizeof(nadir_compiler_procedure_t));
    if (procedures == nullptr) {
        nadir_table_free(constants);
        nadir_table_free(addresses);
        free(compiler);
        return nullptr;
    }

    const auto stack = nadir_stack_new();
    if (stack == nullptr) {
        nadir_table_free(procedures);
        nadir_table_free(constants);
        nadir_table_free(addresses);
        free(compiler);
        return nullptr;
    }

    const auto output = nadir_list_new(sizeof(nadir_u8_t));
    if (output == nullptr) {
        nadir_stack_free(stack);
        nadir_table_free(procedures);
        nadir_table_free(constants);
        nadir_table_free(addresses);
        free(compiler);
        return nullptr;
    }

    compiler->ast = ast;

    compiler->addresses = addresses;
    compiler->constants = constants;
    compiler->procedures = procedures;

    compiler->stack = stack;
    compiler->output = output;

    compiler->binary_location = (nadir_u64_t) -1;
    compiler->binary_origin = 0;

    return compiler;
}

nadir_compiler_error_t nadir_compiler_prepare(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};

    // Guard against an empty abstract syntax tree.
    if (compiler->ast->declarations->length == 0) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_EMPTY, nullptr);
    }

    // Prepare each declaration in the abstract syntax tree.
    for (nadir_u64_t index = 0; index < compiler->ast->declarations->length; ++index) {
        const nadir_ast_declaration_t *declaration = nadir_list_get(compiler->ast->declarations, index);
        switch (declaration->kind) {
            case NADIR_AST_DECLARATION_KIND_CONSTANT:
                error = nadir_compiler_prepare_constant(compiler, &declaration->constant);
                break;
            case NADIR_AST_DECLARATION_KIND_PROCEDURE:
                error = nadir_compiler_prepare_procedure(compiler, &declaration->procedure);
                break;
            case NADIR_AST_DECLARATION_KIND_BINARY:
                compiler->binary_location = index;
                break;
        }

        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }
    }

    // Guard against an undefined binary declaration.
    if (compiler->binary_location == (nadir_u64_t) -1) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY, nullptr);
    }

    const nadir_ast_declaration_t *binary = nadir_list_get(compiler->ast->declarations, compiler->binary_location);
    return nadir_compiler_prepare_binary(compiler, &binary->binary);
}

nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};
    const nadir_ast_declaration_t *binary = nadir_list_get(compiler->ast->declarations, compiler->binary_location);

    // Run each statement in the binary declaration to generate the output.
    for (nadir_u64_t index = 0; index < binary->binary.statements->length; ++index) {
        const nadir_ast_expression_t *statement = nadir_list_get(binary->binary.statements, index);
        if (statement->kind != NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
            continue; // Addresses are handled in the preparation phase
        }

        // Guaranteed to be a procedure call due to the validation in the parser.
        const nadir_compiler_procedure_t *procedure = nadir_table_fetch(compiler->procedures, statement->token->value);
        if (procedure == nullptr) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
        }

        error = nadir_compiler_run_procedure(compiler, statement, procedure);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }
    }

    return error;
}

void nadir_compiler_free(nadir_compiler_t *compiler) {
    if (compiler == nullptr) {
        return;
    }

    if (compiler->output != nullptr) {
        nadir_list_free(compiler->output);
        compiler->output = nullptr;
    }

    if (compiler->stack != nullptr) {
        nadir_stack_free(compiler->stack);
        compiler->stack = nullptr;
    }

    if (compiler->procedures != nullptr) {
        nadir_table_free(compiler->procedures);
        compiler->procedures = nullptr;
    }

    if (compiler->constants != nullptr) {
        nadir_table_free(compiler->constants);
        compiler->constants = nullptr;
    }

    if (compiler->addresses != nullptr) {
        nadir_table_free(compiler->addresses);
        compiler->addresses = nullptr;
    }

    free(compiler);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                       const nadir_ast_declaration_constant_t *declaration) {
    auto error = (nadir_compiler_error_t){};

    // Prepare each constant entry in the constant declaration.
    const auto member_first = declaration->name->value;
    for (nadir_u64_t index = 0; index < declaration->entries->length; ++index) {
        const nadir_ast_declaration_constant_entry_t *constant_entry = nadir_list_get(declaration->entries, index);
        const auto member_second = constant_entry->name->value;

        // Evaluate the constant entry expression to get its value.
        error = nadir_compiler_evaluate(compiler, nullptr, &constant_entry->value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Pop the constant entry value from the stack.
        nadir_i128_t constant_value;
        error = nadir_compiler_stack_pop(compiler, &constant_value, constant_entry->name);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Create a unique key for the constant entry.
        char member_key[NADIR_STRING_MAXIMUM] = {};
        sprintf(member_key, "%s.%s", member_first, member_second);

        const auto constant = (nadir_compiler_constant_t){
            .token = constant_entry->name,
            .value = constant_value,
        };

        if (!nadir_table_insert(compiler->constants, member_key, &constant)) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT, constant_entry->name);
        }
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_prepare_procedure(const nadir_compiler_t *compiler,
                                                        const nadir_ast_declaration_procedure_t *declaration) {
    const auto procedure = (nadir_compiler_procedure_t){
        .token = declaration->name,
        .parameters = declaration->parameters,
        .statements = declaration->statements,
    };

    if (!nadir_table_insert(compiler->procedures, declaration->name->value, &procedure)) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_PROCEDURE, declaration->name);
    }

    return (nadir_compiler_error_t){};
}

nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                     const nadir_ast_declaration_binary_t *declaration) {
    compiler->binary_origin = declaration->origin;

    // Prepare each statement in the binary declaration to calculate the binary origin.
    for (nadir_u64_t index = 0; index < declaration->statements->length; ++index) {
        const nadir_ast_expression_t *statement = nadir_list_get(declaration->statements, index);

        // Handle address statements.
        if (statement->kind == NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS) {
            if (!nadir_table_insert(compiler->addresses, statement->token->value, &compiler->binary_origin)) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS, statement->token);
            }

            continue;
        }

        // Guaranteed to be a procedure call due to the validation in the parser.
        const nadir_compiler_procedure_t *procedure = nadir_table_fetch(compiler->procedures, statement->token->value);
        if (procedure == nullptr) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
        }

        compiler->binary_origin += procedure->statements->length;
    }

    return (nadir_compiler_error_t){};
}

nadir_compiler_error_t nadir_compiler_run_procedure(nadir_compiler_t *compiler,
                                                    const nadir_ast_expression_t *expression,
                                                    const nadir_compiler_procedure_t *procedure) {
    auto error = (nadir_compiler_error_t){};

    // Guard against an argument mismatch between the parameters and the arguments.
    if (procedure->parameters->length != expression->call.arguments->length) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH, expression->token);
    }

    // Create a new context for the procedure call to hold the argument values.
    // This context will be freed if an error occurs.
    const auto context = nadir_list_new(sizeof(nadir_i128_t));
    if (context == nullptr) {
        error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        goto cleanup;
    }

    // Process each argument and validate its type against.
    for (nadir_u64_t index = 0; index < procedure->parameters->length; ++index) {
        const nadir_ast_expression_t *procedure_argument = nadir_list_get(expression->call.arguments, index);

        // Evaluate the argument expression to get its value.
        error = nadir_compiler_evaluate(compiler, nullptr, procedure_argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        // Pop the argument value from the stack.
        nadir_i128_t argument_value;
        error = nadir_compiler_stack_pop(compiler, &argument_value, procedure_argument->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        // Validate the argument type against the expected parameter type.
        bool type_mismatch = false;
        switch (*(nadir_token_kind_t *) nadir_list_get(procedure->parameters, index)) {
            case NADIR_TOKEN_KIND_TYPE_U8:
                type_mismatch = argument_value != (nadir_u8_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_U16:
                type_mismatch = argument_value != (nadir_u16_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_U32:
                type_mismatch = argument_value != (nadir_u32_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_U64:
                type_mismatch = argument_value != (nadir_u64_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_I8:
                type_mismatch = argument_value != (nadir_i8_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_I16:
                type_mismatch = argument_value != (nadir_i16_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_I32:
                type_mismatch = argument_value != (nadir_i32_t) argument_value;
                break;
            case NADIR_TOKEN_KIND_TYPE_I64:
                type_mismatch = argument_value != (nadir_i64_t) argument_value;
                break;
            default:
                break; // Unreachable
        }

        if (type_mismatch) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, procedure_argument->token);
            goto cleanup;
        }

        if (!nadir_list_append(context, &argument_value)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
            goto cleanup;
        }
    }

    // Evaluate each statement in the procedure.
    for (nadir_u64_t index = 0; index < procedure->statements->length; ++index) {
        const nadir_ast_expression_t *statement = nadir_list_get(procedure->statements, index);

        // Evaluate the statement expression in the context of the procedure call.
        error = nadir_compiler_evaluate(compiler, context, statement);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        // Pop the statement value from the stack.
        nadir_i128_t statement_value;
        error = nadir_compiler_stack_pop(compiler, &statement_value, statement->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        // Guard against a byte mismatch when writing the statement value to the output.
        nadir_u8_t statement_byte = (nadir_u8_t) statement_value;
        if (statement_byte != statement_value) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH, statement->token);
            goto cleanup;
        }

        if (!nadir_list_append(compiler->output, &statement_byte)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
            goto cleanup;
        }
    }

cleanup:
    if (context != nullptr) {
        nadir_list_free(context);
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_list_t *context,
                                               const nadir_ast_expression_t *expression) {
    auto error = (nadir_compiler_error_t){};

    // Evaluate the expression based on its kind and push the result onto the stack.
    switch (expression->kind) {
        case NADIR_AST_EXPRESSION_KIND_NUMBER:
            // Directly push the number value onto the stack.
            error = nadir_compiler_stack_push(compiler, expression->token->number, expression->token);
            break;
        case NADIR_AST_EXPRESSION_KIND_TYPE:
            // Convert the token kind to the value.
            const nadir_type_t type_value = expression->token->kind - NADIR_TOKEN_KIND_TYPE_U8;
            error = nadir_compiler_stack_push(compiler, type_value, expression->token);
            break;
        case NADIR_AST_EXPRESSION_KIND_MEMBER: {
            // Format the member key to look up the constant value.
            char member_key[NADIR_STRING_MAXIMUM] = {};
            sprintf(member_key, "%s.%s", expression->token->value, expression->member.field->value);

            // Look up the constant value in the compiler's constant table.
            const nadir_compiler_constant_t *constant = nadir_table_fetch(compiler->constants, member_key);
            if (constant == nullptr) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT,
                                                expression->member.field);
            }

            error = nadir_compiler_stack_push(compiler, constant->value, expression->token);
            break;
        }
        case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            // Evaluate the compile-time call with the provided context and arguments.
            error = nadir_compiler_evaluate_comptime(compiler, context, expression);
            break;
        case NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS: {
            // Look up the address in the compiler's address table.
            const nadir_u64_t *address = nadir_table_fetch(compiler->addresses, expression->token->value);
            if (address == nullptr) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS, expression->token);
            }

            error = nadir_compiler_stack_push(compiler, *address, expression->token);
            break;
        }
        default:
            break; // Unreachable
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_list_t *context,
                                                        const nadir_ast_expression_t *expression) {
    nadir_compiler_error_t error;

    // Create a new list to hold the evaluated argument values for the compile-time call.
    // This list will be freed if an error occurs.
    const auto arguments = nadir_list_new(sizeof(nadir_i128_t));
    if (arguments == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
    }

    // Evaluate each argument expression and store its value in the argument list.
    for (nadir_u64_t index = 0; index < expression->call.arguments->length; ++index) {
        const nadir_ast_expression_t *argument = nadir_list_get(expression->call.arguments, index);

        // Evaluate the argument expression with the context.
        error = nadir_compiler_evaluate(compiler, context, argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        // Pop the argument value from the stack.
        nadir_i128_t argument_value;
        error = nadir_compiler_stack_pop(compiler, &argument_value, argument->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        if (!nadir_list_append(arguments, &argument_value)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
            goto cleanup;
        }
    }

    // Determine the compile-time kind.
    const auto kind = nadir_comptime_kind(expression->token->value);
    if (kind == NADIR_COMPTIME_KIND_NONE) {
        error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME, expression->token);
        goto cleanup;
    }

    const auto comptime = (nadir_comptime_t){
        .kind = kind,
        .arguments = arguments,
    };

    // Evaluate the compile-time call with the provided context and arguments.
    nadir_i128_t comptime_result;
    error = nadir_comptime_run(&comptime, context, &comptime_result);
    if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        // Propagate the error with the expression's token.
        error.token = expression->token;
        goto cleanup;
    }

    error = nadir_compiler_stack_push(compiler, comptime_result, expression->token);

cleanup:
    if (arguments != nullptr) {
        nadir_list_free(arguments);
    }

    return error;
}
