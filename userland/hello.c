#include "../syscall.h"

void main(void)
{
    syscall(SYSCALL_PRINT, "Hello, World!\n");
}