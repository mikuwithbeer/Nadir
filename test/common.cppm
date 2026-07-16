/**
 * @file common.cppm
 * @brief Common helper functions for the testing application.
 */

module;

#include <array>
#include <cstdio>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <span>
#include <string>
#include <vector>
#include <stdexcept>

#ifdef _WIN32
#define PLATFORM_POPEN _popen
#define PLATFORM_PCLOSE _pclose
#else
#include <sys/wait.h>
#define PLATFORM_POPEN popen
#define PLATFORM_PCLOSE pclose
#endif

export module common;

namespace fs = std::filesystem;

// [--------------------------------------------------------------] //
// > Data Structures                                              < //
// [--------------------------------------------------------------] //

/**
 * @brief Result structure for executing a process.
 */
export struct ProcessResult {
    std::string written; // Captured output from the process
    int code = -1; // Exit code of the process
};

// [--------------------------------------------------------------] //
// > Function Declarations                                        < //
// [--------------------------------------------------------------] //

/**
 * @brief Executes a command in a subprocess and captures its output.
 */
export ProcessResult execute_process(const std::string &command) {
    ProcessResult result{};

    // Open a pipe to the command for reading its output.
    std::FILE *pipe = PLATFORM_POPEN(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error(std::format("popen() failed to create process for command: {}", command));
    }

    // Ensure the pipe is closed and the exit code is captured.
    struct PipeGuard {
        std::FILE *pipe_handle;
        ProcessResult &result_reference;

        ~PipeGuard() {
            if (pipe_handle) {
                auto raw_exit_code = PLATFORM_PCLOSE(pipe_handle);
#ifndef _WIN32
                result_reference.code = WIFEXITED(raw_exit_code) ? WEXITSTATUS(raw_exit_code) : -1;
#else
                result_reference.code = raw_exit_code;
#endif
            }
        }
    } guard{pipe, result};

    std::array<char, 1 << 10> buffer{};
    while (std::fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result.written.append(buffer.data());
    }

    return result;
}

/**
 * @brief Reads the contents of a binary file into a vector of bytes.
 */
export std::expected<std::vector<std::uint8_t>, std::string> read_binary_file(const fs::path &file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return std::unexpected(std::format("error: failed to open file: {}", file_path.string()));
    }

    // Determine the size of the file by seeking to the end.
    const std::streamsize size = file.tellg();
    if (size < 0) {
        return std::unexpected(std::format("error: failed to determine size of file: {}", file_path.string()));
    }

    std::vector<std::uint8_t> buffer(static_cast<std::size_t>(size));
    file.seekg(0, std::ios::beg);

    if (file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        return buffer;
    }

    return std::unexpected(std::format("error: failed to read file: {}", file_path.string()));
}

/**
 * @brief Formats a span of bytes as a hexadecimal string.
 */
export std::string format_as_hexadecimal(const std::span<const std::uint8_t> data) {
    // Reserve enough space for the hexadecimal representation.
    std::string hex_string;
    hex_string.reserve(data.size() * 3); // 2 characters per byte + 1 space

    for (const std::uint8_t byte: data) {
        std::format_to(std::back_inserter(hex_string), "{:02X} ", byte);
    }

    if (!hex_string.empty()) {
        hex_string.pop_back(); // Remove the trailing space
    }

    return hex_string;
}
