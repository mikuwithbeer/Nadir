#include "nadir/parser.h"

#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_parser_error_t nadir_parser_run_constant(nadir_parser_t *parser);

static nadir_parser_error_t nadir_parser_run_constant_declaration(nadir_parser_t *parser,
                                                                  nadir_list_t *entry_list);

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
    nadir_token_t *token = nadir_parser_peek(parser);
    if (token == nullptr) {
        return nullptr;
    }

    parser->token_index++;
    return token;
}

nadir_parser_error_t nadir_parser_consume(nadir_parser_t *parser,
                                          const nadir_token_kind_t kind,
                                          nadir_token_t **out_token) {
    nadir_token_t *token = nadir_parser_peek(parser);

    if (token && token->kind == kind) {
        *out_token = nadir_parser_advance(parser);
        return (nadir_parser_error_t){};
    }

    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
}

nadir_parser_error_t nadir_parser_run(nadir_parser_t *parser) {
    while (true) {
        const auto token = nadir_parser_advance(parser);
        if (!token || token->kind == NADIR_TOKEN_KIND_EOF) {
            break;
        }

        // Check if the token is a constant declaration.
        // TODO: Add support for procedures.
        if (token->kind != NADIR_TOKEN_KIND_CONSTANT) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
        }

        const auto error = nadir_parser_run_constant(parser);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }
    }

    return (nadir_parser_error_t){};
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
    nadir_token_t *name_token, *brace_token, *right_brace;

    // Collect the name of the constant declaration.
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Collect the left brace.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_BRACE, &brace_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Create new list to hold all our constant entries.
    const auto entry_list = nadir_list_new(sizeof(nadir_ast_const_entry_t));
    if (entry_list == nullptr) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, brace_token);
    }

    // Parse all the constant entries until right brace.
    while (true) {
        const auto token = nadir_parser_peek(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        // Check if it is end of the constant declaration.
        if (token->kind == NADIR_TOKEN_KIND_RIGHT_BRACE) {
            break;
        }

        // Parse the constant entry.
        error = nadir_parser_run_constant_declaration(parser, entry_list);
        if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }
    }

    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_BRACE, &right_brace);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    const auto declaration = (nadir_ast_declaration_t){
        .token = name_token,
        .kind = NADIR_AST_DECLARATION_KIND_CONSTANT,
        .data.constant = {
            .name = name_token,
            .entries = entry_list,
        }
    };

    if (!nadir_ast_append_declaration(parser->ast, &declaration)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_constant_declaration(nadir_parser_t *parser,
                                                                  nadir_list_t *entry_list) {
    nadir_token_t *name_token, *equal_token, *semi_token;

    // Collect the name of the constant entry.
    auto error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &name_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Collect the equal sign.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_EQUAL, &equal_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    auto entry = (nadir_ast_const_entry_t){
        .name = name_token,
        .value = {}
    };

    // Collect the value of the constant entry.
    error = nadir_parser_run_expression(parser, &entry.value);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Collect the semicolon.
    error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_SEMICOLON, &semi_token);
    if (error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Append the entry to the list of entries.
    if (!nadir_list_append(entry_list, &entry)) {
        error = nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, name_token);
    }

    return error;
}

static nadir_parser_error_t nadir_parser_run_expression(nadir_parser_t *parser,
                                                        nadir_ast_expression_t *expression) {
    nadir_token_t *token = nadir_parser_peek(parser);
    if (!token) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
    }

    // Handle built-in function calls.
    if (token->kind == NADIR_TOKEN_KIND_BUILTIN) {
        if (!nadir_parser_advance(parser)) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        return nadir_parser_run_call(parser, token, expression);
    }

    // Handle number literals.
    if (token->kind == NADIR_TOKEN_KIND_NUMBER) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_NUMBER;
        expression->token = token;

        return (nadir_parser_error_t){};
    }

    // Handle type literals.
    if (nadir_token_is_type(token->kind)) {
        token = nadir_parser_advance(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        expression->kind = NADIR_AST_EXPRESSION_KIND_TYPE;
        expression->token = token;

        return (nadir_parser_error_t){};
    }

    // It could be a constant member, or a procedure call.
    if (token->kind == NADIR_TOKEN_KIND_IDENT) {
        nadir_parser_error_t error;
        nadir_token_t *ident_token;

        if ((error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &ident_token)).kind
            != NADIR_PARSER_ERROR_KIND_NONE) {
            return error;
        }

        token = nadir_parser_peek(parser);
        if (token == nullptr) {
            return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, nullptr);
        }

        // Handle procedure calls.
        if (token->kind == NADIR_TOKEN_KIND_LEFT_PAREN) {
            return nadir_parser_run_call(parser, ident_token, expression);
        }

        // Handle accessing a member field.
        if (token->kind == NADIR_TOKEN_KIND_DOT) {
            nadir_token_t *dot_token, *field_token;

            if ((error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_DOT, &dot_token)).kind
                != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            if ((error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_IDENT, &field_token)).kind
                != NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            expression->kind = NADIR_AST_EXPRESSION_KIND_MEMBER;
            expression->token = dot_token;
            expression->data.member.object = ident_token;
            expression->data.member.field = field_token;

            return (nadir_parser_error_t){};
        }
    }

    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_TOKEN, token);
}

static nadir_parser_error_t nadir_parser_run_call(nadir_parser_t *parser,
                                                  nadir_token_t *token,
                                                  nadir_ast_expression_t *expression) {
    nadir_parser_error_t error;
    nadir_token_t *left_paren, *right_paren;

    // Open up parentheses!
    if ((error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_LEFT_PAREN, &left_paren)).kind
        != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Create a new list to hold the arguments of the call.
    nadir_list_t *arguments = nadir_list_new(sizeof(nadir_ast_expression_t));
    if (!arguments) {
        return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, left_paren);
    }

    right_paren = nadir_parser_peek(parser);

    // If it's not immediately empty `()`, parse the arguments.
    if (right_paren && right_paren->kind != NADIR_TOKEN_KIND_RIGHT_PAREN) {
        while (true) {
            nadir_ast_expression_t argument_expression = {};

            if ((error = nadir_parser_run_expression(parser, &argument_expression)).kind !=
                NADIR_PARSER_ERROR_KIND_NONE) {
                return error;
            }

            if (!nadir_list_append(arguments, &argument_expression)) {
                return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_OUT_OF_MEMORY, left_paren);
            }

            right_paren = nadir_parser_peek(parser);

            // Commas mean more arguments are coming.
            if (right_paren && right_paren->kind == NADIR_TOKEN_KIND_COMMA) {
                if (!nadir_parser_advance(parser)) {
                    return nadir_parser_error_new(NADIR_PARSER_ERROR_KIND_UNEXPECTED_EOF, right_paren);
                }
            } else {
                break;
            }
        }
    }

    // Close the parentheses!
    if ((error = nadir_parser_consume(parser, NADIR_TOKEN_KIND_RIGHT_PAREN, &right_paren)).kind
        != NADIR_PARSER_ERROR_KIND_NONE) {
        return error;
    }

    // Figure out if this was a custom procedure or a system built-in.
    expression->kind = token->kind == NADIR_TOKEN_KIND_IDENT
                           ? NADIR_AST_EXPRESSION_KIND_PROCEDURE_CALL
                           : NADIR_AST_EXPRESSION_KIND_BUILTIN_CALL;

    expression->token = token;
    expression->data.call.name = token;
    expression->data.call.arguments = arguments;

    return (nadir_parser_error_t){};
}
