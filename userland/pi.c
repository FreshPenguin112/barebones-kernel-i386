// pi.c -- bare-metal i386, only gcc built-ins + your string_utils
#include "../syscall.h"
#include "../string_utils.h"
#include <float.h>
#include <stdint.h>

// Newton-Raphson sqrt for long double (no math.h) :contentReference[oaicite:6]{index=6}
// Compute sqrt(N) via Newton-Raphson with stopping criterion for 'digits' decimals.
// Supports N >= 0. Returns 0 for N == 0; no handling for N < 0.
static long double sqrt_newton(long double N, int digits) {
   if (N == 0.0L) return 0.0L;
   long double eps = 0.5L;
   // eps = 0.5 * 10^(-digits)
   for (int i = 0; i < digits; i++) eps *= 0.1L;      // :contentReference[oaicite:4]{index=4}
   long double x = (N > 1.0L ? N : 1.0L);
   for (int iter = 0; iter < 150; iter++) {         // cap iterations
       long double next = 0.5L * (x + N / x);
       long double diff = (next > x ? next - x : x - next);
       if (diff < eps) break;
       x = next;
   }
   return x;
}


// Gauss-Legendre ? to 'digits' decimals :contentReference[oaicite:7]{index=7}
// precompute 10^(-digits)
static long double threshold_for_digits(int digits) {
   long double t = 1.0L;
   for (int i = 0; i < digits; i++)
       t *= 0.1L;    // repeated multiplication by 0.1 :contentReference[oaicite:4]{index=4}
   return t;
}

static long double compute_pi(int digits) {
   long double a = 1.0L;
   long double b = 1.0L / sqrt_newton(2.0L, digits + 2);
   long double t = 0.25L;
   long double p = 1.0L;
   long double thresh = threshold_for_digits(digits);

   for (int iter = 0; iter < 10; iter++) {
       long double an = 0.5L * (a + b);
       long double bn = sqrt_newton(a * b, digits + 2);
       long double delta = a - an;
       t -= p * delta * delta;
       p *= 2.0L;
       a = an;
       b = bn;
       // stop when |a-b| < 10^(-digits)
       if ((a > b ? a - b : b - a) < thresh)
           break;
   }
   return (a + b) * (a + b) / (4.0L * t);
}





// Entry point
void _start(void) {
   syscall(SYSCALL_PRINT, "Compute ? via Gauss-Legendre.\n");
   char buf_in[8];
   syscall(SYSCALL_PRINT, "Digits (1-15)? ");
   syscall(SYSCALL_READLINE, buf_in, sizeof(buf_in));
   int digs = atoi(buf_in);
   if (digs<1 || digs>15) {
       syscall(SYSCALL_PRINT, "Error: enter 1-15.\n");
       return;
   }
   long double pi = compute_pi(digs);
   char buf_out[200];
   dtoa(pi, buf_out, digs);
   syscall(SYSCALL_PRINT, buf_out);
   syscall(SYSCALL_PRINT, "\n");
}