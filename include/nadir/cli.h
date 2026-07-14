#ifndef NADIR_CLI_H
#define NADIR_CLI_H

/**
 * @file cli.h
 * @brief The CLI interface.
 *
 * This file defines the command-line interface structure and related
 * constants for the assembler.
 */

#include "nadir/common/list.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr char NADIR_CLI_DEFAULT_OUTPUT[] = "out.bin"; // Default output file

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Command-line interface structure for the assembler.
 */
typedef struct {
    nadir_arena_t *arena;

    const char *input_file;
    const char *output_file;

    bool help;
    bool version;
    bool dry_run;
} nadir_cli_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new command-line interface structure with the given arena.
 */
[[nodiscard]] nadir_cli_t nadir_cli_new(nadir_arena_t *arena);

/**
 * @brief Parses the command-line arguments and populates the structure.
 */
[[nodiscard]] bool nadir_cli_parse(nadir_cli_t *cli,
                                   int argc,
                                   char **argv);

/**
 * @brief Prints the help message to the standard output.
 */
void nadir_cli_help(void);

/**
 * @brief Prints the version information to the standard output.
 */
void nadir_cli_version(void);

/**
 * @brief Writes the given output to the file.
 */
[[nodiscard]] bool nadir_cli_write(const nadir_cli_t *cli,
                                   const nadir_list_t *output);

/**
 * @brief Closes the command-line interface and frees the associated resources.
 */
void nadir_cli_close(nadir_cli_t *cli);

#endif //NADIR_CLI_H
