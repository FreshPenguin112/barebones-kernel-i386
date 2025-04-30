#include "shell.h"
#include "string_utils.h"
#include "tarfs.h"
#include "elf.h"

extern void kernel_print(const char *str);
extern void kernel_putc(char c);
extern void ansi_clearhome(void);

// Forward declarations
static float bc_expr(const char **s);
static float bc_term(const char **s);
static float bc_factor(const char **s);

// Helper: skip whitespace
static void bc_skipws(const char **s)
{
    while (**s == ' ' || **s == '\t')
        (*s)++;
}

// Parse a floating-point number (e.g., "12.34")
static float bc_number(const char **s)
{
    bc_skipws(s);
    int sign = 1;
    float val = 0.0f;

    if (**s == '-')
    {
        sign = -1;
        (*s)++;
    }

    // Parse integer part
    while (**s >= '0' && **s <= '9')
    {
        val = val * 10 + (**s - '0');
        (*s)++;
    }

    // Parse fractional part
    if (**s == '.')
    {
        (*s)++;
        float frac = 0.1f;
        while (**s >= '0' && **s <= '9')
        {
            val += frac * (**s - '0');
            frac *= 0.1f;
            (*s)++;
        }
    }

    return sign * val;
}

// Parse factor: number or parenthesis
static float bc_factor(const char **s)
{
    bc_skipws(s);
    float val;

    if (**s == '(')
    {
        (*s)++;
        val = bc_expr(s);
        bc_skipws(s);
        if (**s == ')')
            (*s)++;
    }
    else
    {
        val = bc_number(s);
    }

    return val;
}

// Parse term: multiplication/division
static float bc_term(const char **s)
{
    float val = bc_factor(s);
    bc_skipws(s);

    while (**s == '*' || **s == '/')
    {
        char op = **s;
        (*s)++;
        float rhs = bc_factor(s);

        if (op == '*')
            val *= rhs;
        else if (op == '/')
            val /= rhs;

        bc_skipws(s);
    }

    return val;
}

// Parse expression: addition/subtraction
static float bc_expr(const char **s)
{
    float val = bc_term(s);
    bc_skipws(s);

    while (**s == '+' || **s == '-')
    {
        char op = **s;
        (*s)++;
        float rhs = bc_term(s);

        if (op == '+')
            val += rhs;
        else if (op == '-')
            val -= rhs;

        bc_skipws(s);
    }

    return val;
}

// bc command: bc <expr>
void cmd_bc(int argc, char **argv)
{
    if (argc < 2)
    {
        kernel_print("Usage: bc <expr>\n");
        return;
    }

    // Reconstruct the expression from argv[1..]
    char expr[128];
    int pos = 0;
    for (int i = 1; i < argc && pos < 127; i++)
    {
        for (int j = 0; argv[i][j] && pos < 127; j++)
            expr[pos++] = argv[i][j];
        if (i < argc - 1 && pos < 127)
            expr[pos++] = ' ';
    }
    expr[pos] = 0;

    const char *p = expr;
    float result = bc_expr(&p);

    char buf[32];
    ftoa(result, buf, 6); // Convert result to string with 6 decimal places
    kernel_print(buf);
    kernel_print("\n");
}

void cmd_echo(int argc, char **argv)
{
    int interpret_escapes = 0;
    int start = 1;

    // Check for -e flag
    if (argc > 1 && strcmp(argv[1], "-e") == 0)
    {
        interpret_escapes = 1;
        start = 2;
    }

    for (int i = start; i < argc; i++)
    {
        const char *s = argv[i];
        if (interpret_escapes)
        {
            // Print with escape interpretation
            for (int j = 0; s[j]; j++)
            {
                if (s[j] == '\\' && s[j + 1])
                {
                    if (s[j + 1] == 'e')
                    {
                        kernel_putc('\033');
                        j++;
                    }
                    else if (s[j + 1] == 'n')
                    {
                        kernel_putc('\n');
                        j++;
                    }
                    else if (s[j + 1] == 't')
                    {
                        kernel_putc('\t');
                        j++;
                    }
                    else if (s[j + 1] == '\\')
                    {
                        kernel_putc('\\');
                        j++;
                    }
                    else
                    {
                        kernel_putc(s[j]);
                    }
                }
                else
                {
                    kernel_putc(s[j]);
                }
            }
        }
        else
        {
            kernel_print(s);
        }
        if (i < argc - 1)
        {
            kernel_print(" ");
        }
    }
    kernel_print("\n");
}

void cmd_clear(int argc, char **argv)
{
    ansi_clearhome();
}

void cmd_ls(int argc, char **argv)
{
    const char **names;
    int n = tarfs_ls(&names);
    for (int i = 0; i < n; i++)
    {
        // Skip "./"
        if (strcmp(names[i], "./") == 0)
            continue;
        // Strip leading "./" if present
        const char *display = names[i];
        if (display[0] == '.' && display[1] == '/')
            display += 2;
        kernel_print(display);
        kernel_print("\n");
    }
}

void cmd_cat(int argc, char **argv)
{
    if (argc < 2)
    {
        kernel_print("Usage: cat <file>\n");
        return;
    }
    unsigned int sz;
    const char *data = tarfs_cat(argv[1], &sz);
    if (!data)
    {
        kernel_print("No such file\n");
        return;
    }
    for (unsigned int i = 0; i < sz; i++)
        kernel_putc(data[i]);
    kernel_print("\n");
}

void cmd_help(int argc, char **argv)
{
    kernel_print("Available commands:\n");
    kernel_print("  echo [text] - Print text to screen\n");
    kernel_print("  help       - Show this help message\n");
    kernel_print("  bc <expr>  - Calculate math expression\n");
    kernel_print("  clear      - Clear the screen\n");
    kernel_print("  ls         - List files\n");
    kernel_print("  cat <file> - Display file contents\n");
    kernel_print("  run <file.elf> - Run ELF executable\n");
}

void cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        kernel_print("Usage: run <file.elf>\n");
        return;
    }
    int res = elf_run(argv[1]);
    if (res == -1)
        kernel_print("File not found\n");
    else if (res == -2)
        kernel_print("Not an ELF file\n");
    else if (res == -3)
        kernel_print("Unsupported ELF\n");
    // else: program ran and returned
}

// Command table
command_t commands[] = {
    {"echo", cmd_echo, "Print text to screen"},
    {"help", cmd_help, "Show help message"},
    {"bc", cmd_bc, "Calculate math expression"},
    {"clear", cmd_clear, "Clear the screen"},
    {"ls", cmd_ls, "List files"},
    {"cat", cmd_cat, "Display file contents"},
    {"run", cmd_run, "Run ELF executable"},
    {0, 0, 0} // Null terminator
};