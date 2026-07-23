/**
 * @file number.c
 * @brief The number implementation.
 */

#include "nadir/common/number.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

bool nadir_i128_decode_base2(const char *input,
                             const nadir_u64_t length,
                             nadir_i128_t *value) {
    nadir_i128_t result = 0;
    nadir_u64_t index = 1; // Skip the prefix

    // Handle optional sign.
    bool negative = false;
    if (index < length && input[index] == '-') {
        negative = true;
        ++index;
    } else if (index < length && input[index] == '+') {
        ++index;
    }

    // Parse each bit and accumulate the result.
    for (; index < length; ++index) {
        int bit;

        const char character = input[index];
        if (character == '0') {
            bit = 0;
        } else if (character == '1') {
            bit = 1;
        } else if (character == '_') {
            continue; // Ignore underscores.
        } else {
            return false; // Invalid character.
        }

        if (negative) {
            // Check for underflow.
            if (result < (NADIR_I128_MINIMUM + bit) / 2) {
                return false;
            }

            result = (result << 1) - bit;
        } else {
            // Check for overflow.
            if (result > (NADIR_I128_MAXIMUM - bit) / 2) {
                return false;
            }

            result = (result << 1) + bit;
        }
    }

    *value = result;
    return true;
}

bool nadir_i128_decode_base10(const char *input,
                              const nadir_u64_t length,
                              nadir_i128_t *value) {
    nadir_i128_t result = 0;
    nadir_u64_t index = 0;

    // Handle optional sign.
    bool negative = false;
    if (index < length && input[index] == '-') {
        negative = true;
        ++index;
    } else if (index < length && input[index] == '+') {
        ++index;
    }

    // Parse each digit and accumulate the result.
    for (; index < length; ++index) {
        int digit;

        const char character = input[index];
        if (character >= '0' && character <= '9') {
            digit = character - '0';
        } else if (character == '_') {
            continue; // Ignore underscores
        } else {
            return false; // Invalid character
        }

        if (negative) {
            // Check for underflow.
            if (result < (NADIR_I128_MINIMUM + digit) / 10) {
                return false;
            }

            result = result * 10 - digit;
        } else {
            // Check for overflow.
            if (result > (NADIR_I128_MAXIMUM - digit) / 10) {
                return false;
            }

            result = result * 10 + digit;
        }
    }

    *value = result;
    return
            true;
}

bool nadir_i128_decode_base16(const char *input,
                              const nadir_u64_t length,
                              nadir_i128_t *value) {
    nadir_i128_t result = 0;
    nadir_u64_t index = 1; // Skip the prefix

    // Handle optional sign.
    bool negative = false;
    if (index < length && input[index] == '-') {
        negative = true;
        ++index;
    } else if (index < length && input[index] == '+') {
        ++index;
    }

    // Parse each digit and accumulate the result.
    for (; index < length; ++index) {
        int digit;

        const char character = input[index];
        if (character >= '0' && character <= '9') {
            digit = character - '0';
        } else if (character >= 'A' && character <= 'F') {
            digit = character - 'A' + 10;
        } else if (character >= 'a' && character <= 'f') {
            digit = character - 'a' + 10;
        } else if (character == '_') {
            continue; // Ignore underscores
        } else {
            return false; // Invalid character encountered
        }

        if (negative) {
            // Check for underflow.
            if (result < (NADIR_I128_MINIMUM + digit) / 16) {
                return false;
            }

            result = result * 16 - digit;
        } else {
            // Check for overflow.
            if (result > (NADIR_I128_MAXIMUM - digit) / 16) {
                return false;
            }

            result = result * 16 + digit;
        }
    }

    *value = result;
    return true;
}
