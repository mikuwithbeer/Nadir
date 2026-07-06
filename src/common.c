/**
 * @file common.c
 * @brief The common implementation.
 */

#include "nadir/common.h"

#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

[[nodiscard]] static nadir_u64_t nadir_table_hash(const char *key);

[[nodiscard]] static nadir_table_entry_t *nadir_table_find(nadir_table_entry_t *entries,
                                                           nadir_u64_t capacity,
                                                           const char *key);

[[nodiscard]] static nadir_table_entry_t *nadir_table_grow(nadir_table_t *table);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

bool nadir_i128_decode_base10(const char *input,
                              nadir_i128_t *value) {
    nadir_i128_t result = 0;

    // Handle optional sign.
    bool negative = false;
    if (*input == '-') {
        negative = true;
        ++input;
    } else if (*input == '+') {
        ++input;
    }

    // Parse each digit and accumulate the result.
    while (true) {
        int digit;
        if (*input >= '0' && *input <= '9') {
            digit = *input - '0';
        } else if (*input == '\0') {
            break; // End of string
        } else {
            return false; // Invalid character encountered
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

        ++input;
    }

    *value = result;
    return true;
}

bool nadir_i128_decode_base16(const char *input,
                              nadir_i128_t *value) {
    nadir_i128_t result = 0;

    // Handle optional sign.
    bool negative = false;
    if (*input == '-') {
        negative = true;
        ++input;
    } else if (*input == '+') {
        ++input;
    }

    // Parse each digit and accumulate the result.
    while (true) {
        int digit;
        if (*input >= '0' && *input <= '9') {
            digit = *input - '0';
        } else if (*input >= 'A' && *input <= 'F') {
            digit = *input - 'A' + 10;
        } else if (*input >= 'a' && *input <= 'f') {
            digit = *input - 'a' + 10;
        } else if (*input == '\0') {
            break; // End of string
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

        ++input;
    }

    *value = result;
    return true;
}

nadir_list_t *nadir_list_new(const nadir_u64_t size) {
    nadir_list_t *list = malloc(sizeof(nadir_list_t));
    if (list == nullptr) {
        return nullptr;
    }

    list->length = 0;
    list->capacity = NADIR_LIST_DEFAULT_CAPACITY;
    list->size = size;

    void *items = calloc(list->capacity, list->size); // Allocate zero-initialized memory
    if (items == nullptr) {
        free(list);
        return nullptr;
    }

    list->items = items;
    return list;
}

bool nadir_list_append(nadir_list_t *list,
                       const void *item) {
    if (list->length >= list->capacity) {
        const auto new_capacity = list->capacity << 1; // Double the capacity
        const auto new_items = realloc(list->items, new_capacity * list->size);
        if (new_items == nullptr) {
            return false;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    // Copy the item into the list and increment the length.
    memcpy((nadir_u8_t *)list->items + (list->length++) * list->size, item, list->size);
    return true;
}

void *nadir_list_get(const nadir_list_t *list,
                     const nadir_u64_t index) {
    if (index >= list->length) {
        return nullptr;
    }

    // Return a pointer to the item at the given index.
    return (nadir_u8_t *) list->items + index * list->size;
}

void nadir_list_free(nadir_list_t *list) {
    if (list == nullptr) {
        return;
    }

    if (list->items != nullptr) {
        free(list->items);
        list->items = nullptr;
    }

    free(list);
}

nadir_table_t *nadir_table_new(const nadir_u64_t size) {
    nadir_table_t *table = malloc(sizeof(nadir_table_t));
    if (table == nullptr) {
        return nullptr;
    }

    table->length = 0;
    table->capacity = NADIR_TABLE_DEFAULT_CAPACITY;
    table->size = size;

    // Allocate zero-initialized memory for the entries.
    nadir_table_entry_t *entries = calloc(table->capacity, sizeof(nadir_table_entry_t));
    if (entries == nullptr) {
        free(table);
        return nullptr;
    }

    table->entries = entries;
    return table;
}

bool nadir_table_insert(nadir_table_t *table,
                        const char *key,
                        const void *value) {
    // Expand if load factor reaches to half the capacity.
    if (table->length >= table->capacity >> 1) {
        const auto new_entries = nadir_table_grow(table);
        if (!new_entries) {
            return false;
        }

        table->entries = new_entries;
    }

    // Find and locate the appropriate slot.
    const auto entry = nadir_table_find(table->entries, table->capacity, key);
    if (entry->is_used) {
        return false;
    }

    // Copy the key into the entry and mark it as used.
    strncpy(entry->key, key, NADIR_STRING_MAXIMUM - 1);
    entry->key[NADIR_STRING_MAXIMUM - 1] = '\0';
    entry->is_used = true;
    ++table->length;

    // Allocate memory for the new value and copy the data into it.
    const auto new_value = malloc(table->size);
    if (new_value) {
        memcpy(new_value, value, table->size);
    } else {
        return false;
    }

    entry->value = new_value;
    return true;
}

void *nadir_table_fetch(const nadir_table_t *table,
                        const char *key) {
    const auto entry = nadir_table_find(table->entries, table->capacity, key);
    if (!entry->is_used) {
        return nullptr;
    }

    return entry->value;
}

void nadir_table_free(nadir_table_t *table) {
    if (table == nullptr) {
        return;
    }

    if (table->entries != nullptr) {
        // Free the allocated values for each used entry in the table.
        for (nadir_u64_t index = 0; index < table->capacity; index++) {
            if (table->entries[index].is_used) {
                free(table->entries[index].value);
            }
        }

        free(table->entries);
        table->entries = nullptr;
    }

    free(table);
}

nadir_stack_t *nadir_stack_new(void) {
    const auto stack = calloc(1, sizeof(nadir_stack_t));
    if (stack == nullptr) {
        return nullptr;
    }

    return stack;
}

bool nadir_stack_push(nadir_stack_t *stack,
                      const nadir_i128_t value) {
    if (stack->length >= NADIR_STACK_MAXIMUM) {
        return false;
    }

    stack->data[stack->length++] = value;
    return true;
}

bool nadir_stack_pop(nadir_stack_t *stack,
                     nadir_i128_t *value) {
    if (stack->length == 0) {
        return false;
    }

    if (value) {
        *value = stack->data[--stack->length];
    } else {
        --stack->length;
    }

    return true;
}

void nadir_stack_free(nadir_stack_t *stack) {
    if (stack == nullptr) {
        return;
    }

    free(stack);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_u64_t nadir_table_hash(const char *key) {
    // FNV-1a hash algorithm for 64-bit hashes.
    nadir_u64_t hash = 0xCBF29CE484222325;

    for (auto pointer = key; *pointer != '\0'; pointer++) {
        hash ^= (nadir_u64_t) (unsigned char) *pointer;
        hash *= 0x100000001B3; // FNV-1a prime for 64-bit hashes
    }

    return hash;
}

static nadir_table_entry_t *nadir_table_find(nadir_table_entry_t *entries,
                                             const nadir_u64_t capacity,
                                             const char *key) {
    const auto hash = nadir_table_hash(key);
    nadir_u64_t index = hash & capacity - 1;

    while (entries[index].is_used) {
        if (strcmp(key, entries[index].key) == 0) {
            return &entries[index]; // Slot with matching key found
        }

        // Linear probe with bitwise wrap-around.
        index = index + 1 & capacity - 1;
    }

    return &entries[index]; // Empty slot found
}

static nadir_table_entry_t *nadir_table_grow(nadir_table_t *table) {
    const auto new_capacity = table->capacity << 1; // Double the capacity
    nadir_table_entry_t *new_entries = calloc(new_capacity, sizeof(nadir_table_entry_t));
    if (!new_entries) {
        return nullptr;
    }

    for (size_t index = 0; index < table->capacity; index++) {
        const auto old_entry = &table->entries[index];

        // Rehash the old entry into the new table.
        if (old_entry->is_used) {
            const auto new_slot = nadir_table_find(new_entries, new_capacity, old_entry->key);
            *new_slot = *old_entry;
        }
    }

    free(table->entries); // Free the old entries array

    table->capacity = new_capacity;
    return new_entries;
}
