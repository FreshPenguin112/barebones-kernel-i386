#include "shell.h"
#include "string_utils.h"
#include "tarfs.h"
#include "elf.h"
#include "qemu_utils.h"

extern void kernel_print(const char *str);
extern void kernel_print_ansi(const char *text, char *fg_name, char *bg_name);
extern void kernel_putc_ansi(char text, char *fg_name, char *bg_name);
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
        kernel_print_ansi("Usage: bc <expr>\n", "red", "none");
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
    kernel_print_ansi(buf, "green", "none");
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
            kernel_print_ansi(s, "green", "none");
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
        kernel_print_ansi(display, "green", "none");
        kernel_print("\n");
    }
}

// Print 'size' bytes from 'data' in classic hexdump style.
static void hex_dump(const char *data, unsigned int size)
{
    const unsigned char *bytes = (const unsigned char*)data;
    char buf[16+1];

    for (unsigned int off = 0; off < size; off += 16) {
        // 1) print offset
        // e.g. "00000000: "
        {
            char off_str[9];
            // simple hex convert for offset (8 hex digits)
            for (int i = 0; i < 8; i++) {
                unsigned int shift = (7 - i) * 4;
                unsigned int digit = (off >> shift) & 0xF;
                off_str[i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
            }
            off_str[8] = '\0';
            kernel_print_ansi(off_str, "green", "none");
            kernel_print(": ");
        }

        // 2) print hex bytes
        for (int i = 0; i < 16; i++) {
            if (off + i < size) {
                unsigned char b = bytes[off + i];
                // two hex digits
                char h[3];
                h[0] = (b >> 4) < 10 ? '0' + (b >> 4) : 'A' + (b >> 4) - 10;
                h[1] = (b & 0xF) < 10 ? '0' + (b & 0xF) : 'A' + (b & 0xF) - 10;
                h[2] = '\0';
                kernel_print_ansi(h, "green", "none");
            } else {
                // pad beyond end
                kernel_print("  ");
            }
            // space after every byte, extra space at 8-byte mark
            kernel_print(i == 7 ? "  " : " ");
        }

        // 3) ASCII side
        kernel_print(" ");
        for (int i = 0; i < 16 && off + i < size; i++) {
            unsigned char c = bytes[off + i];
            buf[i] = (c >= 32 && c < 127) ? c : '.';
        }
        buf[(off + 16 <= size) ? 16 : (size - off)] = '\0';
        kernel_print_ansi(buf, "green", "none");

        kernel_print("\n");
    }
}

void cmd_hexdump(int argc, char **argv)
{
    if (argc < 2) {
        kernel_print_ansi("Usage: hexdump <file>\n", "red", "none");
        return;
    }

    unsigned int size;
    const char *data = tarfs_cat(argv[1], &size);
    if (!data) {
        kernel_print_ansi("No such file\n", "red", "none");
        return;
    }

    hex_dump(data, size);
}


void cmd_cat(int argc, char **argv)
{
    if (argc < 2)
    {
        kernel_print_ansi("Usage: cat <file>\n", "red", "none");
        return;
    }
    unsigned int sz;
    const char *data = tarfs_cat(argv[1], &sz);
    if (!data)
    {
        kernel_print_ansi("No such file\n", "red", "none");
        return;
    }
    for (unsigned int i = 0; i < sz; i++)
        kernel_putc_ansi(data[i], "green", "none");
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
    kernel_print("  hexdump <file> - Dump file in hex + ASCII\n");
    kernel_print("  exit       - Exit the shell\n");
    kernel_print("  rev <text> - Reverse the given text\n");
}

void cmd_exit(int argc, char **argv)
{
    kernel_print_ansi("Exiting shell...\n", "red", "none");
    qemu_halt_exit(0);
}    

void cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        kernel_print_ansi("Usage: run <file.elf>\n", "red", "none");
        return;
    }
    int res = elf_run(argv[1]);
    if (res == -1)
        kernel_print_ansi("File not found\n", "red", "none");
    else if (res == -2)
        kernel_print_ansi("Not an ELF file\n", "red", "none");
    else if (res == -3)
        kernel_print_ansi("Unsupported ELF\n", "red", "none");
    // else: program ran and returned
}

static void rev(char *s) {
    char t[128];
    int len = strlen(s);
    int i = 0;

    // Copy characters from s into t in reverse order
    while (len > 0) {
        t[i++] = s[--len];
    }
    t[i] = '\0';

    // Copy reversed string back into s
    strcpy(s, t);
}

void cmd_rev(int argc, char **argv) {
    if (argc < 2) {
        kernel_print_ansi("Usage: rev <text>\n", "red", "none");
        return;
    }

    // Reconstruct the input text from argv[1..]
    char buf[128];
    int pos = 0;
    for (int i = 1; i < argc && pos < (int)sizeof(buf) - 1; i++) {
        for (int j = 0; argv[i][j] && pos < (int)sizeof(buf) - 1; j++) {
            buf[pos++] = argv[i][j];
        }
        if (i < argc - 1 && pos < (int)sizeof(buf) - 1) {
            buf[pos++] = ' ';
        }
    }
    buf[pos] = '\0';

    // Reverse it in-place
    rev(buf);

    // Print the result
    kernel_print_ansi(buf, "green", "none");
    kernel_print("\n");
}


// Command table
command_t commands[] = {
    {"echo",     cmd_echo,      "Print text to screen"},
    {"help",     cmd_help,      "Show this help message"},
    {"bc",       cmd_bc,        "Calculate math expression"},
    {"clear",    cmd_clear,     "Clear the screen"},
    {"ls",       cmd_ls,        "List files"},
    {"cat",      cmd_cat,       "Display file contents"},
    {"hexdump",  cmd_hexdump,   "Dump file in hex + ASCII"},
    {"run",      cmd_run,       "Run ELF executable"},
    {"rev",      cmd_rev,       "Reverse the given text"},
    {"exit",     cmd_exit,      "Exit the shell"},
        {"less",     cmd_less,      "View text with scrolling"},
    {0, 0, 0}
};
// Basic less command: supports up/down arrow and q to quit
void cmd_less(int argc, char **argv) {
    const char *data = NULL;
    unsigned int size = 0;
    int from_pipe = 0;
    if (argc >= 2) {
        // If piped, argv[1] is the buffer
        if (argc == 2 && argv[1][0] == '\x01') {
            data = argv[1] + 1;
            size = strlen(data);
            from_pipe = 1;
        } else {
            data = tarfs_cat(argv[1], &size);
            if (!data) {
                kernel_print_ansi("No such file\n", "red", "none");
                return;
            }
        }
    } else {
        kernel_print_ansi("Usage: less <file> or use with pipe\n", "red", "none");
        return;
    }

    int lines = 0, max_lines = 0;
    int line_starts[512];
    line_starts[0] = 0;
    for (unsigned int i = 0; i < size; i++) {
        if (data[i] == '\n') {
            lines++;
            line_starts[lines] = i + 1;
        }
    }
    max_lines = lines + 1;
    int view_line = 0;
    int running = 1;
    while (running) {
        ansi_clearhome();
        for (int l = view_line; l < view_line + 20 && l < max_lines; l++) {
            unsigned int start = line_starts[l];
            unsigned int end = (l + 1 < max_lines) ? line_starts[l + 1] : size;
            for (unsigned int j = start; j < end; j++) {
                kernel_putc_ansi(data[j], "green", "none");
            }
        }
        kernel_print_ansi("\n-- less: Up/Down to scroll, q to quit --\n", "yellow", "none");
        while (1) {
            if (serial_has_received()) {
                char c = serial_read_char();
                if (c == 'q') {
                    running = 0;
                    break;
                }
                if (c == '\033') {
                    if (serial_has_received() && serial_read_char() == '[') {
                        char dir = 0;
                        if (serial_has_received()) dir = serial_read_char();
                        if (dir == 'A' && view_line > 0) { // Up
                            view_line--;
                            break;
                        }
                        if (dir == 'B' && view_line < max_lines - 20) { // Down
                            view_line++;
                            break;
                        }
                    }
                }
            }
        }
    }
    ansi_clearhome();
}
