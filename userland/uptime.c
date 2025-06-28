#include "../syscall.h"
#include "../string_utils.h"

void main(void)
{
    // syscall(SYSCALL_PRINT, "this one is broken lol\n");
    uint32_t ms;
    syscall(SYSCALL_UPTIME_MS, &ms);
    uint32_t seconds = ms / 1000; // Convert ms to seconds
    char buf[16];
    itoa(seconds, buf, 10);
    syscall(SYSCALL_PRINT, "Uptime in seconds: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");
    return;
}