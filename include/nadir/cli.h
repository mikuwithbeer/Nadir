#ifndef NADIR_CLI_H
#define NADIR_CLI_H

#include "nadir/common.h"

// [--------------------------------------------------------------] //
// > Constants                                                    < //
// [--------------------------------------------------------------] //

constexpr char NADIR_CLI_DEFAULT_OUTPUT[] = "out.bin";

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Command-line interface structure for the assembler.
 */
typedef struct {
    const char *input_file;
    const char *output_file;

    char *input; // Input buffer for the input file
    char *output; // Output buffer for the output file

    nadir_u64_t input_length;
    nadir_u64_t output_length;

    bool help;
    bool version;
} nadir_cli_t;

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Creates a new command-line interface structure with default values.
 *
 * @warning Allocates memory for the input and output buffers, which must be freed.
 */
nadir_cli_t nadir_cli_new(void);

/**
 * @brief Parses the command-line arguments.
 */
bool nadir_cli_parse(nadir_cli_t *cli,
                     int argc,
                     char **argv);

/**
 * @brief Loads the given input buffer.
 */
bool nadir_cli_read(nadir_cli_t *cli);

/**
 * @brief Writes the given output to the file.
 */
bool nadir_cli_write(nadir_cli_t *cli,
                     const nadir_list_t *output);

/**
 * @brief Frees the memory allocated for the command-line interface structure.
 */
void nadir_cli_close(nadir_cli_t *cli);

#endif //NADIR_CLI_H
