#include "../syscall.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>

// (insert fact, ipow, ldiv_ld, ldexp_ld, sqrt_ld, dtoa_ld, modfl here)

static long double chud_pi(int terms) {
   long double sum = 0.0L;
   for (int k = 0; k < terms; k++) {
       uint64_t num   = fact(6*k);
       uint64_t den1  = fact(3*k);
       uint64_t den2  = fact(k);
       long double term = (long double)num
           / ((long double)den1 * den1 * den2 * den2 * den2);
       long double mul = 13591409.0L + 545140134.0L*k;
       long double pow = ldexp_ld(1.0L, - (3*k+1) * 10 );
       // 640320^3 = 2^10 * 5^? * ... approximated by ldexp+mul
       sum += (k & 1 ? -1.0L : 1.0L) * term * mul * pow;
   }
   return 1.0L/(12.0L * sum);
}

void _start(void) {
   syscall(SYSCALL_PRINT, "Digits (1-19)? ");
   char buf[8]; syscall(SYSCALL_READLINE, buf, sizeof(buf));
   int D = atoi(buf);
   if (D<1||D>19) {
       syscall(SYSCALL_PRINT,"1-19 only\n"); return;
   }
   int terms = (D+13)/14;
   long double pi = chud_pi(terms);
   char out[64];
   dtoa_ld(pi, out, D);
   syscall(SYSCALL_PRINT, out);
   syscall(SYSCALL_PRINT, "\n");
}