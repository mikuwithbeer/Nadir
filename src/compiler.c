/**
 * @file compiler.c
 * @brief The compiler implementation.
 */

#include "nadir/comptime.h"

#include <string.h>
#include <stddef.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                              const nadir_ast_declaration_constant_t *declaration);

static nadir_compiler_error_t nadir_compiler_prepare_procedure(const nadir_compiler_t *compiler,
                                                               const nadir_ast_declaration_procedure_t *declaration);

static nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                            const nadir_ast_declaration_binary_t *declaration);

static nadir_compiler_error_t nadir_compiler_run_procedure(nadir_compiler_t *compiler,
                                                           const nadir_ast_expression_t *expression,
                                                           const nadir_compiler_procedure_t *procedure);

static nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                                      const nadir_context_t *context,
                                                      const nadir_ast_expression_t *expression,
                                                      nadir_i128_t *result);

static nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                               const nadir_context_t *context,
                                                               const nadir_ast_expression_t *expression,
                                                               nadir_i128_t *result);

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
    compiler->binary_offset = 0;

    auto const addresses = nadir_table_new(arena, sizeof(nadir_u64_t));
    if (addresses == nullptr) {
        return nullptr;
    }

    auto const constants = nadir_table_new(arena, sizeof(nadir_compiler_constant_t));
    if (constants == nullptr) {
        return nullptr;
    }

    auto const procedures = nadir_table_new(arena, sizeof(nadir_compiler_procedure_t));
    if (procedures == nullptr) {
        return nullptr;
    }

    auto const output = nadir_list_new(arena, sizeof(nadir_u8_t));
    if (output == nullptr) {
        return nullptr;
    }

    compiler->addresses = addresses;
    compiler->constants = constants;
    compiler->procedures = procedures;
    compiler->output = output;

    return compiler;
}

nadir_compiler_error_t nadir_compiler_prepare(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};

    // Empty abstract syntax trees are not allowed.
    if (compiler->ast->declarations->length == 0) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_EMPTY, nullptr);
    }

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

    // We must have a binary declaration to compile the program.
    if (compiler->binary_location == (nadir_u64_t) -1) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_BINARY, nullptr);
    }

    const nadir_ast_declaration_t *binary = nadir_list_get(compiler->ast->declarations, compiler->binary_location);
    return nadir_compiler_prepare_binary(compiler, &binary->binary);
}

nadir_compiler_error_t nadir_compiler_run(nadir_compiler_t *compiler) {
    auto error = (nadir_compiler_error_t){};
    const nadir_ast_declaration_t *binary = nadir_list_get(compiler->ast->declarations, compiler->binary_location);

    for (nadir_u64_t index = 0; index < binary->binary.statements->length; ++index) {
        compiler->binary_offset = compiler->output->length;

        const nadir_ast_expression_t *statement = nadir_list_get(binary->binary.statements, index);
        switch (statement->kind) {
            case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            case NADIR_AST_EXPRESSION_KIND_MEMBER:
            case NADIR_AST_EXPRESSION_KIND_NUMBER: {
                nadir_i128_t statement_value; // Value of the statement expression
                error = nadir_compiler_evaluate(compiler, nullptr, statement, &statement_value);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against values that cannot fit in a single byte.
                auto const statement_byte = (nadir_u8_t) statement_value;
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
                nadir_i128_t padding_value; // Value of the padding expression
                error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.value, &padding_value);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against padding values that cannot fit in a single byte.
                if (padding_value < NADIR_U8_MINIMUM || padding_value > NADIR_U8_MAXIMUM) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_BYTE_MISMATCH,
                                                    statement->padding.value->token);
                }

                nadir_i128_t padding_times; // Value of the padding times expression
                error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.times, &padding_times);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                nadir_u64_t repeat_bytes = (nadir_u64_t) padding_times;
                if (statement->kind == NADIR_AST_EXPRESSION_KIND_UNTIL) {
                    if (repeat_bytes < compiler->output->length) {
                        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE,
                                                        statement->padding.times->token);
                    }

                    repeat_bytes -= compiler->output->length;
                }

                auto const padding_byte = (nadir_u8_t) padding_value; // Already validated to be within the range
                if (!nadir_list_fill(compiler->output, &padding_byte, repeat_bytes)) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, statement->token);
                }

                break;
            }
            default:
                unreachable();
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
    compiler->procedures = nullptr;
    compiler->constants = nullptr;
    compiler->addresses = nullptr;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_compiler_error_t nadir_compiler_prepare_constant(nadir_compiler_t *compiler,
                                                              const nadir_ast_declaration_constant_t *declaration) {
    auto error = (nadir_compiler_error_t){};

    auto const object_string_value = declaration->name->string.value;
    auto const object_string_count = declaration->name->string.count;

    for (nadir_u64_t index = 0; index < declaration->entries->length; ++index) {
        const nadir_ast_declaration_constant_entry_t *constant_entry = nadir_list_get(declaration->entries, index);

        auto const field_string_value = constant_entry->name->string.value;
        auto const field_string_count = constant_entry->name->string.count;

        nadir_i128_t constant_value; // Variable to hold the evaluated constant value
        error = nadir_compiler_evaluate(compiler, nullptr, &constant_entry->value, &constant_value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Create a unique member key for the constant entry in the format "object.field".
        char member_key[NADIR_COMPILER_MEMBER_MAXIMUM] = {};
        nadir_u64_t member_key_length = 0;

        memcpy(member_key, object_string_value, object_string_count);
        member_key_length += object_string_count;
        member_key[member_key_length++] = '.';
        memcpy(member_key + member_key_length, field_string_value, field_string_count);
        member_key_length += field_string_count;

        const nadir_compiler_constant_t constant = {
            .token = constant_entry->name,
            .value = constant_value,
        };

        if (!nadir_table_insert(compiler->constants, member_key, member_key_length, &constant)) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_CONSTANT, constant_entry->name);
        }
    }

    return error;
}

static nadir_compiler_error_t nadir_compiler_prepare_procedure(const nadir_compiler_t *compiler,
                                                               const nadir_ast_declaration_procedure_t *declaration) {
    const nadir_compiler_procedure_t procedure = {
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

static nadir_compiler_error_t nadir_compiler_prepare_binary(nadir_compiler_t *compiler,
                                                            const nadir_ast_declaration_binary_t *declaration) {
    compiler->binary_origin = declaration->origin;

    for (nadir_u64_t index = 0; index < declaration->statements->length; ++index) {
        const nadir_ast_expression_t *statement = nadir_list_get(declaration->statements, index);

        switch (statement->kind) {
            case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            case NADIR_AST_EXPRESSION_KIND_MEMBER:
            case NADIR_AST_EXPRESSION_KIND_NUMBER: {
                ++compiler->binary_offset; // Expected to be a single byte in the output
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

                // We can safely add the length of the procedure statements to the binary calculation.
                compiler->binary_offset += procedure->statements->length;
                break;
            }
            case NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS: {
                auto const memory_address = compiler->binary_offset + compiler->binary_origin;

                // Remove the leading character to get the actual address name.
                auto const address_string_value = statement->token->string.value + 1;
                auto const address_string_count = statement->token->string.count - 1;

                if (!nadir_table_insert(compiler->addresses,
                                        address_string_value,
                                        address_string_count,
                                        &memory_address)) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_MULTIPLE_ADDRESS, statement->token);
                }

                break;
            }
            case NADIR_AST_EXPRESSION_KIND_UNTIL:
            case NADIR_AST_EXPRESSION_KIND_REPEAT: {
                nadir_i128_t padding_times; // Variable to hold the evaluated padding times value
                auto const error = nadir_compiler_evaluate(compiler, nullptr, statement->padding.times, &padding_times);
                if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
                    return error;
                }

                // Guard against a padding times value that is out of range.
                if (padding_times <= NADIR_U64_MINIMUM || padding_times >= NADIR_U64_MAXIMUM) {
                    return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE,
                                                    statement->padding.times->token);
                }

                // Guaranteed to be within the range of a 64-bit unsigned integer, so we can safely cast it.
                auto const repeat_bytes = (nadir_u64_t) padding_times;
                if (statement->kind == NADIR_AST_EXPRESSION_KIND_UNTIL) {
                    // We need to check if the current output length does not exceed the specified padding times, as it would be out of range.
                    if (compiler->binary_offset >= repeat_bytes) {
                        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_PADDING_OUT_OF_RANGE,
                                                        statement->padding.times->token);
                    }

                    compiler->binary_offset = repeat_bytes;
                } else {
                    compiler->binary_offset += repeat_bytes;
                }

                break;
            }
            default:
                unreachable();
        }
    }

    return (nadir_compiler_error_t){};
}

static nadir_compiler_error_t nadir_compiler_run_procedure(nadir_compiler_t *compiler,
                                                           const nadir_ast_expression_t *expression,
                                                           const nadir_compiler_procedure_t *procedure) {
    auto error = (nadir_compiler_error_t){};

    if (procedure->parameters->length != expression->call.arguments->length) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_ARGUMENT_MISMATCH, expression->token);
    }

    nadir_context_t context = {}; // Context for the procedure call

    // Argument evaluation and type checking for each parameter in the procedure.
    for (nadir_u64_t index = 0; index < procedure->parameters->length; ++index) {
        const nadir_ast_expression_t *procedure_argument = nadir_list_get(expression->call.arguments, index);

        nadir_i128_t argument_value; // Variable to hold the evaluated argument value
        error = nadir_compiler_evaluate(compiler, nullptr, procedure_argument, &argument_value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Guard against a type mismatch between the parameter and the argument.
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
                unreachable();
        }

        if (!is_valid_type) {
            return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_TYPE_MISMATCH, procedure_argument->token);
        }

        context.value[context.length++] = argument_value;
    }

    // Evaluate each statement in the procedure and write its value to the output.
    for (nadir_u64_t index = 0; index < procedure->statements->length; ++index) {
        // Must be updated before evaluating each statement.
        compiler->binary_offset = compiler->output->length;
        const nadir_ast_expression_t *statement = nadir_list_get(procedure->statements, index);

        nadir_i128_t statement_value; // Variable to hold the evaluated statement value
        error = nadir_compiler_evaluate(compiler, &context, statement, &statement_value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            return error;
        }

        // Every statement value must fit in a single byte, so we check for that here.
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

static nadir_compiler_error_t nadir_compiler_evaluate(nadir_compiler_t *compiler,
                                                      const nadir_context_t *context,
                                                      const nadir_ast_expression_t *expression,
                                                      nadir_i128_t *result) {
    auto error = (nadir_compiler_error_t){};

    switch (expression->kind) {
        case NADIR_AST_EXPRESSION_KIND_NUMBER:
            *result = expression->token->number;
            break;
        case NADIR_AST_EXPRESSION_KIND_TYPE: {
            *result = expression->token->kind - NADIR_TOKEN_KIND_TYPE_U8;
            break;
        }
        case NADIR_AST_EXPRESSION_KIND_MEMBER: {
            auto const object_string_value = expression->token->string.value;
            auto const object_string_count = expression->token->string.count;

            auto const field_string_value = expression->member.field->string.value;
            auto const field_string_count = expression->member.field->string.count;

            // Create a unique member key for the constant in the format "object.field".
            char member_key[NADIR_COMPILER_MEMBER_MAXIMUM] = {};
            nadir_u64_t member_key_length = 0;

            memcpy(member_key, object_string_value, object_string_count);
            member_key_length += object_string_count;
            member_key[member_key_length++] = '.';
            memcpy(member_key + member_key_length, field_string_value, field_string_count);
            member_key_length += field_string_count;

            const nadir_compiler_constant_t *constant = nadir_table_fetch(compiler->constants,
                                                                          member_key,
                                                                          member_key_length);

            if (constant == nullptr) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_CONSTANT, expression->member.field);
            }

            *result = constant->value;
            break;
        }
        case NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL:
            error = nadir_compiler_evaluate_comptime(compiler, context, expression, result);
            break;
        case NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS: {
            // Remove the leading character to get the actual address name.
            auto const address_value = expression->token->string.value + 1;
            auto const address_count = expression->token->string.count - 1;

            const nadir_u64_t *address = nadir_table_fetch(compiler->addresses, address_value, address_count);
            if (address == nullptr) {
                return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_ADDRESS, expression->token);
            }

            *result = *address;
            break;
        }
        default:
            unreachable();
    }

    return error;
}

static nadir_compiler_error_t nadir_compiler_evaluate_comptime(nadir_compiler_t *compiler,
                                                               const nadir_context_t *context,
                                                               const nadir_ast_expression_t *expression,
                                                               nadir_i128_t *result) {
    auto error = (nadir_compiler_error_t){};

    nadir_arena_t comptime_arena = {}; // Arena for the compile-time call arguments
    if (!nadir_arena_init(&comptime_arena, NADIR_COMPILER_ARENA_CAPACITY)) {
        return nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
    }

    // List to hold the evaluated argument values for the compile-time call.
    auto const arguments = nadir_list_new(&comptime_arena, sizeof(nadir_i128_t));
    if (arguments == nullptr) {
        error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
        goto cleanup;
    }

    for (nadir_u64_t index = 0; index < expression->call.arguments->length; ++index) {
        const nadir_ast_expression_t *argument = nadir_list_get(expression->call.arguments, index);

        nadir_i128_t argument_value; // Variable to hold the evaluated argument value
        error = nadir_compiler_evaluate(compiler, context, argument, &argument_value);
        if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        if (!nadir_list_append(arguments, &argument_value)) {
            error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_OUT_OF_MEMORY, expression->token);
            goto cleanup;
        }
    }

    auto const kind = nadir_comptime_kind(expression->token->string.value, expression->token->string.count);
    if (kind == NADIR_COMPTIME_KIND_NONE) {
        error = nadir_compiler_error_new(NADIR_COMPILER_ERROR_KIND_UNDEFINED_COMPTIME, expression->token);
        goto cleanup;
    }

    const nadir_comptime_t comptime = {
        .kind = kind,
        .arguments = arguments,
    };

    nadir_i128_t comptime_result; // Variable to hold the result of the compile-time evaluation
    error = nadir_comptime_run(&comptime, compiler, context, &comptime_result);
    if (error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        // Since the `nadir_comptime_run` function does not have access to the token, we set it here for better error reporting.
        error.token = expression->token;
        goto cleanup;
    }

    *result = comptime_result;

cleanup:
    nadir_arena_reset(&comptime_arena);
    nadir_arena_free(&comptime_arena);
    return error;
}
