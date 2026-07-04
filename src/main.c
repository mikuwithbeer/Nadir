#include "nadir/cli.h"
#include "nadir/error.h"

#include <stdio.h>
#include <stdlib.h>

static nadir_cli_t cli = {};
static nadir_lexer_t *lexer = nullptr;
static nadir_parser_t *parser = nullptr;
static nadir_compiler_t *compiler = nullptr;

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr auto NADIR_EXIT_SUCCESS = 0;
constexpr auto NADIR_EXIT_FAILURE = 1;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

typedef enum {
    NADIR_STATE_CONTINUE,
    NADIR_STATE_EXIT,
    NADIR_STATE_ERROR,
} nadir_state_t;

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static nadir_state_t process_cli(int argc,
                                 char **argv);

static nadir_state_t process_lexer(void);

static nadir_state_t process_parser(void);

static nadir_state_t process_compiler(void);

static nadir_state_t process_success(void);

static void process_cleanup(void);

// [--------------------------------------------------------------] //
// > Main Function                                                < //
// [--------------------------------------------------------------] //

int main(const int argc,
         char **argv) {
    auto state = process_cli(argc, argv);
    if (state == NADIR_STATE_CONTINUE) {
        state = process_lexer();
    }

    if (state == NADIR_STATE_CONTINUE) {
        state = process_parser();
    }

    if (state == NADIR_STATE_CONTINUE) {
        state = process_compiler();
    }

    if (state == NADIR_STATE_CONTINUE) {
        state = process_success();
    }

    process_cleanup();
    return state == NADIR_STATE_EXIT ? NADIR_EXIT_SUCCESS : NADIR_EXIT_FAILURE;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_state_t process_cli(const int argc,
                                 char **argv) {
    cli = nadir_cli_new();

    if (!nadir_cli_parse(&cli, argc, argv)) {
        fprintf(stderr, "error: failed to parse command-line arguments\n");
        return NADIR_STATE_ERROR;
    }

    if (cli.help) {
        nadir_cli_help();
        return NADIR_STATE_EXIT;
    }

    if (cli.version) {
        nadir_cli_version();
        return NADIR_STATE_EXIT;
    }

    if (cli.input_file == nullptr) {
        fprintf(stderr, "error: no input file specified\n");
        return NADIR_STATE_ERROR;
    }

    if (!nadir_cli_read(&cli)) {
        fprintf(stderr, "error: failed to read input file: %s\n", cli.input_file);
        return NADIR_STATE_ERROR;
    }

    return NADIR_STATE_CONTINUE;
}

static nadir_state_t process_lexer(void) {
    lexer = nadir_lexer_new(cli.input, cli.input_length);

    const auto lexer_error = nadir_lexer_collect(lexer);
    if (lexer_error.kind != NADIR_LEXER_ERROR_KIND_NONE) {
        const auto error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_LEXER,
            .lexer = lexer_error,
        };

        const auto message = nadir_error_encode(&error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
            free(message);
        } else {
            fprintf(stderr, "error: lexer\n");
        }

        return NADIR_STATE_ERROR;
    }

    return NADIR_STATE_CONTINUE;
}

static nadir_state_t process_parser(void) {
    parser = nadir_parser_new(lexer->tokens);

    const auto parser_error = nadir_parser_run(parser);
    if (parser_error.kind != NADIR_PARSER_ERROR_KIND_NONE) {
        const auto error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_PARSER,
            .parser = parser_error,
        };

        const auto message = nadir_error_encode(&error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
            free(message);
        } else {
            fprintf(stderr, "error: parser\n");
        }

        return NADIR_STATE_ERROR;
    }

    return NADIR_STATE_CONTINUE;
}

static nadir_state_t process_compiler(void) {
    compiler = nadir_compiler_new(parser->ast);

    auto compiler_error = nadir_compiler_prepare(compiler);
    if (compiler_error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        goto print;
    }

    compiler_error = nadir_compiler_run(compiler);

print:
    if (compiler_error.kind != NADIR_COMPILER_ERROR_KIND_NONE) {
        const auto error = (nadir_error_t){
            .kind = NADIR_ERROR_KIND_COMPILER,
            .compiler = compiler_error,
        };

        const auto message = nadir_error_encode(&error);
        if (message != nullptr) {
            fprintf(stderr, "%s:%s\n", cli.input_file, message);
            free(message);
        } else {
            fprintf(stderr, "error: compiler\n");
        }

        return NADIR_STATE_ERROR;
    }

    return NADIR_STATE_CONTINUE;
}

static nadir_state_t process_success(void) {
    if (!nadir_cli_write(&cli, compiler->output)) {
        fprintf(stderr, "error: failed to write output file: %s\n", cli.output_file);
        return NADIR_STATE_ERROR;
    }

    printf("assembler %s to %s (%llu bytes)\n", cli.input_file, cli.output_file, compiler->output->length);
    return NADIR_STATE_EXIT;
}

static void process_cleanup(void) {
    nadir_compiler_free(compiler);
    nadir_parser_free(parser);
    nadir_lexer_free(lexer);
    nadir_cli_close(&cli);
}
