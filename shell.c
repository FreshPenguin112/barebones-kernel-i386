#include "shell.h"
#include "string_utils.h"
#include "serial.h"
#include "qemu_utils.h"
#include "tarfs.h"

extern void kernel_print(const char *str);
extern void kernel_putc(char c);
extern void kernel_print_ansi(char *text, char *fg_name, char *bg_name);
extern void kernel_putc_ansi(char text, char *fg_name, char *bg_name);

extern command_t commands[];
extern void qemu_halt_exit(int code);

#define HISTORY_SIZE 100

static char input_buffer[MAX_INPUT_LENGTH];
static char *argv[MAX_ARGS];
static char arg_buffer[MAX_ARGS][MAX_ARG_LENGTH];

// Command history
static char history[HISTORY_SIZE][MAX_INPUT_LENGTH];
static int history_count = 0; // Number of entries in history
static int history_pos = 0;   // Current position for new command
static int history_view = -1; // -1 = not viewing history

// Cursor position in input_buffer
static int cursor_pos = 0;
static int input_len = 0;

// Helper: redraw the current input line
static void redraw_input(void)
{
    // Move cursor to start of line
    kernel_print_ansi("\rice $ ", "cyan", "none");
    // Print input buffer
    for (int i = 0; i < input_len; i++)
        kernel_putc_ansi(input_buffer[i], "magenta", "none");
    // Clear to end of line (optional, for visual clarity)
    kernel_print("\033[K");
    // Move cursor to correct position
    int move_left = input_len - cursor_pos;
    for (int i = 0; i < move_left; i++)
        kernel_print("\033[D");
}

// Helper: set input buffer to a string and redraw
static void set_input(const char *str)
{
    int len = 0;
    while (str[len] && len < MAX_INPUT_LENGTH - 1)
    {
        input_buffer[len] = str[len];
        len++;
    }
    input_buffer[len] = 0;
    input_len = len;
    cursor_pos = len;
    redraw_input();
}

// Parse input into arguments
static int parse_input(char *input)
{
    int argc = 0;
    char *token = strtok(input, ' ');
    while (token && argc < MAX_ARGS)
    {
        strcpy(arg_buffer[argc], token);
        argv[argc] = arg_buffer[argc];
        argc++;
        token = strtok(0, ' ');
    }
    return argc;
}

// Find command in command table
static command_t *find_command(const char *name)
{
    for (command_t *cmd = commands; cmd->name; cmd++)
    {
        if (strcmp(cmd->name, name) == 0)
        {
            return cmd;
        }
    }
    return 0;
}

void shell_init(void)
{
    kernel_print_ansi("Arctic Shell\n", "cyan", "none");
}

void shell_run(void)
{
    while (1)
    {
        kernel_print_ansi("ice $ ", "cyan", "none");
        input_len = 0;
        cursor_pos = 0;
        input_buffer[0] = 0;
        history_view = -1;

        while (1)
        {
            if (serial_has_received())
            {
                char c = serial_read_char();

                // Handle Ctrl+D (ASCII 4)
                if (c == 4)
                {
                    qemu_halt_exit(0);
                }

                // Handle escape sequences (arrow keys)
                if (c == '\033')
                {
                    // Expect '[' next
                    if (serial_has_received() && serial_read_char() == '[')
                    {
                        char dir = 0;
                        if (serial_has_received())
                            dir = serial_read_char();
                        if (dir == 'A')
                        { // Up arrow
                            // Show previous command in history
                            if (history_count > 0)
                            {
                                if (history_view == -1)
                                    history_view = history_count - 1;
                                else if (history_view > 0)
                                    history_view--;
                                set_input(history[history_view]);
                            }
                        }
                        else if (dir == 'B')
                        { // Down arrow
                            // Show next command in history
                            if (history_count > 0 && history_view != -1)
                            {
                                if (history_view < history_count - 1)
                                {
                                    history_view++;
                                    set_input(history[history_view]);
                                }
                                else
                                {
                                    // Blank line after last history
                                    history_view = -1;
                                    set_input("");
                                }
                            }
                        }
                        else if (dir == 'C')
                        { // Right arrow
                            if (cursor_pos < input_len)
                            {
                                kernel_print("\033[C");
                                cursor_pos++;
                            }
                        }
                        else if (dir == 'D')
                        { // Left arrow
                            if (cursor_pos > 0)
                            {
                                kernel_print("\033[D");
                                cursor_pos--;
                            }
                        }
                    }
                    continue;
                }

                // Handle backspace (ASCII 8 or 127)
                if (c == 8 || c == 127)
                {
                    if (cursor_pos > 0 && input_len > 0)
                    {
                        // Shift chars left from cursor
                        for (int j = cursor_pos - 1; j < input_len - 1; j++)
                            input_buffer[j] = input_buffer[j + 1];
                        input_len--;
                        cursor_pos--;
                        input_buffer[input_len] = 0;
                        redraw_input();
                    }
                    continue;
                }

                // Handle enter
                if (c == '\n' || c == '\r')
                {
                    kernel_print("\n");
                    input_buffer[input_len] = 0;
                    break;
                }

                // Printable character
                if (c >= 32 && c <= 126 && input_len < MAX_INPUT_LENGTH - 1)
                {
                    // Insert at cursor position
                    for (int j = input_len; j > cursor_pos; j--)
                        input_buffer[j] = input_buffer[j - 1];
                    input_buffer[cursor_pos] = c;
                    input_len++;
                    cursor_pos++;
                    input_buffer[input_len] = 0;
                    redraw_input();
                }
            }
        }

        // Add to history if not empty and not duplicate of last
        if (input_len > 0 && (history_count == 0 || strcmp(input_buffer, history[(history_count - 1) % HISTORY_SIZE]) != 0))
        {
            strcpy(history[history_count % HISTORY_SIZE], input_buffer);
            history_count++;
            if (history_count > HISTORY_SIZE)
                history_count = HISTORY_SIZE;
        }

        // Parse and execute command
        int argc = parse_input(input_buffer);

        // Handle redirection
        int redirect = 0;
        char *out_file = 0;
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], ">") == 0 && i + 1 < argc)
            {
                redirect = 1;
                out_file = argv[i + 1];
                argv[i] = 0; // Truncate argv for command
                argc = i;
                break;
            }
        }

        if (argc > 0)
        {
            command_t *cmd = find_command(argv[0]);
            if (cmd)
            {
                // If redirect, capture output (simple: only for echo)
                if (redirect && strcmp(argv[0], "echo") == 0)
                {
                    // Concatenate args
                    char buf[256];
                    int pos = 0;
                    for (int i = 1; i < argc && pos < 255; i++)
                    {
                        for (int j = 0; argv[i][j] && pos < 255; j++)
                            buf[pos++] = argv[i][j];
                        if (i < argc - 1 && pos < 255)
                            buf[pos++] = ' ';
                    }
                    buf[pos] = 0;
                    tarfs_write(out_file, buf, pos);
                }
                else
                {
                    cmd->func(argc, argv);
                }
            }
            else
            {
                kernel_print_ansi("Unknown command: ", "red", "none");
                kernel_print_ansi(argv[0], "red", "none");
                kernel_print("\n");
            }
        }
    }
}

// State for cooperative shell
static int shell_initialized = 0;
static int shell_prompted = 0;

void shell_run_step(void)
{
    if (!shell_initialized)
    {
        shell_init();
        shell_initialized = 1;
        shell_prompted = 0;
    }
    if (!shell_prompted)
    {
        kernel_print_ansi("ice $ ", "cyan", "none");
        input_len = 0;
        cursor_pos = 0;
        input_buffer[0] = 0;
        history_view = -1;
        shell_prompted = 1;
    }
    if (serial_has_received())
    {
        char c = serial_read_char();
        // Handle Ctrl+D (ASCII 4)
        if (c == 4)
        {
            qemu_halt_exit(0);
        }
        // Handle escape sequences (arrow keys)
        if (c == '\033')
        {
            if (serial_has_received() && serial_read_char() == '[')
            {
                char dir = 0;
                if (serial_has_received())
                    dir = serial_read_char();
                if (dir == 'A')
                { // Up arrow
                    if (history_count > 0)
                    {
                        if (history_view == -1)
                            history_view = history_count - 1;
                        else if (history_view > 0)
                            history_view--;
                        set_input(history[history_view]);
                    }
                }
                else if (dir == 'B')
                { // Down arrow
                    if (history_count > 0 && history_view != -1)
                    {
                        if (history_view < history_count - 1)
                        {
                            history_view++;
                            set_input(history[history_view]);
                        }
                        else
                        {
                            history_view = -1;
                            set_input("");
                        }
                    }
                }
                else if (dir == 'C')
                { // Right arrow
                    if (cursor_pos < input_len)
                    {
                        kernel_print("\033[C");
                        cursor_pos++;
                    }
                }
                else if (dir == 'D')
                { // Left arrow
                    if (cursor_pos > 0)
                    {
                        kernel_print("\033[D");
                        cursor_pos--;
                    }
                }
            }
            return;
        }
        // Handle backspace (ASCII 8 or 127)
        if (c == 8 || c == 127)
        {
            if (cursor_pos > 0 && input_len > 0)
            {
                for (int j = cursor_pos - 1; j < input_len - 1; j++)
                    input_buffer[j] = input_buffer[j + 1];
                input_len--;
                cursor_pos--;
                input_buffer[input_len] = 0;
                redraw_input();
            }
            return;
        }
        // Handle enter
        if (c == '\n' || c == '\r')
        {
            kernel_print("\n");
            input_buffer[input_len] = 0;
            // Add to history if not empty and not duplicate of last
            if (input_len > 0 && (history_count == 0 || strcmp(input_buffer, history[(history_count - 1) % HISTORY_SIZE]) != 0))
            {
                strcpy(history[history_count % HISTORY_SIZE], input_buffer);
                history_count++;
                if (history_count > HISTORY_SIZE)
                    history_count = HISTORY_SIZE;
            }
            // Parse and execute command
            int argc = parse_input(input_buffer);
            int redirect = 0;
            char *out_file = 0;
            for (int i = 0; i < argc; i++)
            {
                if (strcmp(argv[i], ">") == 0 && i + 1 < argc)
                {
                    redirect = 1;
                    out_file = argv[i + 1];
                    argv[i] = 0;
                    argc = i;
                    break;
                }
            }
            if (argc > 0)
            {
                command_t *cmd = find_command(argv[0]);
                if (cmd)
                {
                    if (redirect && strcmp(argv[0], "echo") == 0)
                    {
                        char buf[256];
                        int pos = 0;
                        for (int i = 1; i < argc && pos < 255; i++)
                        {
                            for (int j = 0; argv[i][j] && pos < 255; j++)
                                buf[pos++] = argv[i][j];
                            if (i < argc - 1 && pos < 255)
                                buf[pos++] = ' ';
                        }
                        buf[pos] = 0;
                        tarfs_write(out_file, buf, pos);
                    }
                    else
                    {
                        cmd->func(argc, argv);
                    }
                }
                else
                {
                    kernel_print_ansi("Unknown command: ", "red", "none");
                    kernel_print_ansi(argv[0], "red", "none");
                    kernel_print("\n");
                }
            }
            shell_prompted = 0; // Prompt for next command
            return;
        }
        // Printable character
        if (c >= 32 && c <= 126 && input_len < MAX_INPUT_LENGTH - 1)
        {
            for (int j = input_len; j > cursor_pos; j--)
                input_buffer[j] = input_buffer[j - 1];
            input_buffer[cursor_pos] = c;
            input_len++;
            cursor_pos++;
            input_buffer[input_len] = 0;
            redraw_input();
        }
    }
}

// Add this function to allow kernel to send input chars to the shell
void shell_handle_input_char(char c)
{
    // Use the same logic as shell_run_step for printable chars
    if (!shell_initialized) {
        shell_init();
        shell_initialized = 1;
        shell_prompted = 0;
    }
    if (!shell_prompted) {
        kernel_print_ansi("ice $ ", "cyan", "none");
        input_len = 0;
        cursor_pos = 0;
        input_buffer[0] = 0;
        history_view = -1;
        shell_prompted = 1;
    }
    // Handle backspace
    if (c == 8 || c == 127) {
        if (cursor_pos > 0 && input_len > 0) {
            for (int j = cursor_pos - 1; j < input_len - 1; j++)
                input_buffer[j] = input_buffer[j + 1];
            input_len--;
            cursor_pos--;
            input_buffer[input_len] = 0;
            redraw_input();
        }
        return;
    }
    // Handle enter
    if (c == '\n' || c == '\r') {
        kernel_print("\n");
        input_buffer[input_len] = 0;
        // Add to history if not empty and not duplicate of last
        if (input_len > 0 && (history_count == 0 || strcmp(input_buffer, history[(history_count - 1) % HISTORY_SIZE]) != 0)) {
            strcpy(history[history_count % HISTORY_SIZE], input_buffer);
            history_count++;
            if (history_count > HISTORY_SIZE)
                history_count = HISTORY_SIZE;
        }
        // Parse and execute command
        int argc = parse_input(input_buffer);
        int redirect = 0;
        char *out_file = 0;
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") == 0 && i + 1 < argc) {
                redirect = 1;
                out_file = argv[i + 1];
                argv[i] = 0;
                argc = i;
                break;
            }
        }
        if (argc > 0) {
            command_t *cmd = find_command(argv[0]);
            if (cmd) {
                if (redirect && strcmp(argv[0], "echo") == 0) {
                    char buf[256];
                    int pos = 0;
                    for (int i = 1; i < argc && pos < 255; i++) {
                        for (int j = 0; argv[i][j] && pos < 255; j++)
                            buf[pos++] = argv[i][j];
                        if (i < argc - 1 && pos < 255)
                            buf[pos++] = ' ';
                    }
                    buf[pos] = 0;
                    tarfs_write(out_file, buf, pos);
                } else {
                    cmd->func(argc, argv);
                }
            } else {
                kernel_print_ansi("Unknown command: ", "red", "none");
                kernel_print_ansi(argv[0], "red", "none");
                kernel_print("\n");
            }
        }
        shell_prompted = 0; // Prompt for next command
        return;
    }
    // Printable character
    if (c >= 32 && c <= 126 && input_len < MAX_INPUT_LENGTH - 1) {
        for (int j = input_len; j > cursor_pos; j--)
            input_buffer[j] = input_buffer[j - 1];
        input_buffer[cursor_pos] = c;
        input_len++;
        cursor_pos++;
        input_buffer[input_len] = 0;
        redraw_input();
    }
}