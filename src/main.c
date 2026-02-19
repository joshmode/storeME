#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"

#define MAX_CMD_LEN 1024
#define LOG_FILE "wal.log"

// removes trailing newline from string
void trim_newline(char* s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int main() {
    printf("Thanks for using storeME! Follow me on github @joshmode\n");
    printf("Here are the commands you can use:\n");
    printf("  SET <key> <value>  - Store a key and its value\n");
    printf("  GET <key>          - Retrieve a value by its key\n");
    printf("  DELETE <key>       - Remove a key-value pair\n");
    printf("  EXIT               - Quit the program\n");
    printf("\n");

    // initialize database
    HashTable* store = hashtable_create(1024);
    if (!store) {
        fprintf(stderr, "failed to initialize database\n");
        return 1;
    }

    // recover state from log
    hashtable_recover(store, LOG_FILE);

    // open log file for appending
    FILE* log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("error opening log file");
        hashtable_destroy(store);
        return 1;
    }
    hashtable_set_log_file(store, log);

    char buffer[MAX_CMD_LEN];
    memset(buffer, 0, sizeof(buffer));

    // main command loop
    while (1) {
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }

        trim_newline(buffer);
        if (strlen(buffer) == 0) continue;

        // skip leading whitespace
        char *cmd_start = buffer;
        while (isspace((unsigned char)*cmd_start)) cmd_start++;
        if (*cmd_start == 0) continue;

        // isolate command
        char *cursor = cmd_start;
        while (*cursor && !isspace((unsigned char)*cursor)) cursor++;

        if (*cursor) {
            *cursor = 0;
            cursor++;
        }

        // uppercase command
        for (char *c = cmd_start; *c; ++c) {
            *c = toupper((unsigned char)*c);
        }

        if (strcmp(cmd_start, "EXIT") == 0) {
            printf("Goodbye!\n");
            break;
        }

        int is_set = (strcmp(cmd_start, "SET") == 0);
        int is_get = (strcmp(cmd_start, "GET") == 0);
        int is_delete = (strcmp(cmd_start, "DELETE") == 0);

        if (!is_set && !is_get && !is_delete) {
             printf("unknown command: %s\n", cmd_start);
             continue;
        }

        // find key
        char *key_start = cursor;
        while (*key_start && isspace((unsigned char)*key_start)) key_start++;

        if (*key_start == 0) {
             printf("usage: %s <key> ...\n", cmd_start);
             continue;
        }

        // find end of key
        cursor = key_start;
        while (*cursor && !isspace((unsigned char)*cursor)) cursor++;

        if (*cursor) {
            *cursor = 0;
            cursor++;
        }

        // execute command
        if (is_set) {
             char *val_start = cursor;
             while (*val_start && isspace((unsigned char)*val_start)) val_start++;

             hashtable_set(store, key_start, val_start, 1);
             printf("OK\n");

        } else if (is_get) {
             char* val = hashtable_get(store, key_start);
             if (val) {
                 printf("%s\n", val);
             } else {
                 printf("(nil)\n");
             }

        } else if (is_delete) {
             if (hashtable_delete(store, key_start, 1)) {
                 printf("OK\n");
             } else {
                 printf("(nil)\n");
             }
        }
    }

    if (log) fclose(log);
    hashtable_destroy(store);
    return 0;
}
