#include "nadir/compiler.h"
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
                                                    const nadir_ast_expression_t *call,
                                                    const nadir_compiler_procedure_t *procedure);

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_list_t *context,
                                               const nadir_ast_expression_t *expression);

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_list_t *context,
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
    compiler->location = 0;

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
                compiler->location = index;
                break;
        }

        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }
    }

    // Prepare the binary declaration after all constants and procedures have been prepared.
    const auto binary = (nadir_ast_declaration_t *) nadir_list_get(compiler->ast->declarations, compiler->location);
    if (binary == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY, nullptr);
    }

    return nadir_compiler_prepare_binary(compiler, &binary->data.binary);
}

nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};

    // Directly access the binary declaration using the stored location index.
    const auto binary = (nadir_ast_declaration_t *) nadir_list_get(compiler->ast->declarations, compiler->location);

    // Run procedure calls in the binary statements.
    for (nadir_u64_t index = 0; index < binary->data.binary.statements->length; ++index) {
        // Check if the statement is a procedure call.
        const auto statement = (nadir_ast_expression_t *) nadir_list_get(binary->data.binary.statements, index);
        if (statement->kind != NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
            continue;
        }

        // Fetch the procedure from the procedure table.
        const auto procedure_name = statement->token->value;
        const auto procedure = (nadir_compiler_procedure_t *) nadir_table_fetch(compiler->procedures, procedure_name);
        if (procedure == nullptr) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
        }

        // Run the procedure with the provided arguments.
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
        error = nadir_compiler_evaluate(compiler, nullptr, &const_entry->value);
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

nadir_compiler_error_t nadir_compiler_prepare_procedure(const nadir_compiler_t *compiler,
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
    // Prepare addresses and calculate the expected output size.
    for (nadir_u64_t index = 0; index < declaration->statements->length; ++index) {
        // Check if the statement is a store address operation.
        const auto statement = (nadir_ast_expression_t *) nadir_list_get(declaration->statements, index);
        if (statement->kind == NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS) {
            const auto address_name = statement->token->value;
            if (!nadir_table_insert(compiler->addresses, address_name, &compiler->expected)) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TABLE_FAILED, statement->token);
            }

            continue;
        }

        // Otherwise statement is a procedure call, guaranteed from parsing stage.
        const auto procedure_name = statement->token->value;
        const auto procedure = (nadir_compiler_procedure_t *) nadir_table_fetch(compiler->procedures, procedure_name);
        if (procedure == nullptr) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
        }

        compiler->expected += procedure->statements->length;
    }

    return (nadir_compiler_error_t){};
}


nadir_compiler_error_t nadir_compiler_run_procedure(nadir_compiler_t *compiler,
                                                    const nadir_ast_expression_t *call,
                                                    const nadir_compiler_procedure_t *procedure) {
    auto error = (nadir_compiler_error_t){};

    // Check if the number of arguments in the call matches the number of parameters in the procedure.
    if (procedure->parameters->length != call->data.call.arguments->length) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH, call->token);
    }

    // Should be freed if an error occurs before the argument list is used.
    const auto context = nadir_list_new(sizeof(nadir_i128_t));
    if (context == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, call->token);
    }

    // Evaluate each argument in the call and check if it matches the expected parameter type.
    for (nadir_u64_t index = 0; index < procedure->parameters->length; ++index) {
        // Evaluate the argument expression.
        const auto argument = (nadir_ast_expression_t *) nadir_list_get(call->data.call.arguments, index);
        error = nadir_compiler_evaluate(compiler, nullptr, argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            nadir_list_free(context);
            return error;
        }

        // Pop the evaluated argument value from the stack.
        nadir_i128_t value;
        if (!nadir_stack_pop(&compiler->stack, &value)) {
            nadir_list_free(context);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, call->token);
        }

        // Check if the argument value matches the expected parameter type.
        const auto type = (nadir_token_kind_t *) nadir_list_get(procedure->parameters, index);
        switch (*type) {
            case NADIR_TOKEN_KIND_TYPE_U8: {
                const auto value_u8 = (nadir_u8_t) value;
                if (value != value_u8) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_U16: {
                const auto value_u16 = (nadir_u16_t) value;
                if (value != value_u16) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_U32: {
                const auto value_u32 = (nadir_u32_t) value;
                if (value != value_u32) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_U64: {
                const auto value_u64 = (nadir_u64_t) value;
                if (value != value_u64) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_I8: {
                const auto value_i8 = (nadir_i8_t) value;
                if (value != value_i8) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_I16: {
                const auto value_i16 = (nadir_i16_t) value;
                if (value != value_i16) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_I32: {
                const auto value_i32 = (nadir_i32_t) value;
                if (value != value_i32) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            case NADIR_TOKEN_KIND_TYPE_I64: {
                const auto value_i64 = (nadir_i64_t) value;
                if (value != value_i64) {
                    nadir_list_free(context);
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, call->token);
                }

                break;
            }
            default:
                // Unreachable.
                break;
        }

        if (!nadir_list_append(context, &value)) {
            nadir_list_free(context);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, call->token);
        }
    }

    // Evaluate each statement in the procedure and push the resulting values to the output list.
    for (nadir_u64_t index = 0; index < procedure->statements->length; ++index) {
        // Evaluate the statement in the context of the procedure's arguments.
        const auto statement = (nadir_ast_expression_t *) nadir_list_get(procedure->statements, index);
        error = nadir_compiler_evaluate(compiler, context, statement);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            nadir_list_free(context);
            return error;
        }

        // Pop the evaluated value from the stack.
        nadir_i128_t value;
        if (!nadir_stack_pop(&compiler->stack, &value)) {
            nadir_list_free(context);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, call->token);
        }

        // Check if the value can be represented as a byte.
        nadir_u8_t output_byte = (nadir_u8_t) value;
        if (output_byte != value) {
            nadir_list_free(context);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH, call->token);
        }

        if (!nadir_list_append(compiler->output, &value)) {
            nadir_list_free(context);
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, call->token);
        }
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                               const nadir_list_t *context,
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
            error = nadir_compiler_evaluate_comptime(compiler, context, expression);
            break;
        case NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS:
            // Fetch the address from the address table.
            const auto address_name = expression->token->value;
            const auto address = (nadir_u64_t *) nadir_table_fetch(compiler->addresses, address_name);
            if (address == nullptr) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS, expression->token);
                break;
            }

            if (!nadir_stack_push(&compiler->stack, *address)) {
                error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_STACK_FAILED, expression->token);
            }

            break;
        default:
            // Unreachable.
            break;
    }

    return error;
}

nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                        const nadir_list_t *context,
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
        error = nadir_compiler_evaluate(compiler, context, argument);
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
    if (!nadir_comptime_run(&comptime, context, &result)) {
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
