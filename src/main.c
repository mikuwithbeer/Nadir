/**
 * @file main.c
 * @brief The entry point for the application.
 *
 * This file contains the main process that contains the entire
 * assembler process, including command-line argument parsing,
 * lexing, parsing, and compiling.
 */

#include "nadir/cli.h"
#include "nadir/error.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Global Variables                                             < //
// [--------------------------------------------------------------] //

static nadir_arena_t cli_arena = {};
static nadir_arena_t comptime_arena = {};
static nadir_arena_t assembler_arena = {};

static nadir_cli_t cli = {};
static nadir_lexer_t *lexer = nullptr;
static nadir_parser_t *parser = nullptr;
static nadir_compiler_t *compiler = nullptr;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

typedef enum [[nodiscard]] {
    STATE_CONTINUE,
    STATE_EXIT,
    STATE_ERROR,
} state_t;

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static state_t process_arena(void);

static state_t process_cli(int argc,
                           char **argv);

static state_t process_lexer(void);

static state_t process_parser(void);

static state_t process_compiler(void);

static state_t process_success(void);

static void process_cleanup(void);

// [--------------------------------------------------------------] //
// > Main Function                                                < //
// [--------------------------------------------------------------] //

int main(const int argc,
         char **argv) {
    auto state = process_arena();
    if (state == STATE_CONTINUE) {
        state = process_cli(argc, argv);
    }

    if (state == STATE_CONTINUE) {
        state = process_lexer();
    }

    if (state == STATE_CONTINUE) {
        state = process_parser();
    }

    if (state == STATE_CONTINUE) {
        state = process_compiler();
    }

    if (state == STATE_CONTINUE) {
        state = process_success();
    }

    process_cleanup();
    return state == STATE_EXIT ? EXIT_SUCCESS : EXIT_FAILURE;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static state_t process_arena(void) {
    if (!nadir_arena_init(&cli_arena, NADIR_ARENA_DEFAULT_CAPACITY)) {
        fprintf(stderr, "error: failed to initialize command-line arena allocator\n");
        return STATE_ERROR;
    }

    if (!nadir_arena_init(&comptime_arena, NADIR_ARENA_DEFAULT_CAPACITY)) {
        fprintf(stderr, "error: failed to initialize comptime arena allocator\n");
        return STATE_ERROR;
    }

    if (!nadir_arena_init(&assembler_arena, NADIR_ARENA_DEFAULT_CAPACITY)) {
        fprintf(stderr, "error: failed to initialize assembler arena allocator\n");
        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_cli(const int argc,
                           char **argv) {
    cli = nadir_cli_new(&cli_arena);

    if (!nadir_cli_parse(&cli, argc, argv)) {
        fprintf(stderr, "error: failed to parse command-line arguments\n");
        return STATE_ERROR;
    }

    if (cli.help) {
        nadir_cli_help();
        return STATE_EXIT;
    }

    if (cli.version) {
        nadir_cli_version();
        return STATE_EXIT;
    }

    if (cli.input_file == nullptr) {
        fprintf(stderr, "error: no input file specified\n");
        return STATE_ERROR;
    }

    if (!nadir_cli_read(&cli)) {
        fprintf(stderr, "error: failed to read input file: %s\n", cli.input_file);
        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_lexer(void) {
    lexer = nadir_lexer_new(&assembler_arena, cli.input, cli.input_length);

    const auto lexer_error = nadir_lexer_collect(lexer);
    if (lexer_error.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        const nadir_error_t error = {
            .kind = NADIR_ERROR_KIND_LEXER,
            .lexer = lexer_error,
        };

        const auto message = nadir_error_encode(&cli_arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
        } else {
            fprintf(stderr, "error: out of memory\n");
        }

        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_parser(void) {
    parser = nadir_parser_new(&assembler_arena, lexer->tokens);

    const auto parser_error = nadir_parser_run(parser);
    if (parser_error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        const nadir_error_t error = {
            .kind = NADIR_ERROR_KIND_PARSER,
            .parser = parser_error,
        };

        const auto message = nadir_error_encode(&cli_arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
        } else {
            fprintf(stderr, "error: out of memory\n");
        }

        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_compiler(void) {
    compiler = nadir_compiler_new(&assembler_arena, &comptime_arena, parser->ast);

    auto compiler_error = nadir_compiler_prepare(compiler);
    if (compiler_error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        goto print;
    }

    compiler_error = nadir_compiler_run(compiler);

print:
    if (compiler_error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        const nadir_error_t error = {
            .kind = NADIR_ERROR_KIND_COMPILER,
            .compiler = compiler_error,
        };

        const auto message = nadir_error_encode(&cli_arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
        } else {
            fprintf(stderr, "error: out of memory\n");
        }

        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_success(void) {
    if (cli.dry_run) {
        printf("assembled %" PRIu64 " bytes, no output written due to dry-run mode\n", compiler->output->length);
        return STATE_EXIT;
    }

    if (!nadir_cli_write(&cli, compiler->output)) {
        fprintf(stderr, "error: failed to write output to file: %s\n", cli.output_file);
        return STATE_ERROR;
    }

    printf("wrote %" PRIu64 " bytes to: %s\n", compiler->output->length, cli.output_file);
    return STATE_EXIT;
}

static void process_cleanup(void) {
    nadir_compiler_free(compiler);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    nadir_cli_close(&cli);

    nadir_arena_reset(&assembler_arena);
    nadir_arena_free(&assembler_arena);

    nadir_arena_reset(&comptime_arena);
    nadir_arena_free(&comptime_arena);

    nadir_arena_reset(&cli_arena);
    nadir_arena_free(&cli_arena);
}
