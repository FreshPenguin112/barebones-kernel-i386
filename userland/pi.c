#include "../syscall.h"
#include "../string_utils.h"

// Calculate arctan(x) using its Taylor series
static double arctan(double x, int iterations)
{
    double result = 0.0;
    double term = x;
    int sign = 1;

    for (int i = 1; i <= iterations; i += 2)
    {
        result += sign * term / i;
        term *= x * x; // x^(2n+1)
        sign = -sign;  // Alternate signs
    }

    return result;
}

void _start(void)
{   
    syscall(SYSCALL_PRINT, "this is not very accurate btw, only up to like 5 digits maybe\n");
    char input[16];
    syscall(SYSCALL_PRINT, "Enter the number of digits of pi: ");
    syscall(SYSCALL_READLINE, input, sizeof(input));

    // Convert input to an integer
    int digits = atoi(input);
    if (digits <= 0 || digits > 15) // Limit precision to avoid buffer overflow
    {
        syscall(SYSCALL_PRINT, "Invalid input. Please enter a number between 1 and 15.\n");
        return;
    }

    // Calculate pi using the Machin-like formula
    int iterations = digits * 10; // Adjust iterations based on requested precision
    double pi = 4.0 * (4.0 * arctan(1.0 / 5.0, iterations) - arctan(1.0 / 239.0, iterations));

    // Round pi to the requested precision
    double scale = 1.0;
    for (int i = 0; i < digits; i++)
        scale *= 10.0;
    pi = ((long long)(pi * scale + 0.5)) / scale;

    // Convert pi to a string with the requested precision
    char buf[32];
    ftoa(pi, buf, digits);

    // Print the result
    syscall(SYSCALL_PRINT, "Calculated value of pi: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");

    // Exit the program
    return;
}