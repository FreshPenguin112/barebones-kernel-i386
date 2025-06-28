#include "../syscall.h"

void main(void)
{
    syscall(SYSCALL_ANSI_PRINT, "Hello, World!\n", "yellow", "none");
    return;
}