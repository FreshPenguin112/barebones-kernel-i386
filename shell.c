#include "shell.h"
#include "string_utils.h"
#include "serial.h"
#include "qemu_utils.h"
#include "tarfs.h"
#include "kernel.h"

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
static char *argv2[MAX_ARGS];
static char arg_buffer2[MAX_ARGS][MAX_ARG_LENGTH];

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

        int command_run = 0;
        while (!command_run)
        {
            if (serial_has_received())
            {
                char c = serial_read_char();
                kernel_print_ansi("[DEBUG] keypress received in shell main loop\n", "magenta", "none");

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
                    continue;
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
                    continue;
                }

                // Handle enter
                if (c == '\n' || c == '\r')
                {
                    kernel_print("\n");
                    input_buffer[input_len] = 0;
                    for (int dbg_i = 0; dbg_i < input_len; dbg_i++) {
                        kernel_putc(input_buffer[dbg_i]);
                    }
                    char dbg_len_str[8];
                    itoa(input_len, dbg_len_str, 10);
                    break;
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

        // Add to history if not empty and not duplicate of last
        if (input_len > 0 && (history_count == 0 || strcmp(input_buffer, history[(history_count - 1) % HISTORY_SIZE]) != 0))
        {
            strcpy(history[history_count % HISTORY_SIZE], input_buffer);
            history_count++;
            if (history_count > HISTORY_SIZE)
                history_count = HISTORY_SIZE;
        }

        char debug_input[MAX_INPUT_LENGTH];
        for (int i = 0; i < input_len && i < MAX_INPUT_LENGTH - 1; i++)
            debug_input[i] = input_buffer[i];
        debug_input[input_len < MAX_INPUT_LENGTH - 1 ? input_len : MAX_INPUT_LENGTH - 1] = 0;
        kernel_print(debug_input);
        kernel_print("\n");
        input_buffer[input_len] = 0;

        // --- PIPE HANDLING ---
        char *pipe_pos = 0;
        for (int i = 0; i < input_len; i++)
        {
            if (input_buffer[i] == '|')
            {
                pipe_pos = &input_buffer[i];
                break;
            }
        }
        if (pipe_pos)
        {
            *pipe_pos = 0;
            char *first_cmd = input_buffer;
            char *second_cmd = pipe_pos + 1;
            while (*second_cmd == ' ')
                second_cmd++;
            int first_len = strlen(first_cmd);
            while (first_len > 0 && first_cmd[first_len - 1] == ' ')
            {
                first_cmd[first_len - 1] = 0;
                first_len--;
            }
            int argc1 = parse_input(first_cmd);
            int argc2 = parse_input(second_cmd);
            char pipe_buf[4096];
            pipe_buf[0] = 0;
            // Run first command with output capture
            command_t *cmd1 = find_command(argv[0]);
            if (!cmd1) {
                kernel_print_ansi("Unknown command: ", "red", "none");
                kernel_print_ansi(argv[0], "red", "none");
                kernel_print("\n");
                return;
            }
            kernel_output_capture_start(pipe_buf, sizeof(pipe_buf));
            cmd1->func(argc1, argv);
            kernel_output_capture_stop();

            // Prepare argv2 for second command
            char *token2 = strtok(second_cmd, ' ');
            int argc2_pipe = 0;
            while (token2 && argc2_pipe < MAX_ARGS - 2) {
                strcpy(arg_buffer2[argc2_pipe], token2);
                argv2[argc2_pipe] = arg_buffer2[argc2_pipe];
                argc2_pipe++;
                token2 = strtok(0, ' ');
            }
            static char pipe_arg_static[4096];
            pipe_arg_static[0] = '\x01';
            strncpy(pipe_arg_static + 1, pipe_buf, 4094);
            pipe_arg_static[4095] = 0;
            argv2[argc2_pipe] = pipe_arg_static;
            argc2_pipe++;

            command_t *cmd2 = find_command(argv2[0]);
            if (cmd2) {
                cmd2->func(argc2_pipe, argv2);
                // If interactive, enter input loop
                if (strcmp(argv2[0], "less") == 0) {
                    extern volatile int less_running;
                    less_running = 1;
                    while (less_running) {
                        if (serial_has_received()) {
                            char c = serial_read_char();
                            extern void less_handle_input(char c);
                            less_handle_input(c);
                        }
                    }
                }
            } else {
                kernel_print_ansi("Unknown command: ", "red", "none");
                kernel_print_ansi(argv2[0], "red", "none");
                kernel_print("\n");
            }
            return;
        }
    }
}

// State for cooperative shell
static int shell_initialized = 0;
static int shell_prompted = 0;
static int shell_pipe_mode = 0;

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
    if (shell_pipe_mode) return; // Disable user input in pipe mode
    if (serial_has_received())
    {
        char c = serial_read_char();
        // Handle Ctrl+D (ASCII 4)
        if (c == 4) { qemu_halt_exit(0); }
        // Handle escape sequences (arrow keys)
        if (c == '\033') {
            if (serial_has_received() && serial_read_char() == '[') {
                char dir = 0;
                if (serial_has_received()) dir = serial_read_char();
                if (dir == 'A') { if (history_count > 0) { if (history_view == -1) history_view = history_count - 1; else if (history_view > 0) history_view--; set_input(history[history_view]); } }
                else if (dir == 'B') { if (history_count > 0 && history_view != -1) { if (history_view < history_count - 1) { history_view++; set_input(history[history_view]); } else { history_view = -1; set_input(""); } } }
                else if (dir == 'C') { if (cursor_pos < input_len) { kernel_print("\033[C"); cursor_pos++; } }
                else if (dir == 'D') { if (cursor_pos > 0) { kernel_print("\033[D"); cursor_pos--; } }
            }
            return;
        }
        // Handle backspace (ASCII 8 or 127)
        if (c == 8 || c == 127) {
            if (cursor_pos > 0 && input_len > 0) { for (int j = cursor_pos - 1; j < input_len - 1; j++) input_buffer[j] = input_buffer[j + 1]; input_len--; cursor_pos--; input_buffer[input_len] = 0; redraw_input(); } return;
        }
        // Handle enter
        if (c == '\n' || c == '\r') {
            kernel_print("\n");
            input_buffer[input_len] = 0;
            if (input_len > 0 && (history_count == 0 || strcmp(input_buffer, history[(history_count - 1) % HISTORY_SIZE]) != 0)) { strcpy(history[history_count % HISTORY_SIZE], input_buffer); history_count++; if (history_count > HISTORY_SIZE) history_count = HISTORY_SIZE; }
            input_buffer[input_len] = 0;
            // --- PIPE HANDLING ---
            char *pipe_pos = 0;
            for (int i = 0; i < input_len; i++) {
                if (input_buffer[i] == '|' && (i == 0 || input_buffer[i-1] != '\\')) {
                    pipe_pos = &input_buffer[i];
                    break;
                }
            }
            if (pipe_pos) {
                *pipe_pos = 0;
                char *first_cmd = input_buffer;
                char *second_cmd = pipe_pos + 1;
                while (*second_cmd == ' ') second_cmd++;
                int first_len = strlen(first_cmd);
                while (first_len > 0 && first_cmd[first_len - 1] == ' ') { first_cmd[first_len - 1] = 0; first_len--; }
                int argc1 = parse_input(first_cmd);
                int argc2 = parse_input(second_cmd);
                char pipe_buf[4096];
                pipe_buf[0] = 0;
                command_t *cmd1 = find_command(argv[0]);
                if (!cmd1) { kernel_print_ansi("Unknown command: ", "red", "none"); kernel_print_ansi(argv[0], "red", "none"); kernel_print("\n"); shell_prompted = 0; return; }
                shell_pipe_mode = 1;
                kernel_print_ansi("[DEBUG] output capture START\n", "magenta", "none");
                kernel_output_capture_start(pipe_buf, sizeof(pipe_buf));
                cmd1->func(argc1, argv);
                kernel_print_ansi("[DEBUG] output capture STOP\n", "magenta", "none");
                kernel_output_capture_stop();
                shell_pipe_mode = 0; // Ensure input is not blocked before second command
                int argc2_pipe = 0;
                char *token2 = strtok(second_cmd, ' ');
                if (token2) { strcpy(arg_buffer2[0], token2); argv2[0] = arg_buffer2[0]; argc2_pipe = 1; token2 = strtok(0, ' '); while (token2 && argc2_pipe < MAX_ARGS - 2) { strcpy(arg_buffer2[argc2_pipe], token2); argv2[argc2_pipe] = arg_buffer2[argc2_pipe]; argc2_pipe++; token2 = strtok(0, ' '); } }
                static char pipe_arg_static[4096];
                pipe_arg_static[0] = '\x01';
                strncpy(pipe_arg_static + 1, pipe_buf, 4094);
                pipe_arg_static[4095] = 0;
                argv2[argc2_pipe] = pipe_arg_static;
                argc2_pipe++;
                kernel_print_ansi("[DEBUG] about to run second command (no output capture)\n", "magenta", "none");
                command_t *cmd2 = find_command(argv2[0]);
                if (cmd2) { cmd2->func(argc2_pipe, argv2); } else { kernel_print_ansi("Unknown command: ", "red", "none"); kernel_print_ansi(argv2[0], "red", "none"); kernel_print("\n"); }
                shell_prompted = 0;
                input_buffer[0] = 0;
                history_view = -1;
                // Exit the shell_run main loop immediately after piping
                return;
            }

            // --- REDIRECTION HANDLING ---
            int argc = parse_input(input_buffer);
            int redirect = 0;
            char *out_file = 0;
            for (int i = 0; i < argc; i++) {
                if (strcmp(argv[i], ">") == 0 && i + 1 < argc) {
                    redirect = 1;
                    out_file = argv[i + 1];
                    argv[i] = 0; // Truncate argv for command
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
            shell_prompted = 0;
            return;
        }
        // Printable character
        if (c >= 32 && c <= 126 && input_len < MAX_INPUT_LENGTH - 1) { for (int j = input_len; j > cursor_pos; j--) input_buffer[j] = input_buffer[j - 1]; input_buffer[cursor_pos] = c; input_len++; cursor_pos++; input_buffer[input_len] = 0; redraw_input(); }
    }
}

// Add this function to allow kernel to send input chars to the shell
void shell_handle_input_char(char c)
{
    // Use the same logic as shell_run_step for printable chars
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
    // Handle backspace
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