#include "string_utils.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Simple tokenizer that works with a single delimiter
static char* next_token = 0;

char* strtok(char* str, const char delimiter) {
    char* token_start;
    
    if (str != 0) {
        next_token = str;
    }
    
    if (next_token == 0) {
        return 0;
    }

    token_start = next_token;
    while (*next_token) {
        if (*next_token == delimiter) {
            *next_token = '\0';
            next_token++;
            return token_start;
        }
        next_token++;
    }

    next_token = 0;
    return token_start;
}