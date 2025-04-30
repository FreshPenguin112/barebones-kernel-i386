#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

// String utility functions
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
void strcpy(char *dest, const char *src);
char *strtok(char *str, const char delimiter);
void itoa(int value, char *str, int base);

#endif