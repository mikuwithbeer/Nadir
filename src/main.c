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
#include "nadir/module.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// [--------------------------------------------------------------] //
// > Global Variables                                             < //
// [--------------------------------------------------------------] //

static nadir_arena_t arena = {};

static nadir_cli_t cli = {};
static nadir_module_t *module = nullptr;
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

static state_t process_module(void);

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
        state = process_module();
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
    if (!nadir_arena_init(&arena, NADIR_ARENA_DEFAULT_CAPACITY)) {
        fprintf(stderr, "error: failed to initialize arena allocator\n");
        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_cli(const int argc,
                           char **argv) {
    cli = nadir_cli_new(&arena);

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

    return STATE_CONTINUE;
}

static state_t process_module(void) {
    module = nadir_module_new(&arena);
    if (module == nullptr) {
        fprintf(stderr, "error: failed to create module\n");
        return STATE_ERROR;
    }

    if (!nadir_module_resolve(module, cli.input_file)) {
        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static state_t process_compiler(void) {
    compiler = nadir_compiler_new(&arena, module->ast);

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

        const auto message = nadir_error_encode(&arena, &error);
        if (message != nullptr) {
            fprintf(stderr, "%s\n", message);
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
        fprintf(stderr, "error: failed to write output to file '%s'\n", cli.output_file);
        return STATE_ERROR;
    }

    printf("wrote %" PRIu64 " bytes to '%s'\n", compiler->output->length, cli.output_file);
    return STATE_EXIT;
}

static void process_cleanup(void) {
    nadir_compiler_free(compiler);
    nadir_module_free(module);
    nadir_cli_close(&cli);

    nadir_arena_reset(&arena);
    nadir_arena_free(&arena);
}
