#include "../syscall.h"

void _start(void) {
    syscall(SYSCALL_PRINT, (void*)"Hello, world!\n");
    return;
}