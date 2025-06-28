#include "../syscall.h"
#include "../string_utils.h"

void main(void)
{
    float seconds = (float)1 / 2; // Convert ms to seconds
    char buf[16];
    ftoa(seconds, buf, 10);
    syscall(SYSCALL_PRINT, "half: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");
    return;
}