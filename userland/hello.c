#include "../syscall.h"

void _start(void)
{
    syscall(SYSCALL_PRINT, "Hello, World!\n");
    return;
}