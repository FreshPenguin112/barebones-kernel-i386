#include "string_utils.h"
#include <stdbool.h>
size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void strcpy(char *dest, const char *src)
{
    while ((*dest++ = *src++))
        ;
}

    // Custom strchr implementation
    char *strchr(const char *s, int c)
    {
        while (*s)
        {
            if (*s == (char)c)
                return (char *)s;
            s++;
        }
        return 0;
    }

    // Custom strcat implementation
    char *strcat(char *dest, const char *src)
    {
        char *d = dest;
        while (*d) d++;
        while ((*d++ = *src++));
        return dest;
    }

    // Custom strncpy implementation
    char *strncpy(char *dest, const char *src, size_t n)
    {
        size_t i;
        for (i = 0; i < n && src[i]; i++)
            dest[i] = src[i];
        for (; i < n; i++)
            dest[i] = '\0';
        return dest;
    }

    // Minimal hex formatting for hexdump (replaces snprintf)
    void hex_byte_to_str(unsigned char byte, char *out)
    {
        const char hex[] = "0123456789ABCDEF";
        out[0] = hex[(byte >> 4) & 0xF];
        out[1] = hex[byte & 0xF];
        out[2] = ' ';
        out[3] = '\0';
    }

// Simple tokenizer that works with a single delimiter
static char *next_token = 0;

char *strtok(char *str, const char delimiter)
{
    char *token_start;

    if (str != 0)
    {
        next_token = str;
    }

    if (next_token == 0)
    {
        return 0;
    }

    token_start = next_token;
    while (*next_token)
    {
        if (*next_token == delimiter)
        {
            *next_token = '\0';
            next_token++;
            return token_start;
        }
        next_token++;
    }

    next_token = 0;
    return token_start;
}

void itoa(int32_t value, char *str, int base)
{
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char *ptr = str, *ptr1, tmp_char;
    uint32_t uvalue;
    bool negative = false;

    // Handle zero explicitly
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    // Record sign and work with unsigned absolute
    if (base == 10 && value < 0) {
        negative = true;
        uvalue = (uint32_t)(- (int64_t)value);
    } else {
        uvalue = (uint32_t)value;
    }

    // Convert digits in reverse order
    while (uvalue > 0) {
        uint32_t digit = uvalue % base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[digit];
        uvalue /= base;
    }

    // Add sign if negative
    if (negative) {
        *ptr++ = '-';
    }
    *ptr = '\0';

    // Reverse the whole string
    ptr1 = str;
    for (char *end = ptr - 1; ptr1 < end; ++ptr1, --end) {
        tmp_char = *ptr1;
        *ptr1 = *end;
        *end = tmp_char;
    }
}


void utoa(uint32_t value, char *str, int base)
{
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    uint32_t tmp_value;

    if (base < 2 || base > 36)
    {
        *str = '\0';
        return;
    }

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    } while (value);

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


void ftoa(float value, char *str, int precision)
{
    char *ptr = str;

    // Handle negative numbers
    if (value < 0)
    {
        *ptr++ = '-';
        value = -value;
    }

    // Extract integer part
    int int_part = (int)value;
    value -= int_part;

    // Convert integer part to string
    itoa(int_part, ptr, 10);
    while (*ptr != '\0') // Move pointer to the end of the integer part
        ptr++;

    // Add decimal point
    *ptr++ = '.';

    // Convert fractional part to string
    for (int i = 0; i < precision; i++)
    {
        value *= 10;
        int digit = (int)value;
        *ptr++ = '0' + digit;
        value -= digit;
    }

    // Null-terminate the string
    *ptr = '\0';

    // Remove trailing zeros
    char *end = ptr - 1; // Start from the last character
    while (end > str && *end == '0') // Remove zeros
        end--;

    if (*end == '.') // Remove the decimal point if it's the last character
        end--;

    *(end + 1) = '\0'; // Null-terminate the string
}

// Very basic modf: works correctly for |value| < 2^31
static double modf(double value, double *iptr) {
    // Truncate toward zero via C cast (pulls in __fixsfsi or similar, though)
    long int_part = (long)value;
    *iptr = (double)int_part;
    return value - *iptr;
}


void dtoa(double value, char *str, int precision) {
    char *ptr = str;
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }

    // Extract integer part using FP arithmetic only
    double intpart_d;
    double frac = modf(value, &intpart_d);
    if (intpart_d < 1.0) {
        *ptr++ = '0';
    } else {
        // find highest power of 10 <= intpart_d
        double pow10 = 1.0;
        while (pow10 * 10.0 <= intpart_d) {
            pow10 *= 10.0;
        }
        // extract digits
        while (pow10 >= 1.0) {
            int digit = (int)(intpart_d / pow10);
            *ptr++ = '0' + digit;
            intpart_d -= digit * pow10;
            pow10 /= 10.0;
        }
    }

    *ptr++ = '.';

    // fractional digits
    for (int i = 0; i < precision; i++) {
        frac *= 10.0;
        int digit = (int)frac;
        *ptr++ = '0' + digit;
        frac -= digit;
    }

    *ptr = '\0';

    // trim trailing zeros
    char *end = ptr - 1;
    while (end > str && *end == '0') --end;
    if (*end == '.') --end;
    *(end + 1) = '\0';
}


int atoi(const char *str)
{
    int result = 0;
    int sign = 1;

    // Skip leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
        str++;

    // Handle optional sign
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert digits to integer
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

float atof(const char *str)
{
    float result = 0.0f;
    float sign = 1.0f;
    float divisor = 1.0f;

    // Skip leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
        str++;

    // Handle optional sign
    if (*str == '-')
    {
        sign = -1.0f;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert integer part
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10.0f + (*str - '0');
        str++;
    }

    // Convert fractional part
    if (*str == '.')
    {
        str++;
        while (*str >= '0' && *str <= '9')
        {
            divisor *= 10.0f;
            result += (*str - '0') / divisor;
            str++;
        }
    }

    return result * sign;
}

double atod(const char *str) {
    double result = 0.0;
    double sign = 1.0;
    double divisor = 1.0;

    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        ++str;
    }
    // Optional sign
    if (*str == '-') {
        sign = -1.0; ++str;
    } else if (*str == '+') {
        ++str;
    }

    // Integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        ++str;
    }

    // Fractional part
    if (*str == '.') {
        ++str;
        while (*str >= '0' && *str <= '9') {
            divisor *= 10.0;
            result += (*str - '0') / divisor;
            ++str;
        }
    }

    return result * sign;
}