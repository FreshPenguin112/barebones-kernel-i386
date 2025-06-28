#include "../syscall.h"
#include "../string_utils.h"

void main(void) {
    uint32_t n, num1 = 0, num2 = 1, nextNum;
    char buffer[100];
    syscall(SYSCALL_PRINT, "Number of fibonacci numbers to generate? \n");
    syscall(SYSCALL_READLINE, buffer, sizeof(buffer));
    uint32_t len = (uint32_t)atoi(buffer);
    if (len > 0) {
        n = len;
    } else {
        syscall(SYSCALL_PRINT, "Invalid input. Please enter a positive integer.\n");
        return;
    }

    if (len <= 48) {
        n = len;
    } else {
        syscall(SYSCALL_PRINT, "Inputted length is too high for precision.\n");
        return;
    }

    syscall(SYSCALL_PRINT, "Fibonacci Series: \n");

    for (uint32_t _ = 1; _ <= n; ++_) {
        utoa(num1, buffer, 10);
        syscall(SYSCALL_PRINT, buffer);
        syscall(SYSCALL_PRINT, "\n");
        nextNum = num1 + num2;
        num1 = num2;
        num2 = nextNum;
    }
    syscall(SYSCALL_PRINT, "\n");
}