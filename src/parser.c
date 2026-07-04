#include "nadir/parser.h"

#include <stdlib.h>

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

static nadir_parser_error_t nadir_parser_run_statements(nadir_parser_t *parser,
                                                        nadir_ast_declaration_kind_t kind,
                                                        nadir_list_t *statements);

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression);

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression);

// [--------------------------------------------------------------] //
// > Inline Functions                                             < //
// [--------------------------------------------------------------] //

static inline nadir_token_t *nadir_parser_peek(const nadir_parser_t *parser) {
    if (parser->token_index >= parser->tokens->length) {
        return nullptr;
    }

    return nadir_list_get(parser->tokens, parser->token_index);
}

static inline nadir_token_t *nadir_parser_advance(nadir_parser_t *parser) {
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

    // Peek and check if the next token matches the expected kind.
    auto token = nadir_parser_peek(parser);
    if (token && token->kind == kind) {
        token = nadir_parser_advance(parser);

        // If the output pointer is not null, assign the token to it.
        if (output != nullptr) {
            *output = token;
        }
    } else {
        if (kind == NADIR_TOKEN_KIND_SEMICOLON) {
            error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_MISSING_SEMICOLON, token);
        } else {
            error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
        }
    }

    return error;
}

static inline bool nadir_parser_is_constant_expression(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

static inline bool nadir_parser_is_procedure_statement(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

static inline bool nadir_parser_is_binary_statement(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS;
}

static inline bool nadir_parser_is_procedure_argument(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

static inline bool nadir_parser_is_comptime_argument(const nadir_ast_expression_kind_t kind) {
    return kind == NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL ||
           kind == NADIR_AST_EXPRESSION_KIND_MEMBER ||
           kind == NADIR_AST_EXPRESSION_KIND_TYPE ||
           kind == NADIR_AST_EXPRESSION_KIND_NUMBER;
}

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_parser_t *nadir_parser_new(nadir_list_t *tokens) {
    nadir_parser_t *parser = malloc(sizeof(nadir_parser_t));
    if (parser == nullptr) {
        return nullptr;
    }

    nadir_ast_t *ast = nadir_ast_new();
    if (ast == nullptr) {
        free(parser);
        return nullptr;
    }

    parser->ast = ast;

    parser->tokens = tokens;
    parser->token_index = 0;

    parser->seen_binary = false;

    return parser;
}

nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser) {
    auto error = (nadir_parser_error_t){};

    // Parse tokens until the end of the file is reached or an error occurs.
    while (error.kind == NADIR_PARSER_ERROR_KIND_NONE) {
        const auto token = nadir_parser_advance(parser);
        if (!token || token->kind == NADIR_TOKEN_KIND_EOF) {
            break;
        }

        if (token->kind == NADIR_TOKEN_KIND_CONSTANT) {
            error = nadir_parser_run_constant(parser);
        } else if (token->kind == NADIR_TOKEN_KIND_PROCEDURE) {
            error = nadir_parser_run_procedure(parser);
        } else if (token->kind == NADIR_TOKEN_KIND_BINARY) {
            if (parser->seen_binary) {
                error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_ALREADY_FOUND_BINARY, token);
            } else {
                parser->seen_binary = true;
                error = nadir_parser_run_binary(parser, token);
            }
        } else {
            error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
        }
    }

    return error;
}

void nadir_parser_free(nadir_parser_t *parser) {
    if (parser == nullptr) {
        return;
    }

    nadir_ast_free(parser->ast);
    free(parser);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_parser_error_t nadir_parser_run_constant(nadir_parser_t *parser) {
    // Consume the constant declaration name.
    nadir_token_t *name_token;
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the left brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    auto token = nadir_parser_peek(parser);

    // Check for an empty block.
    if (token != nullptr && token->kind == NADIR_TOKEN_KIND_RIGHT_BRACE) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK, name_token);
    }

    // Should be freed if an error occurs before the declaration is appended to the abstract syntax tree.
    const auto entries = nadir_list_new(sizeof(nadir_ast_constant_entry_t));
    if (entries == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // Parse constant entries until the right brace is reached.
    while (token != nullptr && token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        error = nadir_parser_run_constant_entry(parser, entries);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            goto cleanup;
        }

        token = nadir_parser_peek(parser);
    }

    // Check for unexpected end of file.
    if (token == nullptr) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, name_token);
        goto cleanup;
    }

    // Consume the right brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_CONSTANT,
        .data.constant = {
            .name = name_token,
            .entries = entries,
        }
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

cleanup:
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        if (entries != nullptr) {
            nadir_list_free(entries);
        }
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_constant_entry(nadir_parser_t *parser,
                                                            nadir_list_t *entries) {
    // Consume the constant entry name.
    nadir_token_t *name_token;
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the equal sign.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_EQUAL, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    auto expression = (nadir_ast_expression_t){};

    // Parse the constant entry expression.
    error = nadir_parser_run_expression(parser, &expression);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Check if the expression is a valid constant expression.
    if (!nadir_parser_is_constant_expression(expression.kind)) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, expression.token);
    }

    // Consume the semicolon.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto entry = (nadir_ast_constant_entry_t){
        .name = name_token,
        .value = expression,
    };

    if (!nadir_list_append(entries, &entry)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_procedure(nadir_parser_t *parser) {
    // Consume the procedure name.
    nadir_token_t *name_token;
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the left parenthesis.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Should be freed if an error occurs before the declaration is appended to the abstract syntax tree.
    const auto parameters = nadir_list_new(sizeof(nadir_token_kind_t));
    if (parameters == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // Should be freed if an error occurs before the declaration is appended to the abstract syntax tree.
    const auto statements = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (statements == nullptr) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
        goto cleanup;
    }

    // Parse procedure parameters.
    error = nadir_parser_run_procedure_parameters(parser, parameters);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    // Consume the right parenthesis.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    // Consume the left brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    // Parse procedure body statements.
    error = nadir_parser_run_statements(parser, NADIR_AST_DECLARATION_KIND_PROCEDURE, statements);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_PROCEDURE,
        .data.procedure = {
            .name = name_token,
            .parameters = parameters,
            .statements = statements
        }
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

cleanup:
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        if (statements != nullptr) {
            nadir_list_free(statements);
        }

        if (parameters != nullptr) {
            nadir_list_free(parameters);
        }
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_procedure_parameters(nadir_parser_t *parser,
                                                                  nadir_list_t *parameters) {
    auto next_token = nadir_parser_peek(parser);

    // Check if there are parameters to parse.
    if (next_token && next_token->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        bool expect_parameter = false;

        do {
            // Check if the maximum number of parameters has been reached.
            if (parameters->length >= NADIR_PARSER_ARGUMENTS_MAXIMUM) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS, next_token);
            }

            // Peek at the next token to check for a parameter type.
            next_token = nadir_parser_advance(parser);
            if (next_token == nullptr) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
            }

            // Check if the token is a valid parameter type.
            const auto kind = next_token->kind;
            if (nadir_token_is_type(kind)) {
                if (!nadir_list_append(parameters, &kind)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
                }
            } else {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, next_token);
            }

            // Check if the next token is a comma.
            next_token = nadir_parser_peek(parser);
            expect_parameter = next_token && next_token->kind == NADIR_TOKEN_KIND_COMMA;
            if (expect_parameter) {
                if (nadir_parser_advance(parser) == nullptr) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, next_token);
                }
            }
        } while (expect_parameter);
    }

    return (nadir_parser_error_t){};
}

static nadir_parser_error_t nadir_parser_run_binary(nadir_parser_t *parser,
                                                    nadir_token_t *token) {
    // Consume the left brace.
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Should be freed if an error occurs before the declaration is appended to the abstract syntax tree.
    const auto statements = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (statements == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

    // Parse binary body statements.
    error = nadir_parser_run_statements(parser, NADIR_AST_DECLARATION_KIND_BINARY, statements);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        goto cleanup;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = token,
        .kind = NADIR_AST_DECLARATION_KIND_BINARY,
        .data.binary.statements = statements
    };

    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
    }

cleanup:
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        if (statements != nullptr) {
            nadir_list_free(statements);
        }
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_statements(nadir_parser_t *parser,
                                                        nadir_ast_declaration_kind_t kind,
                                                        nadir_list_t *statements) {
    auto next_token = nadir_parser_peek(parser);

    // Check for an empty block.
    if (next_token != nullptr && next_token->kind == NADIR_TOKEN_KIND_RIGHT_BRACE) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_EMPTY_BLOCK, next_token);
    }

    // Parse statements until the right brace is reached.
    while (next_token != nullptr && next_token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        auto expression = (nadir_ast_expression_t){};

        // Parse the next statement in the procedure body.
        auto error = nadir_parser_run_expression(parser, &expression);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        // Check if the expression is valid for the given declaration kind.
        if (kind == NADIR_AST_DECLARATION_KIND_PROCEDURE) {
            if (!nadir_parser_is_procedure_statement(expression.kind)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, expression.token);
            }
        } else {
            if (!nadir_parser_is_binary_statement(expression.kind)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION, expression.token);
            }
        }

        // Consume the semicolon after the statement.
        error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        if (!nadir_list_append(statements, &expression)) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
        }

        next_token = nadir_parser_peek(parser);
    }

    // Check for unexpected end of file.
    if (next_token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    // Consume the right brace.
    return nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
}

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression) {
    auto error = (nadir_parser_error_t){};

    // Determine the expression kind.
    auto token = nadir_parser_peek(parser);
    if (token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    // Parse compile-time calls.
    if (token->kind == NADIR_TOKEN_KIND_COMPTIME) {
        if (nadir_parser_advance(parser) == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, token);
        }

        return nadir_parser_run_call(parser, token, expression);
    }

    // Parse number literals.
    if (token->kind == NADIR_TOKEN_KIND_NUMBER) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_NUMBER;
        expression->token = token;

        return error;
    }

    // Parse store address expressions.
    if (token->kind == NADIR_TOKEN_KIND_STORE_ADDRESS) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_STORE_ADDRESS;
        expression->token = token;

        return error;
    }

    // Parse load address expressions.
    if (token->kind == NADIR_TOKEN_KIND_LOAD_ADDRESS) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_LOAD_ADDRESS;
        expression->token = token;

        return error;
    }

    // Parse type literals.
    if (nadir_token_is_type(token->kind)) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_TYPE;
        expression->token = token;

        return error;
    }

    // Parse identifiers (constant members or procedure calls).
    if (token->kind == NADIR_TOKEN_KIND_IDENT) {
        // Consume the identifier token.
        nadir_token_t *ident_token;
        error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &ident_token);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        // Determine whether it's a call or access.
        token = nadir_parser_peek(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, ident_token);
        }

        // Parse if procedure calls.
        if (token->kind == NADIR_TOKEN_KIND_LEFT_PAREN) {
            return nadir_parser_run_call(parser, ident_token, expression);
        }

        // Parse if member field accesses.
        if (token->kind == NADIR_TOKEN_KIND_DOT) {
            // Consume the dot.
            error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_DOT, nullptr);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            // Consume the field identifier.
            nadir_token_t *field_token;
            error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &field_token);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            expression->kind = NADIR_AST_EXPRESSION_KIND_MEMBER;
            expression->token = ident_token;
            expression->data.member.field = field_token;

            return error;
        }
    }

    // None of the above cases matched.
    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_MISSING_EXPRESSION, token);
    return error;
}

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression) {
    // Consume the left parenthesis.
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Should be freed if an error occurs before the expression is appended to the abstract syntax tree.
    nadir_list_t *arguments = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (!arguments) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, nullptr);
    }

    // Determine the expression kind based on the token kind.
    expression->kind = token->kind == NADIR_TOKEN_KIND_IDENT
                           ? NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL
                           : NADIR_AST_EXPRESSION_KIND_COMPTIME_CALL;

    // Parse call arguments until the right parenthesis is reached.
    auto next_token = nadir_parser_peek(parser);
    if (next_token && next_token->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        bool expect_argument = false;

        do {
            // Check if the maximum number of arguments has been reached.
            if (arguments->length >= NADIR_PARSER_ARGUMENTS_MAXIMUM) {
                error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_TOO_MANY_ARGUMENTS, next_token);
                goto cleanup;
            }

            // Parse the next argument expression.
            nadir_ast_expression_t argument_expression = {};
            error = nadir_parser_run_expression(parser, &argument_expression);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                goto cleanup;
            }

            // Check if the argument expression is valid for the call.
            if (expression->kind == NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL) {
                if (!nadir_parser_is_procedure_argument(argument_expression.kind)) {
                    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION,
                                                   argument_expression.token);
                    goto cleanup;
                }
            } else {
                if (!nadir_parser_is_comptime_argument(argument_expression.kind)) {
                    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EXPRESSION,
                                                   argument_expression.token);
                    goto cleanup;
                }
            }

            if (!nadir_list_append(arguments, &argument_expression)) {
                error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
                goto cleanup;
            }

            // Check if there is a comma.
            next_token = nadir_parser_peek(parser);
            expect_argument = next_token && next_token->kind == NADIR_TOKEN_KIND_COMMA;
            if (expect_argument) {
                if (nadir_parser_advance(parser) == nullptr) {
                    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, next_token);
                    goto cleanup;
                }
            }
        } while (expect_argument);
    }

    // Consume the right parenthesis.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, &next_token);

cleanup:
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        if (arguments != nullptr) {
            nadir_list_free(arguments);
        }
    } else {
        expression->token = token;
        expression->data.call.arguments = arguments;
    }

    return error;
}
