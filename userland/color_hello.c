#include "../syscall.h"

void _start(void)
{
    syscall(SYSCALL_ANSI_PRINT, "Hello, World!\n", "yellow", "none");
    return;
}