#define _POSIX_C_SOURCE 200809L
#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

// computes the hash value for a given string
unsigned long hash_string(const char *input) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// escapes special characters (newline, backslash) for persistence
static char* escape_string(const char* input) {
    if (!input) return NULL;
    size_t len = strlen(input);
    size_t new_len = 0;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' || input[i] == '\n') new_len += 2;
        else new_len++;
    }
    char* new_str = (char*)malloc(new_len + 1);
    if (!new_str) return NULL;

    char* p = new_str;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\') {
            *p++ = '\\';
            *p++ = '\\';
        } else if (input[i] == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else {
            *p++ = input[i];
        }
    }
    *p = '\0';
    return new_str;
}

// unescapes characters from the persistence log
static char* unescape_string(const char* input) {
    if (!input) return NULL;
    size_t len = strlen(input);
    char* new_str = (char*)malloc(len + 1);
    if (!new_str) return NULL;

    char* p = new_str;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' && i + 1 < len) {
            if (input[i+1] == '\\') {
                *p++ = '\\';
                i++;
            } else if (input[i+1] == 'n') {
                *p++ = '\n';
                i++;
            } else {
                *p++ = input[i];
            }
        } else {
            *p++ = input[i];
        }
    }
    *p = '\0';
    return new_str;
}

HashTable* hashtable_create(int capacity) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->capacity = capacity;
    table->buckets = (KeyValueNode**)calloc(capacity, sizeof(KeyValueNode*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    table->log_file = NULL;
    return table;
}

void hashtable_destroy(HashTable* table) {
    if (!table) return;

    for (int i = 0; i < table->capacity; i++) {
        KeyValueNode* entry = table->buckets[i];
        while (entry) {
            KeyValueNode* next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }
    free(table->buckets);
    free(table);
}

void hashtable_set(HashTable* table, const char* key, const char* value, int persist) {
    if (!table || !key || !value) return;

    unsigned long index = hash_string(key) % table->capacity;
    KeyValueNode* entry = table->buckets[index];

    // check if key exists
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // update value
            char* new_val = strdup(value);
            if (!new_val) return;

            free(entry->value);
            entry->value = new_val;

            // log if required
            if (persist && table->log_file) {
                char* escaped_val = escape_string(value);
                if (escaped_val) {
                    fprintf(table->log_file, "SET %s %s\n", key, escaped_val);
                    fflush(table->log_file);
                    free(escaped_val);
                }
            }
            return;
        }
        entry = entry->next;
    }

    // create new entry
    KeyValueNode* new_entry = (KeyValueNode*)malloc(sizeof(KeyValueNode));
    if (!new_entry) return;

    new_entry->key = strdup(key);
    new_entry->value = strdup(value);

    if (!new_entry->key || !new_entry->value) {
        free(new_entry->key);
        free(new_entry->value);
        free(new_entry);
        return;
    }

    // insert at head of bucket
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;

    // log if required
    if (persist && table->log_file) {
        char* escaped_val = escape_string(value);
        if (escaped_val) {
            fprintf(table->log_file, "SET %s %s\n", key, escaped_val);
            fflush(table->log_file);
            free(escaped_val);
        }
    }
}

char* hashtable_get(HashTable* table, const char* key) {
    if (!table || !key) return NULL;

    unsigned long index = hash_string(key) % table->capacity;
    KeyValueNode* entry = table->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

int hashtable_delete(HashTable* table, const char* key, int persist) {
    if (!table || !key) return 0;

    unsigned long index = hash_string(key) % table->capacity;
    KeyValueNode* entry = table->buckets[index];
    KeyValueNode* prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // remove node
            if (prev) {
                prev->next = entry->next;
            } else {
                table->buckets[index] = entry->next;
            }

            free(entry->key);
            free(entry->value);
            free(entry);

            // log if required
            if (persist && table->log_file) {
                fprintf(table->log_file, "DELETE %s\n", key);
                fflush(table->log_file);
            }
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    return 0;
}

void hashtable_set_log_file(HashTable* table, FILE* file) {
    if (table) {
        table->log_file = file;
    }
}

void hashtable_recover(HashTable* table, const char* filename) {
    if (!table || !filename) return;

    FILE* file = fopen(filename, "r");
    if (!file) return;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = 0;

        char *cmd = line;
        char *delimiter = strchr(cmd, ' ');
        if (!delimiter) continue;
        *delimiter = 0;

        char *key = delimiter + 1;
        char *value = NULL;

        delimiter = strchr(key, ' ');
        if (delimiter) {
            *delimiter = 0;
            value = delimiter + 1;
        } else {
            value = "";
        }

        if (strcmp(cmd, "SET") == 0) {
             char* unescaped_val = unescape_string(value);
             if (unescaped_val) {
                 hashtable_set(table, key, unescaped_val, 0);
                 free(unescaped_val);
             }
        } else if (strcmp(cmd, "DELETE") == 0) {
             hashtable_delete(table, key, 0);
        }
    }
    fclose(file);
}
