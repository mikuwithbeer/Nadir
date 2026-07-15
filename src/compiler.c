/**
 * @file compiler.c
 * @brief The compiler implementation.
 */

#include "nadir/comptime.h"

#include <string.h>

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

nadir_compiler_t *nadir_compiler_new(nadir_arena_t *arena,
                                     nadir_ast_t *ast) {
    nadir_compiler_t *compiler = nadir_arena_allocate(arena, sizeof(nadir_compiler_t));
    if (compiler == nullptr) {
        return nullptr;
    }

    compiler->arena = arena;
    compiler->ast = ast;

    compiler->binary_location = (nadir_u64_t) -1;
    compiler->binary_origin = 0;
    compiler->binary_calculation = 0;

    const auto addresses = nadir_table_new(arena, sizeof(nadir_u64_t));
    if (addresses == nullptr) {
        return nullptr;
    }

    const auto constants = nadir_table_new(arena, sizeof(nadir_compiler_constant_t));
    if (constants == nullptr) {
        return nullptr;
    }

    const auto procedures = nadir_table_new(arena, sizeof(nadir_compiler_procedure_t));
    if (procedures == nullptr) {
        return nullptr;
    }

    const auto stack = nadir_stack_new(arena);
    if (stack == nullptr) {
        return nullptr;
    }

    const auto output = nadir_list_new(arena, sizeof(nadir_u8_t));
    if (output == nullptr) {
        return nullptr;
    }

    compiler->addresses = addresses;
    compiler->constants = constants;
    compiler->procedures = procedures;
    compiler->stack = stack;
    compiler->output = output;

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
                // Guard against multiple binary declarations.
                if (compiler->binary_location != (nadir_u64_t) -1) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_ALREADY_FOUND_BINARY, declaration->token);
                }

                compiler->binary_location = index;
                break;
            case NADIR_AST_DECLARATION_KIND_INCLUDE:
                continue; // Includes are handled before compiling
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
        compiler->binary_calculation = compiler->binary_origin + compiler->output->length;

        const nadir_ast_expression_t *statement = nadir_list_get(binary->binary.statements, index);
        switch (statement->kind) {
            case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            case NADIR_AST_EXPRESSION_KIND_MEMBER:
            case NADIR_AST_EXPRESSION_KIND_NUMBER: {
                // Evaluate the statement expression to get its value.
                error = nadir_compiler_evaluate(compiler, nullptr, statement);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Pop the statement value from the stack.
                nadir_i128_t statement_value;
                error = nadir_compiler_stack_pop(compiler, &statement_value, statement->token);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against a byte mismatch when writing the statement value to the output.
                const auto statement_byte = (nadir_u8_t) statement_value;
                if (statement_value != statement_byte) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH, statement->token);
                }

                if (!nadir_list_append(compiler->output, &statement_byte)) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, statement->token);
                }

                break;
            }
            case NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL: {
                const nadir_compiler_procedure_t *procedure = nadir_table_fetch(
                    compiler->procedures,
                    statement->token->string.value,
                    statement->token->string.count);

                if (procedure == nullptr) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
                }

                // Run the procedure call to generate the output for the procedure.
                error = nadir_compiler_run_procedure(compiler, statement, procedure);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                break;
            }
            case NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS:
                continue; // Already handled in the preparation phase
            case NADIR_AST_EXPRESSION_KIND_UNTIL:
            case NADIR_AST_EXPRESSION_KIND_REPEAT: {
                // Evaluate the padding value expression to get its value.
                error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.value);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                nadir_i128_t padding_value;
                error = nadir_compiler_stack_pop(compiler, &padding_value, statement->token);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against a byte mismatch when writing the padding value to the output.
                if (padding_value < NADIR_U8_MINIMUM || padding_value > NADIR_U8_MAXIMUM) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH,
                                                    statement->padding.value->token);
                }

                // Evaluate the padding times expression to get its value.
                error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.times);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                nadir_i128_t padding_times;
                error = nadir_compiler_stack_pop(compiler, &padding_times, statement->token);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                nadir_u64_t repeat_bytes = (nadir_u64_t) padding_times; // Already validated to be within the range
                if (statement->kind == NADIR_AST_EXPRESSION_KIND_UNTIL) {
                    repeat_bytes -= compiler->output->length; // Already validated to be within the range
                }

                // Append the padding bytes to the output.
                for (nadir_u64_t i = 0; i < repeat_bytes; ++i) {
                    const auto padding_byte = (nadir_u8_t) padding_value;
                    if (!nadir_list_append(compiler->output, &padding_byte)) {
                        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, statement->token);
                    }
                }

                break;
            }
            default:
                break; // Unreachable
        }
    }

    return error;
}

void nadir_compiler_free(nadir_compiler_t *compiler) {
    if (compiler == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    compiler->output = nullptr;
    compiler->stack = nullptr;
    compiler->procedures = nullptr;
    compiler->constants = nullptr;
    compiler->addresses = nullptr;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                       const nadir_ast_declaration_constant_t *declaration) {
    auto error = (nadir_compiler_error_t){};

    // Prepare each constant entry in the constant declaration.
    const auto member_first = declaration->name->string.value;
    const auto member_first_length = declaration->name->string.count;

    for (nadir_u64_t index = 0; index < declaration->entries->length; ++index) {
        const nadir_ast_declaration_constant_entry_t *constant_entry = nadir_list_get(declaration->entries, index);

        const auto member_second = constant_entry->name->string.value;
        const auto member_second_length = constant_entry->name->string.count;

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
        char member_key[NADIR_COMPILER_MEMBER_MAXIMUM] = {};
        nadir_u64_t member_key_length = 0;

        memcpy(member_key, member_first, member_first_length);
        member_key_length += member_first_length;

        member_key[member_key_length++] = '.';

        memcpy(member_key + member_key_length, member_second, member_second_length);
        member_key_length += member_second_length;

        const auto constant = (nadir_compiler_constant_t){
            .token = constant_entry->name,
            .value = constant_value,
        };

        if (!nadir_table_insert(compiler->constants, member_key, member_key_length, &constant)) {
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

    if (!nadir_table_insert(compiler->procedures,
                            declaration->name->string.value,
                            declaration->name->string.count,
                            &procedure)) {
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

        switch (statement->kind) {
            case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            case NADIR_AST_EXPRESSION_KIND_MEMBER:
            case NADIR_AST_EXPRESSION_KIND_NUMBER: {
                ++compiler->binary_calculation; // Expected to be a single byte in the output
                break;
            }
            case NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL: {
                const nadir_compiler_procedure_t *procedure = nadir_table_fetch(
                    compiler->procedures,
                    statement->token->string.value,
                    statement->token->string.count);

                if (procedure == nullptr) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_PROCEDURE, statement->token);
                }

                // Update the binary origin by adding the number of statements in the procedure.
                compiler->binary_calculation += procedure->statements->length;
                break;
            }
            case NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS: {
                const auto memory_address = compiler->binary_calculation + compiler->binary_origin;

                // Store the address in the addresses table.
                if (!nadir_table_insert(compiler->addresses,
                                        statement->token->string.value + 1,
                                        statement->token->string.count - 1,
                                        &memory_address)) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS, statement->token);
                }

                break;
            }
            case NADIR_AST_EXPRESSION_KIND_UNTIL:
            case NADIR_AST_EXPRESSION_KIND_REPEAT: {
                // Evaluate the padding value expression to get its value.
                auto error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.times);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                nadir_i128_t padding_times;
                error = nadir_compiler_stack_pop(compiler, &padding_times, statement->token);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against a padding times value that is out of range.
                if (padding_times <= NADIR_U64_MINIMUM || padding_times >= NADIR_U64_MAXIMUM) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE,
                                                    statement->padding.times->token);
                }

                const auto repeat_bytes = (nadir_u64_t) padding_times;
                if (statement->kind == NADIR_AST_EXPRESSION_KIND_UNTIL) {
                    // Guard against a padding to value that is less or equal than the current binary calculation.
                    if (compiler->binary_calculation >= repeat_bytes) {
                        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE,
                                                        statement->padding.times->token);
                    }

                    compiler->binary_calculation = repeat_bytes;
                } else {
                    compiler->binary_calculation += repeat_bytes;
                }

                break;
            }
            default:
                break; // Unreachable
        }
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
    const auto context = nadir_list_new(compiler->arena, sizeof(nadir_i128_t));
    if (context == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
    }

    // Process each argument and validate its type against.
    for (nadir_u64_t index = 0; index < procedure->parameters->length; ++index) {
        const nadir_ast_expression_t *procedure_argument = nadir_list_get(expression->call.arguments, index);

        // Evaluate the argument expression to get its value.
        error = nadir_compiler_evaluate(compiler, nullptr, procedure_argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Pop the argument value from the stack.
        nadir_i128_t argument_value;
        error = nadir_compiler_stack_pop(compiler, &argument_value, procedure_argument->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Validate the argument type against the expected parameter type.
        bool is_valid_type = false;
        switch (*(nadir_token_kind_t *) nadir_list_get(procedure->parameters, index)) {
            case NADIR_TOKEN_KIND_TYPE_U8:
                is_valid_type = argument_value >= NADIR_U8_MINIMUM && argument_value <= NADIR_U8_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_U16:
                is_valid_type = argument_value >= NADIR_U16_MINIMUM && argument_value <= NADIR_U16_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_U32:
                is_valid_type = argument_value >= NADIR_U32_MINIMUM && argument_value <= NADIR_U32_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_U64:
                is_valid_type = argument_value >= NADIR_U64_MINIMUM && argument_value <= NADIR_U64_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_I8:
                is_valid_type = argument_value >= NADIR_I8_MINIMUM && argument_value <= NADIR_I8_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_I16:
                is_valid_type = argument_value >= NADIR_I16_MINIMUM && argument_value <= NADIR_I16_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_I32:
                is_valid_type = argument_value >= NADIR_I32_MINIMUM && argument_value <= NADIR_I32_MAXIMUM;
                break;
            case NADIR_TOKEN_KIND_TYPE_I64:
                is_valid_type = argument_value >= NADIR_I64_MINIMUM && argument_value <= NADIR_I64_MAXIMUM;
                break;
            default:
                break; // Unreachable
        }

        if (!is_valid_type) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, procedure_argument->token);
        }

        if (!nadir_list_append(context, &argument_value)) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        }
    }

    // Evaluate each statement in the procedure.
    for (nadir_u64_t index = 0; index < procedure->statements->length; ++index) {
        compiler->binary_calculation = compiler->binary_origin + compiler->output->length;
        const nadir_ast_expression_t *statement = nadir_list_get(procedure->statements, index);

        // Evaluate the statement expression in the context of the procedure call.
        error = nadir_compiler_evaluate(compiler, context, statement);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Pop the statement value from the stack.
        nadir_i128_t statement_value;
        error = nadir_compiler_stack_pop(compiler, &statement_value, statement->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Guard against a byte mismatch when writing the statement value to the output.
        nadir_u8_t statement_byte = (nadir_u8_t) statement_value;
        if (statement_byte != statement_value) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH, statement->token);
        }

        if (!nadir_list_append(compiler->output, &statement_byte)) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        }
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
        case NADIR_AST_EXPRESSION_KIND_TYPE: {
            // Convert the token kind to the value.
            const auto type_value = expression->token->kind - NADIR_TOKEN_KIND_TYPE_U8;
            error = nadir_compiler_stack_push(compiler, type_value, expression->token);
            break;
        }
        case NADIR_AST_EXPRESSION_KIND_MEMBER: {
            const auto member_first = expression->token->string.value;
            const auto member_first_length = expression->token->string.count;

            const auto member_second = expression->member.field->string.value;
            const auto member_second_length = expression->member.field->string.count;

            // Format the member key to look up the constant value.
            char member_key[NADIR_COMPILER_MEMBER_MAXIMUM] = {};
            nadir_u64_t member_key_length = 0;

            memcpy(member_key, member_first, member_first_length);
            member_key_length += member_first_length;

            member_key[member_key_length++] = '.';

            memcpy(member_key + member_key_length, member_second, member_second_length);
            member_key_length += member_second_length;

            // Look up the constant value in the compiler's constant table.
            const nadir_compiler_constant_t *constant = nadir_table_fetch(compiler->constants,
                                                                          member_key,
                                                                          member_key_length);

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
            const nadir_u64_t *address = nadir_table_fetch(compiler->addresses,
                                                           expression->token->string.value + 1,
                                                           expression->token->string.count - 1);

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
    const auto arguments = nadir_list_new(compiler->arena, sizeof(nadir_i128_t));
    if (arguments == nullptr) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
    }

    // Evaluate each argument expression and store its value in the argument list.
    for (nadir_u64_t index = 0; index < expression->call.arguments->length; ++index) {
        const nadir_ast_expression_t *argument = nadir_list_get(expression->call.arguments, index);

        // Evaluate the argument expression with the context.
        error = nadir_compiler_evaluate(compiler, context, argument);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Pop the argument value from the stack.
        nadir_i128_t argument_value;
        error = nadir_compiler_stack_pop(compiler, &argument_value, argument->token);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        if (!nadir_list_append(arguments, &argument_value)) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        }
    }

    // Determine the compile-time kind.
    const auto kind = nadir_comptime_kind(expression->token->string.value, expression->token->string.count);
    if (kind == NADIR_COMPTIME_KIND_NONE) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME, expression->token);
    }

    const auto comptime = (nadir_comptime_t){
        .kind = kind,
        .arguments = arguments,
    };

    // Evaluate the compile-time call with the provided context and arguments.
    nadir_i128_t comptime_result;
    error = nadir_comptime_run(&comptime, compiler, context, &comptime_result);
    if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        // Propagate the error with the expression's token.
        error.token = expression->token;
        return error;
    }

    return nadir_compiler_stack_push(compiler, comptime_result, expression->token);
}
