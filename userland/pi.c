// pi.c -- bare-metal i386, only gcc built-ins + your string_utils
#include "../syscall.h"
#include "../string_utils.h"
#include <float.h>
#include <stdint.h>

// Newton-Raphson sqrt for long double (no math.h) :contentReference[oaicite:6]{index=6}
static long double sqrt_newton(long double N, long double eps) {
   long double x = (N > 1.0L ? N : 1.0L);
   while (1) {
       long double next = 0.5L * (x + N / x);
       if ((next > x ? next - x : x - next) < eps) break;
       x = next;
   }
   return x;
}

// Gauss-Legendre ? to 'digits' decimals :contentReference[oaicite:7]{index=7}
static long double compute_pi(int digits) {
   long double eps = LDBL_EPSILON * 10.0L;       // stop threshold
   long double a = 1.0L;
   long double b = 1.0L / sqrt_newton(2.0L, eps);
   long double t = 0.25L;
   long double p = 1.0L;

   for (int iter = 0; iter < 5; iter++) {
       long double an = 0.5L * (a + b);
       long double bn = sqrt_newton(a * b, eps);
       long double delta = a - an;
       t -= p * delta * delta;
       p  *= 2.0L;
       a = an; b = bn;
       if (delta < eps) break;
   }
   return (a + b) * (a + b) / (4.0L * t);
}



// Entry point
void _start(void) {
   syscall(SYSCALL_PRINT,
     "Compute pi via Gauss-Legendre (quadratic convergence).\n");
   char input[8];
   syscall(SYSCALL_PRINT, "Digits (1-100)? ");
   syscall(SYSCALL_READLINE, input, sizeof(input));
   int digits = atoi(input);
   if (digits < 1 || digits > 100) {
       syscall(SYSCALL_PRINT, "Enter 1-100.\n");
       return;
   }

   long double pi = compute_pi(digits);
   char buf[64];
   dtoa(pi, buf, digits);

   syscall(SYSCALL_PRINT, buf);
   syscall(SYSCALL_PRINT, "\n");
}