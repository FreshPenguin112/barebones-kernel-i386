#include "kernel.h"
#include "shell.h"
#include "string_utils.h"
#include "serial.h"
#include "qemu_utils.h"
#include "tarfs.h"
#include "keyboard.h"

extern command_t commands[];
extern void qemu_halt_exit(int code);

#define HISTORY_SIZE 100

static char input_buffer[MAX_INPUT_LENGTH];
static char *argv[MAX_ARGS];
static char arg_buffer[MAX_ARGS][MAX_ARG_LENGTH];

// Command history
static char history[HISTORY_SIZE][MAX_INPUT_LENGTH];
static int history_count = 0;
static int history_pos = 0;
static int history_view = -1;

// Helper: clear input buffer
static void clear_input_buffer(void) {
    for (int i = 0; i < MAX_INPUT_LENGTH; i++) input_buffer[i] = 0;
}

// Parse input into arguments
static int parse_input(char *input) {
    int argc = 0;
    char *token = strtok(input, ' ');
    while (token && argc < MAX_ARGS) {
        strcpy(arg_buffer[argc], token);
        argv[argc] = arg_buffer[argc];
        argc++;
        token = strtok(0, ' ');
    }
    return argc;
}

// Find command in command table
static command_t *find_command(const char *name) {
    for (command_t *cmd = commands; cmd->name; cmd++) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }
    return 0;
}

void shell_init(void) {
    // No initialization needed for serial shell
}

void shell_run(void) {
    while (1) {
        keyboard_clear_buffer();
        clear_input_buffer();
        kernel_print("ice $ ");
        int input_len = 0;
        char c;
        while (1) {
            c = keyboard_read_char();
            if (!c) continue;
            if (c == '\n') {
                serial_write_str("\n");
                input_buffer[input_len] = 0;
                break;
            } else if (c == '\b' || c == 127) {
                if (input_len > 0) {
                    input_len--;
                    input_buffer[input_len] = 0;
                    serial_write_str("\b \b"); // Erase char on serial
                }
            } else if (c >= 32 && c <= 126 && input_len < MAX_INPUT_LENGTH - 1) {
                input_buffer[input_len++] = c;
                input_buffer[input_len] = 0;
                serial_write_char(c);
            }
        }
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
                serial_write_str("Unknown command: ");
                serial_write_str(argv[0]);
                serial_write_str("\n");
            }
        }
    }
}