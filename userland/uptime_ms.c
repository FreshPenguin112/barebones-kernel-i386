#include "../syscall.h"
#include "../string_utils.h"

void main(void)
{
    // syscall(SYSCALL_PRINT, "this one is broken lol\n");
    uint32_t ms;
    syscall(SYSCALL_UPTIME_MS, &ms);
    char buf[16];
    itoa(ms, buf, 10);
    syscall(SYSCALL_PRINT, "Uptime in milliseconds: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");
    return;
}