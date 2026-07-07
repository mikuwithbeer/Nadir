/**
 * @file cli.c
 * @brief The CLI implementation.
 */

#include "nadir/cli.h"

#include <getopt.h>
#include <stdio.h>

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

// Command-line options for the assembler.
static const struct option cli_options[5] = {
    {"help", no_argument, nullptr, 'h'},
    {"version", no_argument, nullptr, 'v'},
    {"input", required_argument, nullptr, 'i'},
    {"output", required_argument, nullptr, 'o'},
    {nullptr, 0, nullptr, 0},
};

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_cli_t nadir_cli_new(nadir_arena_t *arena) {
    return (nadir_cli_t){
        .arena = arena,

        .input_file = nullptr,
        .output_file = NADIR_CLI_DEFAULT_OUTPUT,

        .input = nullptr,
        .output = nullptr,

        .input_length = 0,
        .output_length = 0,

        .help = false,
        .version = false,
    };
}

bool nadir_cli_parse(nadir_cli_t *cli,
                     const int argc,
                     char **argv) {
    while (true) {
        // Parse the next command-line option.
        const auto option = getopt_long(argc, argv, "hvi:o:", cli_options, nullptr);
        if (option == -1) {
            break;
        }

        switch (option) {
            case 'h':
                cli->help = true;
                break;
            case 'v':
                cli->version = true;
                break;
            case 'i':
                cli->input_file = optarg;
                break;
            case 'o':
                cli->output_file = optarg;
                break;
            default:
                return false;
        }
    }

    return true;
}

void nadir_cli_help(void) {
    puts("Usage: Nadir [OPTIONS]\n"
        "\n"
        "Options:\n"
        "  -h, --help       Print this help message and exit\n"
        "  -v, --version    Print the version number and exit\n"
        "  -i, --input      Specify the input file to compile\n"
        "  -o, --output     Specify the output file to write to\n"
        "\n"
        "Examples:\n"
        "  Nadir --version\n"
        "  Nadir -i main.asm -o main.bin");
}

void nadir_cli_version(void) {
    printf("Nadir Assembler v%s\n", NADIR_VERSION);
}

bool nadir_cli_read(nadir_cli_t *cli) {
    FILE *input_file = fopen(cli->input_file, "rb");
    if (input_file == nullptr) {
        return false;
    }

    // Determine the size of the input file.
    fseek(input_file, 0, SEEK_END);
    const auto input_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    if (input_size <= 0) {
        fclose(input_file);
        return false;
    }

    char *input_buffer = nadir_arena_allocate(cli->arena, (nadir_u64_t) input_size + 1);
    if (input_buffer == nullptr) {
        fclose(input_file);
        return false;
    }

    const auto input_length = fread(input_buffer, sizeof(char), (nadir_u64_t) input_size, input_file);
    input_buffer[input_length] = '\0';

    // Validate whether the number of bytes read matches the expected size.
    if ((nadir_u64_t) input_size != input_length) {
        fclose(input_file);
        return false;
    }

    cli->input = input_buffer;
    cli->input_length = input_length;

    fclose(input_file);
    return true;
}

bool nadir_cli_write(nadir_cli_t *cli,
                     const nadir_list_t *output) {
    FILE *output_file = fopen(cli->output_file, "wb");
    if (output_file == nullptr) {
        return false;
    }

    const auto written = fwrite(output->items, output->size, output->length, output_file);

    // Validate whether the number of items written matches the expected length.
    if (output->length != written) {
        fclose(output_file);
        return false;
    }

    cli->output = (char *) output->items;
    cli->output_length = output->length * output->size; // Total bytes

    fclose(output_file);
    return true;
}

void nadir_cli_close(nadir_cli_t *cli) {
    if (cli == nullptr) {
        return;
    }

    // The arena handles resource management, so we just reset the structure.
    cli->input_file = nullptr;
    cli->output_file = nullptr;
    cli->input = nullptr;
    cli->output = nullptr;
    cli->input_length = 0;
    cli->output_length = 0;
}
