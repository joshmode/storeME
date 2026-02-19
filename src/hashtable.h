#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>

// structure to hold key-value pairs
typedef struct KeyValueNode {
    char* key;
    char* value;
    struct KeyValueNode* next;
} KeyValueNode;

// main hashtable structure
typedef struct HashTable {
    int capacity;
    KeyValueNode** buckets;
    FILE* log_file;
} HashTable;

// initializes a new hashtable
HashTable* hashtable_create(int capacity);

// frees the hashtable and its resources
void hashtable_destroy(HashTable* table);

// sets a key-value pair in the hashtable.
// if persist is true, the operation is logged to the file.
void hashtable_set(HashTable* table, const char* key, const char* value, int persist);

// retrieves a value by its key. returns null if not found.
char* hashtable_get(HashTable* table, const char* key);

// removes a key-value pair from the hashtable.
// returns 1 if successful, 0 if not found.
int hashtable_delete(HashTable* table, const char* key, int persist);

// recovers state from the log file.
void hashtable_recover(HashTable* table, const char* filename);

// sets the log file for persistence.
void hashtable_set_log_file(HashTable* table, FILE* file);

#endif
