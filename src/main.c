#include "nadir/cli.h"
#include "nadir/compiler.h"
#include "nadir/lexer.h"
#include "nadir/parser.h"

#include <stdio.h>

int main(const int argc,
         char **argv) {
    auto cli = nadir_cli_new();
    if (!nadir_cli_parse(&cli, argc, argv)) {
        puts("Failed to parse command-line arguments!");

        nadir_cli_close(&cli);
        return 1;
    }

    if (cli.input_file == nullptr) {
        puts("No input file specified!");

        nadir_cli_close(&cli);
        return 1;
    }

    if (!nadir_cli_read(&cli)) {
        puts("Failed to read input file!");

        nadir_cli_close(&cli);
        return 1;
    }

    const auto lexer = nadir_lexer_new(cli.input, cli.input_length);
    const auto lexer_result = nadir_lexer_collect(lexer);
    if (lexer_result.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        puts("Lexer error!");

        nadir_lexer_free(lexer);
        nadir_cli_close(&cli);
        return 1;
    }

    const auto parser = nadir_parser_new(lexer->tokens);
    const auto parser_result = nadir_parser_run(parser);
    if (parser_result.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        puts("Parser error!");

        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        nadir_cli_close(&cli);
        return 1;
    }

    const auto compiler = nadir_compiler_new(parser->ast);
    auto compiler_result = nadir_compiler_prepare(compiler);
    if (compiler_result.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        puts("Compiler phase 1 error!");

        nadir_compiler_free(compiler);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        nadir_cli_close(&cli);
        return 1;
    }

    compiler_result = nadir_compiler_run(compiler);
    if (compiler_result.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        puts("Compiler phase 2 error!");

        nadir_compiler_free(compiler);
        nadir_parser_free(parser);
        nadir_lexer_free(lexer);
        nadir_cli_close(&cli);
        return 1;
    }

    if (!nadir_cli_write(&cli, compiler->output)) {
        puts("Failed to write output file!");
    }

    nadir_compiler_free(compiler);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    nadir_cli_close(&cli);

    printf("Written %llu bytes to: %s\n", cli.output_length, cli.output_file);
    return 0;
}
