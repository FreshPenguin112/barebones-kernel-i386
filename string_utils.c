#include "string_utils.h"

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

void itoa(int value, char *str, int base)
{
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

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

    // Apply negative sign for base 10
    if (tmp_value < 0 && base == 10)
        *ptr++ = '-';

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