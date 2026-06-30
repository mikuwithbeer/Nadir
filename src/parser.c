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

static nadir_parser_error_t nadir_parser_run_procedure_statements(nadir_parser_t *parser,
                                                                  nadir_list_t *statements);

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression);

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression);

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

    return parser;
}

nadir_token_t *nadir_parser_peek(const nadir_parser_t *parser) {
    if (parser->token_index >= parser->tokens->length) {
        return nullptr;
    }

    return nadir_list_get(parser->tokens, parser->token_index);
}

nadir_token_t *nadir_parser_advance(nadir_parser_t *parser) {
    const auto token = nadir_parser_peek(parser);
    if (token == nullptr) {
        return nullptr;
    }

    ++parser->token_index;
    return token;
}

nadir_parser_error_t nadir_parser_consume(nadir_parser_t *parser,
                                          const nadir_token_kind_t kind,
                                          nadir_token_t **output) {
    auto token = nadir_parser_peek(parser);
    if (token && token->kind == kind) {
        token = nadir_parser_advance(parser);
        if (output != nullptr) {
            *output = token;
        }

        return (nadir_parser_error_t){};
    }

    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
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

    // Create a list for constant entries.
    const auto entries = nadir_list_new(sizeof(nadir_ast_const_entry_t));
    if (entries == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // Parse constant entries until the right brace is reached.
    auto token = nadir_parser_peek(parser);
    while (token != nullptr && token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        error = nadir_parser_run_constant_entry(parser, entries);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        token = nadir_parser_peek(parser);
    }

    // Check for unexpected end of file.
    if (token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, name_token);
    }

    // Consume the right brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_CONSTANT,
        .data.constant = {
            .name = name_token,
            .entries = entries,
        }
    };

    // Append the declaration to the AST.
    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
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

    // Consume the semicolon.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Create the constant entry.
    const auto entry = (nadir_ast_const_entry_t){
        .name = name_token,
        .value = expression,
    };

    // Append the entry to the list.
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

    // Create a list for procedure parameters.
    const auto parameters = nadir_list_new(sizeof(nadir_token_t *));
    if (parameters == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // Parse procedure parameters until the right parenthesis is reached.
    error = nadir_parser_run_procedure_parameters(parser, parameters);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the right parenthesis.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the left brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Create a list for procedure body statements.
    const auto body_statements = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (body_statements == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    // Parse procedure body statements until the right brace is reached.
    error = nadir_parser_run_procedure_statements(parser, body_statements);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Consume the right brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, nullptr);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_PROCEDURE,
        .data.procedure = {
            .name = name_token,
            .parameters = parameters,
            .statements = body_statements
        }
    };

    // Append the declaration to the AST.
    if (!nadir_list_append(parser->ast->declarations, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
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
            next_token = nadir_parser_advance(parser);
            if (next_token == nullptr) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
            }

            // Check if the token is a valid parameter type.
            if (nadir_token_is_type(next_token->kind)) {
                if (!nadir_list_append(parameters, &next_token)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
                }
            } else {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, next_token);
            }

            // Peek at the next token to check for a comma.
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

static nadir_parser_error_t nadir_parser_run_procedure_statements(nadir_parser_t *parser,
                                                                  nadir_list_t *statements) {
    auto error = (nadir_parser_error_t){};

    auto token = nadir_parser_peek(parser);
    while (token != nullptr && token->kind != NADIR_TOKEN_KIND_RIGHT_BRACE) {
        auto expression = (nadir_ast_expression_t){};

        // Parse the next statement in the procedure body.
        error = nadir_parser_run_expression(parser, &expression);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        // Consume the semicolon after the statement.
        error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, nullptr);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        // Append the statement to the list.
        if (!nadir_list_append(statements, &expression)) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, token);
        }

        // Peek at the next token.
        token = nadir_parser_peek(parser);
    }

    // Check for unexpected end of file.
    if (token == nullptr) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression) {
    auto error = (nadir_parser_error_t){};

    // Peek at the next token to determine the expression kind.
    auto token = nadir_parser_peek(parser);
    if (token == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    // Parse built-in calls.
    if (token->kind == NADIR_TOKEN_KIND_BUILTIN) {
        if (nadir_parser_advance(parser) == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
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
        nadir_token_t *ident_token;
        error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &ident_token);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        token = nadir_parser_peek(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, ident_token);
        }

        // Parse procedure calls.
        if (token->kind == NADIR_TOKEN_KIND_LEFT_PAREN) {
            return nadir_parser_run_call(parser, ident_token, expression);
        }

        // Parse member field accesses.
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

    error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
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

    // Create a list for call arguments.
    nadir_list_t *arguments = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (!arguments) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, nullptr);
    }

    // Parse arguments if the parameter list is not empty.
    auto next_token = nadir_parser_peek(parser);
    if (next_token && next_token->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        bool expect_argument = false;

        do {
            // Parse the next argument expression.
            nadir_ast_expression_t argument_expression = {};
            error = nadir_parser_run_expression(parser, &argument_expression);
            if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            if (!nadir_list_append(arguments, &argument_expression)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, next_token);
            }

            // Peek at the next token to check for a comma.
            next_token = nadir_parser_peek(parser);
            expect_argument = next_token && next_token->kind == NADIR_TOKEN_KIND_COMMA;
            if (expect_argument) {
                if (nadir_parser_advance(parser) == nullptr) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, next_token);
                }
            }
        } while (expect_argument);
    }

    // Consume the right parenthesis.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, &next_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Determine if the call is a custom procedure or a built-in.
    expression->kind = token->kind == NADIR_TOKEN_KIND_IDENT
                           ? NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL
                           : NADIR_AST_EXPRESSION_KIND_BUILTIN_CALL;

    expression->token = token;
    expression->data.call.arguments = arguments;

    return error;
}
