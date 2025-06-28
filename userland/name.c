#include "../syscall.h"

void main(void)
{
    char name[100];
    syscall(SYSCALL_ANSI_PRINT, "Whats your name? $ ", "magenta", "none");
    syscall(SYSCALL_READLINE, name, sizeof(name));
    syscall(SYSCALL_ANSI_PRINT, "Hello, ", "green", "none");
    syscall(SYSCALL_ANSI_PRINT, name, "cyan", "none");
    syscall(SYSCALL_ANSI_PRINT, "!\n", "cyan", "none");
    syscall(SYSCALL_EXIT);
}