/**
 * @file parser.c
 * @brief The parser implementation.
 */

#include "nadir/parser.h"

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_parser_error_t nadir_parser_run_constant(nadir_parser_t *parser);

static nadir_parser_error_t nadir_parser_run_constant_entry(nadir_parser_t *parser,
                                                            nadir_list_t *entries);

static nadir_parser_error_t nadir_parser_run_procedure(nadir_parser_t *parser);

static nadir_parser_error_t nadir_parser_run_procedure_parameters(nadir_parser_t *parser,
                                                                  nadir_list_t *parameters);

static nadir_parser_error_t nadir_parser_run_binary(nadir_parser_t *parser,
                                                    nadir_token_t *token);

static nadir_parser_error_t nadir_parser_run_include(nadir_parser_t *parser,
                                                     nadir_token_t *token);

static nadir_parser_error_t nadir_parser_run_statements(nadir_parser_t *parser,
                                                        nadir_ast_declaration_kind_t kind,
                                                        nadir_list_t *statements);

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression);

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression);

static nadir_parser_error_t nadir_parser_run_padding(nadir_parser_t *parser,
                                                     nadir_token_t *token,
                                                     nadir_ast_expression_t *expression);

static nadir_parser_error_t nadir_parser_run_ident(nadir_parser_t *parser,
                                                   nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Inline Functions                                             < //
// [--------------------------------------------------------------] //

[[nodiscard]] static inline nadir_token_t *nadir_parser_peek(const nadir_parser_t *parser) {
    if (parser->token_index >= parser->tokens->length) {
        return nullptr;
    }

    return nadir_list_get(parser->tokens, parser->token_index);
}

[[nodiscard]] static inline nadir_token_t *nadir_parser_advance(nadir_parser_t *parser) {
    const auto token = nadir_parser_peek(parser);
    if (token == nullptr) {
        return nullptr;
    }

    ++parser->token_index;
    return token;
}

static inline nadir_parser_error_t nadir_parser_consume(nadir_parser_t *parser,
                                                        const nadir_token_kind_t kind,
                                                        nadir_token_t **output) {
    auto error = (nadir_parser_error_t){};

    auto token = nadir_parser_peek(parser);
    if (token && token->kind == kind) {
        token = nadir_parser_advance(parser);
        if (output != nullptr) {
            *output = token;
        }
    } else {
        // Special case for missing semicolon to provide a more specific error message.
        if (kind == NADIR_TOKEN_KIND_SEMICOLON) {
            error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_MISSING_SEMICOLON, token);
        } else {
            error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
        }
    }

    return error;
}

[[nodiscard]] static inline bool nadir_parser_is_constant_expression(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

[[nodiscard]] static inline bool nadir_parser_is_procedure_statement(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

[[nodiscard]] static inline bool nadir_parser_is_binary_statement(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS ||
           kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_UNTIL ||
           kind == NADIR_AST_EXPRESSION_KIND_REPEAT;
}

[[nodiscard]] static inline bool nadir_parser_is_procedure_argument(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

[[nodiscard]] static inline bool nadir_parser_is_comptime_argument(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_TYPE ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

[[nodiscard]] static inline bool nadir_parser_is_padding_value(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

[[nodiscard]] static inline bool nadir_parser_is_padding_times(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_parser_t *nadir_parser_new(nadir_arena_t *arena,
                                 nadir_list_t *tokens) {
    nadir_parser_t *parser = nadir_arena_allocate(arena, sizeof(nadir_parser_t));
    if (parser == nullptr) {
        return nullptr;
    }

    nadir_ast_t *ast = nadir_ast_new(arena);
    if (ast == nullptr) {
        return nullptr;
    }

    parser->arena = arena;
    parser->ast = ast;

    parser->tokens = tokens;
    parser->token_index = 0;

    parser->seen_binary = false;

    return parser;
}

nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser) {
    auto error = (nadir_parser_error_t){};

    while (error.kind == NADIR_PARSER_ERROR_KIND_NONE) {
        const auto token = nadir_parser_advance(parser);
        if (!token || token->kind == NADIR_TOKEN_KIND_EOF) {
            break;
        }

        switch (token->kind) {
            case NADIR_TOKEN_KIND_CONSTANT:
                error = nadir_parser_run_constant(parser);
                break;
            case NADIR_TOKEN_KIND_PROCEDURE:
                error = nadir_parser_run_procedure(parser);
                break;
            case NADIR_TOKEN_KIND_BINARY:
                if (parser->seen_binary) {
                    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY, token);
                } else {
                    parser->seen_binary = true; // Prevent multiple binary declarations
                    error = nadir_parser_run_binary(parser, token);
                }

                break;
            case NADIR_TOKEN_KIND_INCLUDE:
                error = nadir_parser_run_include(parser, token);
                break;
            default:
                error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
                break;
        }
    }

    return error;
}

void nadir_parser_free(nadir_parser_t *parser) {
    if (parser == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    parser->ast = nullptr;
    parser->token_index = 0;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_parser_error_t nadir_parser_run_constant(nadir_parser_t *parser) {
    nadir_token_t *name_token; // Constant declaration name
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    auto next_token = nadir_parser_peek(parser);

    // Empty block is not allowed for constant declarations.
    if (next_token != nullptr && next_token->kind == NADIR_TOKEN_KIND_RIGHT_BRACE) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK, name_token);
    }

    // List to hold the constant entries.
    const auto entries = nadir_list_new(parser->arena, sizeof(nadir_ast_declaration_constant_entry_t));
    if (entries == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    while (next_token != nullptr && next_token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        error = nadir_parser_run_constant_entry(parser, entries);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        next_token = nadir_parser_peek(parser);
    }

    if (next_token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, name_token);
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_CONSTANT,
        .constant = {
            .name = name_token,
            .entries = entries,
        }
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_constant_entry(nadir_parser_t *parser,
                                                            nadir_list_t *entries) {
    nadir_token_t *name_token; // Constant entry name
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_EQUAL, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    nadir_ast_expression_t constant_expression = {}; // Constant entry value expression
    error = nadir_parser_run_expression(parser, &constant_expression);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Guard against a non-constant expression.
    if (!nadir_parser_is_constant_expression(constant_expression.kind)) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, constant_expression.token);
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto entry = (nadir_ast_declaration_constant_entry_t){
        .name = name_token,
        .value = constant_expression,
    };

    if (!nadir_list_append(entries, &entry)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_procedure(nadir_parser_t *parser) {
    nadir_token_t *name_token; // Procedure declaration name
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // List to hold the procedure parameters.
    const auto parameters = nadir_list_new(parser->arena, sizeof(nadir_token_kind_t));
    if (parameters == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // List to hold the procedure body statements.
    const auto statements = nadir_list_new(parser->arena, sizeof(nadir_ast_expression_t));
    if (statements == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    error = nadir_parser_run_procedure_parameters(parser, parameters);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    error = nadir_parser_run_statements(parser, NADIR_AST_DECLARATION_KIND_PROCEDURE, statements);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_PROCEDURE,
        .procedure = {
            .name = name_token,
            .parameters = parameters,
            .statements = statements
        }
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_procedure_parameters(nadir_parser_t *parser,
                                                                  nadir_list_t *parameters) {
    // Check for an empty parameter list.
    auto next_token = nadir_parser_peek(parser);
    if (next_token && next_token->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        bool expect_parameter = false;

        do {
            if (parameters->length >= NADIR_PARSER_ARGUMENTS_MAXIMUM) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS, next_token);
            }

            next_token = nadir_parser_advance(parser);
            if (next_token == nullptr) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
            }

            const auto kind = next_token->kind;
            if (nadir_token_value_type(kind)) {
                if (!nadir_list_append(parameters, &kind)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
                }
            } else {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, next_token);
            }

            next_token = nadir_parser_peek(parser);

            expect_parameter = next_token && next_token->kind == NADIR_TOKEN_KIND_COMMA;
            if (expect_parameter) {
                (void) nadir_parser_advance(parser); // Consume the comma
            }
        } while (expect_parameter);
    }

    return (nadir_parser_error_t){};
}

static nadir_parser_error_t nadir_parser_run_binary(nadir_parser_t *parser,
                                                    nadir_token_t *token) {
    nadir_token_t *origin_token; // Binary declaration origin number
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_NUMBER, &origin_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        error.kind = NADIR_PARSER_ERROR_KIND_MISSING_BINARY_ORIGIN;
        return error;
    }

    // Guard against an out-of-range origin number.
    const auto origin_value = (nadir_u64_t) origin_token->number;
    if (origin_token->number != origin_value) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_INVALID_BINARY_ORIGIN, origin_token);
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // List to hold the binary body statements.
    const auto statements = nadir_list_new(parser->arena, sizeof(nadir_ast_expression_t));
    if (statements == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

    error = nadir_parser_run_statements(parser, NADIR_AST_DECLARATION_KIND_BINARY, statements);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = token,
        .kind = NADIR_AST_DECLARATION_KIND_BINARY,
        .binary = {
            .origin = origin_value,
            .statements = statements,
        }
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_include(nadir_parser_t *parser,
                                                     nadir_token_t *token) {
    nadir_token_t *path_token; // Include declaration path
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_PATH, &path_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = token,
        .kind = NADIR_AST_DECLARATION_KIND_INCLUDE,
        .include.path = path_token,
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_statements(nadir_parser_t *parser,
                                                        const nadir_ast_declaration_kind_t kind,
                                                        nadir_list_t *statements) {
    // Empty blocks are not allowed for procedure or binary declarations.
    auto next_token = nadir_parser_peek(parser);
    if (next_token != nullptr && next_token->kind == NADIR_TOKEN_KIND_RIGHT_BRACE) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK, next_token);
    }

    while (next_token != nullptr && next_token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        nadir_ast_expression_t expression = {}; // Statement expression
        auto error = nadir_parser_run_expression(parser, &expression);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        // Guard against an unexpected expression based on the declaration kind.
        if (kind == NADIR_AST_DECLARATION_KIND_PROCEDURE) {
            if (!nadir_parser_is_procedure_statement(expression.kind)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, expression.token);
            }
        } else {
            if (!nadir_parser_is_binary_statement(expression.kind)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, expression.token);
            }
        }

        error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        if (!nadir_list_append(statements, &expression)) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
        }

        next_token = nadir_parser_peek(parser);
    }

    if (next_token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    return nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
}

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression) {
    const auto error = (nadir_parser_error_t){};

    // Peek to decide how to parse the expression.
    const auto next_token = nadir_parser_peek(parser);
    if (next_token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    switch (next_token->kind) {
        case NADIR_TOKEN_KIND_COMPTIME:
            (void) nadir_parser_advance(parser);

            return nadir_parser_run_call(parser, next_token, expression);
        case NADIR_TOKEN_KIND_NUMBER:
            (void) nadir_parser_advance(parser);
            expression->kind = NADIR_AST_EXPRESSION_KIND_NUMBER;
            expression->token = next_token;

            return error;
        case NADIR_TOKEN_KIND_STORE_ADDRESS:
            (void) nadir_parser_advance(parser);
            expression->kind = NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS;
            expression->token = next_token;

            return error;
        case NADIR_TOKEN_KIND_LOAD_ADDRESS:
            (void) nadir_parser_advance(parser);
            expression->kind = NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS;
            expression->token = next_token;

            return error;
        case NADIR_TOKEN_KIND_UNTIL:
            (void) nadir_parser_advance(parser);
            expression->kind = NADIR_AST_EXPRESSION_KIND_UNTIL;

            return nadir_parser_run_padding(parser, next_token, expression);
        case NADIR_TOKEN_KIND_REPEAT:
            (void) nadir_parser_advance(parser);
            expression->kind = NADIR_AST_EXPRESSION_KIND_REPEAT;

            return nadir_parser_run_padding(parser, next_token, expression);
        case NADIR_TOKEN_KIND_IDENT:
            return nadir_parser_run_ident(parser, expression);
        default:
            if (nadir_token_value_type(next_token->kind)) {
                (void) nadir_parser_advance(parser);
                expression->kind = NADIR_AST_EXPRESSION_KIND_TYPE;
                expression->token = next_token;

                return error;
            }

            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION, next_token);
    }
}

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression) {
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // List to hold the call arguments.
    nadir_list_t *arguments = nadir_list_new(parser->arena, sizeof(nadir_ast_expression_t));
    if (!arguments) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, nullptr);
    }

    expression->call.arguments = arguments;
    expression->token = token;
    expression->kind = token->kind == NADIR_TOKEN_KIND_IDENT
                           ? NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL
                           : NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL;

    // Determine if there are any arguments.
    auto next_token = nadir_parser_peek(parser);
    if (next_token && next_token->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        bool expect_argument = false;

        do {
            if (arguments->length >= NADIR_PARSER_ARGUMENTS_MAXIMUM) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS, next_token);
            }

            nadir_ast_expression_t argument_expression = {}; // Argument expression for the call
            error = nadir_parser_run_expression(parser, &argument_expression);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            // Guard against an unexpected expression based on the call kind.
            if (expression->kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
                if (!nadir_parser_is_procedure_argument(argument_expression.kind)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION,
                                                  argument_expression.token);
                }
            } else {
                if (!nadir_parser_is_comptime_argument(argument_expression.kind)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION,
                                                  argument_expression.token);
                }
            }

            if (!nadir_list_append(arguments, &argument_expression)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
            }

            next_token = nadir_parser_peek(parser);

            expect_argument = next_token && next_token->kind == NADIR_TOKEN_KIND_COMMA;
            if (expect_argument) {
                (void) nadir_parser_advance(parser); // Consume the comma
            }
        } while (expect_argument);
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, &next_token);
    return error;
}

static nadir_parser_error_t nadir_parser_run_padding(nadir_parser_t *parser,
                                                     nadir_token_t *token,
                                                     nadir_ast_expression_t *expression) {
    nadir_ast_expression_t value_expression = {}; // Padding value expression
    auto error = nadir_parser_run_expression(parser, &value_expression);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Guard against a non-padding value expression.
    if (!nadir_parser_is_padding_value(value_expression.kind)) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, value_expression.token);
    }

    nadir_ast_expression_t times_expression = {}; // Padding times expression
    error = nadir_parser_run_expression(parser, &times_expression);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Guard against a non-padding times expression.
    if (!nadir_parser_is_padding_times(times_expression.kind)) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, times_expression.token);
    }

    // Need allocation to avoid lifetime issues since they are local variables.
    nadir_ast_expression_t *value = nadir_arena_allocate(parser->arena, sizeof(nadir_ast_expression_t));
    nadir_ast_expression_t *times = nadir_arena_allocate(parser->arena, sizeof(nadir_ast_expression_t));
    if (value == nullptr || times == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

    *value = value_expression;
    *times = times_expression;

    expression->token = token;
    expression->padding.value = value;
    expression->padding.times = times;

    return error;
}

static nadir_parser_error_t nadir_parser_run_ident(nadir_parser_t *parser,
                                                   nadir_ast_expression_t *expression) {
    nadir_token_t *ident_token; // Identifier token for procedure call or member access
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &ident_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto next_token = nadir_parser_peek(parser);
    if (next_token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, ident_token);
    }

    switch (next_token->kind) {
        case NADIR_TOKEN_KIND_LEFT_PAREN:
            return nadir_parser_run_call(parser, ident_token, expression);
        case NADIR_TOKEN_KIND_DOT:
            error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_DOT, nullptr);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            nadir_token_t *field_token; // Field token for member access
            error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &field_token);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            expression->kind = NADIR_AST_EXPRESSION_KIND_MEMBER;
            expression->token = ident_token;
            expression->member.field = field_token;

            return error;
        default:
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION, next_token);
    }
}
