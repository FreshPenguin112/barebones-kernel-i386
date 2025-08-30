#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdint.h>

// String utility functions
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
void strcpy(char *dest, const char *src);
char *strtok(char *str, const char delimiter);
void itoa(int32_t value, char *str, int base);
void utoa(uint32_t value, char *str, int base);
void ftoa(float value, char *str, int precision);
void dtoa(double value, char *str, int precision);
int atoi(const char *str);
float atof(const char *str);
double atod(const char *str);

// Custom replacements for kernel
char *strchr(const char *s, int c);
char *strcat(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
void hex_byte_to_str(unsigned char byte, char *out);


#endif