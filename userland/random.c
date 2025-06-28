#include "../syscall.h"
#include "../string_utils.h"
#include <stdint.h>

void main(void) {
    /* 32‑bit unsigned */
    uint32_t u;
    syscall(SYSCALL_RANDOM_U32, &u);
    char buf[12];
    utoa(u, buf, 10);
    syscall(SYSCALL_PRINT, "U32: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");

    /* 32‑bit signed */
    int32_t i;
    syscall(SYSCALL_RANDOM_I32, &i);
    itoa(i, buf, 10);
    syscall(SYSCALL_PRINT, "I32: ");
    syscall(SYSCALL_PRINT, buf);
    syscall(SYSCALL_PRINT, "\n");

    /* double in [0,1) */
    double d;
    syscall(SYSCALL_RANDOM_DOUBLE, &d);
    // Assuming you have dtoa():
    char dbuf[32];
    dtoa(d, dbuf, 10);
    syscall(SYSCALL_PRINT, "Double: ");
    syscall(SYSCALL_PRINT, dbuf);
    syscall(SYSCALL_PRINT, "\n");
}
