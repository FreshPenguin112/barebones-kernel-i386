#include "shell.h"
#include "string_utils.h"

extern void kernel_print(const char* str);
extern void kernel_putc(char c);
extern void ansi_clearhome(void); // Add this line

#define FP_SCALE 100  // 2 decimal places

// Forward declarations
static int bc_expr(const char **s);
static int bc_term(const char **s);
static int bc_factor(const char **s);
static int bc_number(const char **s);
static int bc_pow(const char **s);

// Helper: skip whitespace
static void bc_skipws(const char **s) {
    while (**s == ' ' || **s == '\t') (*s)++;
}

// Parse a fixed-point number (e.g., "12.34" -> 1234)
static int bc_number(const char **s) {
    bc_skipws(s);
    int sign = 1, val = 0, frac = 0, frac_div = FP_SCALE;
    if (**s == '-') { sign = -1; (*s)++; }
    while (**s >= '0' && **s <= '9') {
        val = val * 10 + (**s - '0');
        (*s)++;
    }
    val *= FP_SCALE;
    if (**s == '.') {
        (*s)++;
        int digits = 0;
        while (**s >= '0' && **s <= '9' && digits < 2) { // 2 decimal places
            frac = frac * 10 + (**s - '0');
            (*s)++;
            digits++;
        }
        if (digits == 1) frac *= 10; // e.g., "1.2" -> "1.20"
        val += frac;
    }
    return sign * val;
}

// Parse factor: number or parenthesis or exponentiation
static int bc_factor(const char **s) {
    bc_skipws(s);
    int val;
    if (**s == '(') {
        (*s)++;
        val = bc_expr(s);
        bc_skipws(s);
        if (**s == ')') (*s)++;
        return val;
    } else {
        return bc_number(s);
    }
}

// Parse exponentiation (right-associative)
static int bc_pow(const char **s) {
    int base = bc_factor(s);
    bc_skipws(s);
    while (**s == '^') {
        (*s)++;
        int exp = bc_pow(s); // right-associative
        // Integer exponentiation for fixed-point
        int result = FP_SCALE;
        for (int i = 0; i < exp / FP_SCALE; i++) result = (result * base) / FP_SCALE;
        base = result;
        bc_skipws(s);
    }
    return base;
}

// Parse term: multiplication/division
static int bc_term(const char **s) {
    int val = bc_pow(s);
    bc_skipws(s);
    while (**s == '*' || **s == '/') {
        char op = **s;
        (*s)++;
        int rhs = bc_pow(s);
        if (op == '*') val = (val * rhs) / FP_SCALE;
        else if (op == '/') val = (val * FP_SCALE) / rhs;
        bc_skipws(s);
    }
    return val;
}

// Parse expression: addition/subtraction
static int bc_expr(const char **s) {
    int val = bc_term(s);
    bc_skipws(s);
    while (**s == '+' || **s == '-') {
        char op = **s;
        (*s)++;
        int rhs = bc_term(s);
        if (op == '+') val += rhs;
        else if (op == '-') val -= rhs;
        bc_skipws(s);
    }
    return val;
}

// Fixed-point to string (e.g., 1234 -> "12.34", 1200 -> "12", 1250 -> "12.5")
static void bc_ftoa(int n, char *buf) {
    int neg = 0;
    if (n < 0) { neg = 1; n = -n; }
    int intpart = n / FP_SCALE;
    int fracpart = n % FP_SCALE;
    // Integer part
    int i = 0;
    if (neg) buf[i++] = '-';
    // Convert intpart to string
    char tmp[12];
    int j = 0;
    if (intpart == 0) tmp[j++] = '0';
    else {
        int x = intpart;
        while (x > 0) {
            tmp[j++] = '0' + (x % 10);
            x /= 10;
        }
    }
    // Reverse intpart
    for (int k = j - 1; k >= 0; k--) buf[i++] = tmp[k];

    if (fracpart == 0) {
        buf[i] = 0;
        return;
    }

    buf[i++] = '.';
    // Always 2 digits for fracpart, but trim trailing zeros
    int d1 = fracpart / 10;
    int d2 = fracpart % 10;
    if (d2 == 0) {
        buf[i++] = '0' + d1;
    } else {
        buf[i++] = '0' + d1;
        buf[i++] = '0' + d2;
    }
    buf[i] = 0;

    // Remove trailing zeros after decimal point
    if (buf[i-1] == '0' && buf[i-2] != '.') {
        buf[i-1] = 0;
    }
    if (buf[i-1] == '.' ) {
        buf[i-1] = 0;
    }
}

// bc command: bc <expr>
void cmd_bc(int argc, char **argv) {
    if (argc < 2) {
        kernel_print("Usage: bc <expr>\n");
        return;
    }
    // Reconstruct the expression from argv[1..]
    char expr[128];
    int pos = 0;
    for (int i = 1; i < argc && pos < 127; i++) {
        for (int j = 0; argv[i][j] && pos < 127; j++)
            expr[pos++] = argv[i][j];
        if (i < argc - 1 && pos < 127)
            expr[pos++] = ' ';
    }
    expr[pos] = 0;

    const char *p = expr;
    int result = bc_expr(&p);

    char buf[32];
    bc_ftoa(result, buf);
    kernel_print(buf);
    kernel_print("\n");
}

void cmd_echo(int argc, char** argv) {
    int interpret_escapes = 0;
    int start = 1;

    // Check for -e flag
    if (argc > 1 && strcmp(argv[1], "-e") == 0) {
        interpret_escapes = 1;
        start = 2;
    }

    for (int i = start; i < argc; i++) {
        const char* s = argv[i];
        if (interpret_escapes) {
            // Print with escape interpretation
            for (int j = 0; s[j]; j++) {
                if (s[j] == '\\' && s[j+1]) {
                    if (s[j+1] == 'e') {
                        kernel_putc('\033');
                        j++;
                    } else if (s[j+1] == 'n') {
                        kernel_putc('\n');
                        j++;
                    } else if (s[j+1] == 't') {
                        kernel_putc('\t');
                        j++;
                    } else if (s[j+1] == '\\') {
                        kernel_putc('\\');
                        j++;
                    } else {
                        kernel_putc(s[j]);
                    }
                } else {
                    kernel_putc(s[j]);
                }
            }
        } else {
            kernel_print(s);
        }
        if (i < argc - 1) {
            kernel_print(" ");
        }
    }
    kernel_print("\n");
}

void cmd_clear(int argc, char **argv) {
    ansi_clearhome();
}

void cmd_help(int argc, char** argv) {
    kernel_print("Available commands:\n");
    kernel_print("  echo [text] - Print text to screen\n");
    kernel_print("  help       - Show this help message\n");
    kernel_print("  bc <expr>  - Calculate math expression\n");
    kernel_print("  clear      - Clear the screen\n"); // Add this line
}

// Command table
command_t commands[] = {
    {"echo", cmd_echo, "Print text to screen"},
    {"help", cmd_help, "Show help message"},
    {"bc",   cmd_bc,   "Calculate math expression"},
    {"clear", cmd_clear, "Clear the screen"}, // Add this line
    {0, 0, 0} // Null terminator
};