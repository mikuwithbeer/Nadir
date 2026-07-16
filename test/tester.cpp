/**
 * @file tester.cpp
 * @brief The entry point for the testing application.
 *
 * This file contains the main process that contains the entire
 * testing process, including command-line argument parsing,
 * executing the assembler, and validating the output.
 */

#include <cstdlib>
#include <filesystem>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <expected>

import common;

namespace fs = std::filesystem;

// [--------------------------------------------------------------] //
// > Global Variables                                             < //
// [--------------------------------------------------------------] //

static std::string_view assembler;
static std::string_view test_mode;
static std::string_view source_file;
static std::string_view output_file;

static int expected_output = -1;
static std::optional<std::string_view> expected_file = std::nullopt;

static ProcessResult *process_result = nullptr;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

enum ProcessState {
    STATE_CONTINUE,
    STATE_EXIT,
    STATE_ERROR,
};

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

static ProcessState process_cli(int argc, char **argv);

static ProcessState process_assembler();

static ProcessState process_validation();

static void process_cleanup();

// [--------------------------------------------------------------] //
// > Main Function                                                < //
// [--------------------------------------------------------------] //

int main(const int argc, char **argv) {
    auto state = process_cli(argc, argv);

    if (state == STATE_CONTINUE) {
        state = process_assembler();
    }

    if (state == STATE_CONTINUE) {
        state = process_validation();
    }

    process_cleanup();
    return state == STATE_EXIT ? EXIT_SUCCESS : EXIT_FAILURE;
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static ProcessState process_cli(const int argc, char **argv) {
    std::span args(argv, argc);

    if (args.size() < 5) {
        std::println(stderr, "usage: {} <assembler> <test_mode> <source_file> <output_file> [expected_file]", args[0]);
        return STATE_ERROR;
    }

    assembler = args[1];
    test_mode = args[2];
    source_file = args[3];
    output_file = args[4];

    if (args.size() > 5) {
        expected_file = args[5];
    }

    if (test_mode == "success") {
        expected_output = EXIT_SUCCESS;
    } else if (test_mode == "failure") {
        expected_output = EXIT_FAILURE;
    } else {
        std::println(stderr, "error: invalid test mode '{}'", test_mode);
        return STATE_ERROR;
    }

    std::println("testing for mode: {}: {}", test_mode, source_file);
    return STATE_CONTINUE;
}

static ProcessState process_assembler() {
    const std::string command = std::format(R"("{}" -i "{}" -o "{}" 2>&1)", assembler, source_file, output_file);
    process_result = new ProcessResult();

    try {
        *process_result = execute_process(command);
    } catch (const std::exception &exception) {
        std::println(stderr, "error: failed to execute assembler: {}", exception.what());
        return STATE_ERROR;
    }

    return STATE_CONTINUE;
}

static ProcessState process_validation() {
    if (expected_output == EXIT_SUCCESS) {
        if (process_result->code != 0) {
            std::println(stderr,
                         "error: assembler failed to compile: {}\n  code: {}\n  message: {}",
                         source_file,
                         process_result->code,
                         process_result->written);

            return STATE_ERROR;
        }

        if (!fs::exists(output_file)) {
            std::println(stderr, "error: output file not found: {}", output_file);
            return STATE_ERROR;
        }

        if (!expected_file || !fs::exists(*expected_file)) {
            std::println(stderr, "error: expected validation file not found: {}", expected_file.value_or("None"));
            return STATE_ERROR;
        }

        auto actual_binary_result = read_binary_file(output_file);
        auto expected_binary_result = read_binary_file(*expected_file);

        if (!actual_binary_result) {
            std::println(stderr, "{}", actual_binary_result.error());
            return STATE_ERROR;
        }

        if (!expected_binary_result) {
            std::println(stderr, "{}", expected_binary_result.error());
            return STATE_ERROR;
        }

        const auto &actual_bytes = *actual_binary_result;
        const auto &expected_bytes = *expected_binary_result;

        if (actual_bytes == expected_bytes) {
            std::println("success: output binary matches the expected validation binary:");
            return STATE_EXIT;
        }

        std::println(stderr, "  actual: {}", format_as_hexadecimal(actual_bytes));
        std::println(stderr, "  expected: {}", format_as_hexadecimal(expected_bytes));

        return STATE_ERROR;
    }

    if (expected_output == EXIT_FAILURE) {
        if (process_result->code == 0) {
            std::println(stderr, "error: assembler succeeded unexpectedly on invalid source");
            return STATE_ERROR;
        }

        std::println("success: assembler failed as expected:\n  code: {}\n  message: {}",
                     process_result->code,
                     process_result->written);

        return STATE_EXIT;
    }

    return STATE_ERROR;
}

static void process_cleanup() {
    if (process_result != nullptr) {
        delete process_result;
        process_result = nullptr;
    }
}
