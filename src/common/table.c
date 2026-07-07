/**
 * @file table.c
 * @brief The table implementation.
 */

#include "nadir/common/table.h"

#include <stdlib.h>
#include <string.h>

// [--------------------------------------------------------------] //
// > Forward Declarations                                         < //
// [--------------------------------------------------------------] //

[[nodiscard]] static nadir_u64_t nadir_table_hash(const char *key,
                                                  nadir_u64_t length);

[[nodiscard]] static nadir_table_entry_t *nadir_table_find(nadir_table_entry_t *entries,
                                                           nadir_u64_t capacity,
                                                           const char *key,
                                                           nadir_u64_t length);

[[nodiscard]] static nadir_table_entry_t *nadir_table_grow(nadir_table_t *table);

// [--------------------------------------------------------------] //
// > Function Implementations                                     < //
// [--------------------------------------------------------------] //

nadir_table_t *nadir_table_new(const nadir_u64_t size) {
    nadir_table_t *table = malloc(sizeof(nadir_table_t));
    if (table == nullptr) {
        return nullptr;
    }

    table->length = 0;
    table->capacity = NADIR_TABLE_DEFAULT_CAPACITY;
    table->size = size;

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
                        const nadir_u64_t length,
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
    const auto entry = nadir_table_find(table->entries, table->capacity, key, length);
    if (entry->exists) {
        return false;
    }

    // Allocate memory for the new key and copy the data into it.
    char *new_key = malloc(length + 1);
    if (new_key != nullptr) {
        memcpy(new_key, key, length);
        new_key[length] = '\0';
    } else {
        return false;
    }

    // Allocate memory for the new value and copy the data into it.
    char *new_value = malloc(table->size);
    if (new_value != nullptr) {
        memcpy(new_value, value, table->size);
    } else {
        free(new_key);
        return false;
    }

    entry->key = new_key;
    entry->key_length = length;

    entry->value = new_value;
    entry->exists = true;

    ++table->length;
    return true;
}

void *nadir_table_fetch(const nadir_table_t *table,
                        const char *key,
                        const nadir_u64_t length) {
    const auto entry = nadir_table_find(table->entries, table->capacity, key, length);
    if (!entry->exists) {
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
            if (table->entries[index].exists) {
                free(table->entries[index].key);
                free(table->entries[index].value);
            }
        }

        free(table->entries);
        table->entries = nullptr;
    }

    free(table);
}

// [--------------------------------------------------------------] //
// > Internal Functions                                           < //
// [--------------------------------------------------------------] //

static nadir_u64_t nadir_table_hash(const char *key,
                                    const nadir_u64_t length) {
    // FNV-1a hash algorithm for 64-bit hashes.
    nadir_u64_t hash = 0xCBF29CE484222325;

    for (nadir_u64_t index = 0; index < length; ++index) {
        hash ^= (nadir_u64_t) (unsigned char) key[index];
        hash *= 0x100000001B3; // FNV-1a prime for 64-bit hashes
    }

    return hash;
}

static nadir_table_entry_t *nadir_table_find(nadir_table_entry_t *entries,
                                             const nadir_u64_t capacity,
                                             const char *key,
                                             const nadir_u64_t length) {
    const auto hash = nadir_table_hash(key, length);
    nadir_u64_t index = hash & capacity - 1;

    while (entries[index].exists) {
        if (entries[index].key_length == length && memcmp(key, entries[index].key, length) == 0) {
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

    for (nadir_u64_t index = 0; index < table->capacity; index++) {
        const auto old_entry = &table->entries[index];

        // Rehash the old entry into the new table.
        if (old_entry->exists) {
            const auto new_slot = nadir_table_find(
                new_entries,
                new_capacity,
                old_entry->key,
                old_entry->key_length);

            *new_slot = *old_entry;
        }
    }

    free(table->entries); // Free the old entries array

    table->capacity = new_capacity;
    return new_entries;
}
