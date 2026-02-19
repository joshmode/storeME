#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/hashtable.h"

// clears the log file
void clear_log(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (f) fclose(f);
}

void test_basic_operations() {
    HashTable* ht = hashtable_create(10);
    assert(ht != NULL);

    hashtable_set(ht, "key1", "value1", 0);
    char* val = hashtable_get(ht, "key1");
    assert(val != NULL);
    assert(strcmp(val, "value1") == 0);

    // update existing key
    hashtable_set(ht, "key1", "value2", 0);
    val = hashtable_get(ht, "key1");
    assert(val != NULL);
    assert(strcmp(val, "value2") == 0);

    // delete key
    int deleted = hashtable_delete(ht, "key1", 0);
    assert(deleted == 1);
    val = hashtable_get(ht, "key1");
    assert(val == NULL);

    // delete non-existent key
    deleted = hashtable_delete(ht, "key1", 0);
    assert(deleted == 0);

    hashtable_destroy(ht);
    printf("test_basic_operations passed.\n");
}

void test_collisions() {
    HashTable* ht = hashtable_create(1); // force collisions
    assert(ht != NULL);

    hashtable_set(ht, "key1", "value1", 0);
    hashtable_set(ht, "key2", "value2", 0);
    hashtable_set(ht, "key3", "value3", 0);

    assert(strcmp(hashtable_get(ht, "key1"), "value1") == 0);
    assert(strcmp(hashtable_get(ht, "key2"), "value2") == 0);
    assert(strcmp(hashtable_get(ht, "key3"), "value3") == 0);

    hashtable_delete(ht, "key2", 0);
    assert(hashtable_get(ht, "key2") == NULL);
    assert(strcmp(hashtable_get(ht, "key1"), "value1") == 0);
    assert(strcmp(hashtable_get(ht, "key3"), "value3") == 0);

    hashtable_destroy(ht);
    printf("test_collisions passed.\n");
}

void test_persistence_escaping() {
    const char* log_file = "test_wal_esc.log";
    clear_log(log_file);

    HashTable* ht = hashtable_create(10);
    FILE* file = fopen(log_file, "a");
    hashtable_set_log_file(ht, file);

    // test special characters
    const char* weird_val = "line1\nline2\\backslashed";
    hashtable_set(ht, "weird", weird_val, 1);

    // verify in memory
    assert(strcmp(hashtable_get(ht, "weird"), weird_val) == 0);

    fclose(file);
    hashtable_destroy(ht);

    // recover from log
    ht = hashtable_create(10);
    hashtable_recover(ht, log_file);

    char* val = hashtable_get(ht, "weird");
    assert(val != NULL);
    if (strcmp(val, weird_val) != 0) {
        printf("FAIL: recovered value '%s' does not match original '%s'\n", val, weird_val);
        exit(1);
    }

    hashtable_destroy(ht);
    remove(log_file);
    printf("test_persistence_escaping passed.\n");
}

void test_edge_cases() {
    HashTable* ht = hashtable_create(10);

    // null inputs
    hashtable_set(ht, NULL, "val", 0);
    hashtable_set(ht, "key", NULL, 0);
    assert(hashtable_get(ht, NULL) == NULL);
    assert(hashtable_delete(ht, NULL, 0) == 0);

    // empty strings
    hashtable_set(ht, "", "empty", 0);
    assert(strcmp(hashtable_get(ht, ""), "empty") == 0);

    hashtable_set(ht, "emptyval", "", 0);
    assert(strcmp(hashtable_get(ht, "emptyval"), "") == 0);

    hashtable_destroy(ht);
    printf("test_edge_cases passed.\n");
}

int main() {
    test_basic_operations();
    test_collisions();
    test_persistence_escaping();
    test_edge_cases();
    printf("all unit tests passed.\n");
    return 0;
}
