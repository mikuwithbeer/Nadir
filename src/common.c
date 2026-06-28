#include "nadir/common.h"

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

bool nadir_common_string_to_i128(const char *input,
                                 nadir_i128_t *value) {
    auto result = (typeof(*value)) 0;
    auto negative = false;

    // Check for sign.
    if (*input == '-') {
        negative = true;
        input++;
    } else if (*input == '+') {
        input++;
    }

    // Loop through each character in the string.
    while (*input >= '0' && *input <= '9') {
        const auto digit = *input - '0';

        if (negative) {
            // Check for underflow.
            if (result < (NADIR_I128_MIN + digit) / 10) {
                return false;
            }

            result = result * 10 - digit;
        } else {
            // Check for overflow.
            if (result > (NADIR_I128_MAX - digit) / 10) {
                return false;
            }

            result = result * 10 + digit;
        }

        ++input;
    }

    // Check if we reached the end of the string.
    if (*input != '\0') {
        return false;
    }

    *value = result;
    return true;
}
