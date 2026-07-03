#include <stdio.h>
#include <string.h>

#include "nadir/analyzer.h"
#include "nadir/lexer.h"
#include "nadir/parser.h"

int main(void) {
    const char *source =
            "constant Register {\n"
            "    A = @lmao(5, u8);\n"
            "    B = 1;\n"
            "    C = Register.A;\n"
            "}\n"
            "\n"
            "procedure jmp(u16, u8) {\n"
            "    10;\n"
            "    @bit_and(@at(0), 255);\n"
            "    @bit_and(@bit_shr(@at(0), 8), 255);\n"
            "}\n"
            "\n"
            "binary { <LOL; jmp(); }\n";
    const auto lexer = nadir_lexer_new(source, strlen(source));
    const auto lexer_result = nadir_lexer_collect(lexer);
    if (lexer_result.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        printf("Token Error: %d, Line: %llu, Column: %llu\n", lexer_result.kind, lexer_result.line,
               lexer_result.column);
        nadir_lexer_free(lexer);
        return 1;
    }


    const auto parser = nadir_parser_new(lexer->tokens);
    const auto parser_result = nadir_parser_run(parser);
    if (parser_result.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        printf("Parse Error: %d, Token: %s, Line: %llu, Column: %llu\n", parser_result.kind, parser_result.token->value,
               parser_result.token->line, parser_result.token->column);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        return 1;
    }

    const auto analyser = nadir_analyzer_new(parser->ast);
    const auto analyser_result = nadir_analyzer_run(analyser);
    if (analyser_result.kind != NADIR_ANALYZER_ERROR_KIND_NONE) {
        printf("Analyse Error: %d, Token: %s, Line: %llu, Column: %llu\n", analyser_result.kind,
               analyser_result.token->value,
               analyser_result.token->line, analyser_result.token->column);
        return 1;
    }

    for (nadir_u64_t index = 0; index < analyser->constants->capacity; ++index) {
        if (analyser->constants->entries[index].is_used) {
            auto key = analyser->constants->entries[index].key;
            auto value = (nadir_analyzer_constant_t *) analyser->constants->entries[index].value;

            printf("%s %d = %s\n", key, value->value->kind, value->value->token->value);
        }
    }

    for (nadir_u64_t index = 0; index < analyser->procedures->capacity; ++index) {
        if (analyser->procedures->entries[index].is_used) {
            auto key = analyser->procedures->entries[index].key;
            auto value = (nadir_analyzer_procedure_t *) analyser->procedures->entries[index].value;

            printf("%s %llu parameters, %llu statements\n", key, value->parameters->length, value->statements->length);
        }
    }

    nadir_analyzer_free(analyser);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    return 0;
}
