#include <stddef.h>
#include <stdint.h>

// VGA buffer setup
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 25;

int term_col = 0;
int term_row = 0;
uint8_t term_color = 0x0F; // Default: black background, white foreground

void term_init()
{
    for (int col = 0; col < VGA_COLS; col++)
    {
        for (int row = 0; row < VGA_ROWS; row++)
        {
            const size_t index = (VGA_COLS * row) + col;
            vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
        }
    }
    term_col = 0;
    term_row = 0;
}

void term_setcolor(uint8_t foreground, uint8_t background)
{
    term_color = (background << 4) | (foreground & 0x0F);
}

void term_reset_color()
{
    term_setcolor(0x0F, 0x00); // White on black
}

// Map ANSI color codes to VGA color values (0-7)
uint8_t ansi_to_vga_color(int ansi_code)
{
    switch (ansi_code)
    {
    case 30:
        return 0; // Black
    case 31:
        return 4; // Red
    case 32:
        return 2; // Green
    case 33:
        return 6; // Yellow
    case 34:
        return 1; // Blue
    case 35:
        return 5; // Magenta
    case 36:
        return 3; // Cyan
    case 37:
        return 7; // White
    default:
        return 7; // Default to white
    }
}

void term_handle_ansi_code(int code)
{
    if (code == 0)
    {
        term_reset_color();
    }
    else if (code >= 30 && code <= 37)
    {
        term_setcolor(ansi_to_vga_color(code), term_color >> 4);
    }
    else if (code >= 40 && code <= 47)
    {
        term_setcolor(term_color & 0x0F, ansi_to_vga_color(code - 10));
    }
}

void term_putc(char c)
{
    if (c == '\n')
    {
        term_col = 0;
        term_row++;
    }
    else
    {
        const size_t index = (VGA_COLS * term_row) + term_col;
        vga_buffer[index] = ((uint16_t)term_color << 8) | c;
        term_col++;
    }

    if (term_col >= VGA_COLS)
    {
        term_col = 0;
        term_row++;
    }

    if (term_row >= VGA_ROWS)
    {
        term_col = 0;
        term_row = 0;
    }
}

void term_print(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        term_putc(str[i]);
    }
}

void term_handle_ansi(const char *sequence)
{
    if (sequence[0] == '0' && sequence[1] == 'm')
    {
        // Reset to default: black bg, white fg
        term_setcolor(0x0F, 0x00);
        return;
    }

    if (sequence[0] == '3' && sequence[2] == 'm')
    {
        uint8_t foreground = sequence[1] - '0';
        term_setcolor(foreground, term_color >> 4);
    }
    else if (sequence[0] == '4' && sequence[2] == 'm')
    {
        uint8_t background = sequence[1] - '0';
        term_setcolor(term_color & 0x0F, background);
    }
}

void term_parse_and_print(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\033' && str[i + 1] == '[')
        {
            i += 2; // skip '\033['

            int code = 0;
            while (str[i] >= '0' && str[i] <= '9')
            {
                code = code * 10 + (str[i] - '0');
                i++;
            }

            if (str[i] == 'm')
            {
                term_handle_ansi_code(code);
            }
        }
        else
        {
            term_putc(str[i]);
        }
    }
}

void kernel_main()
{
    term_init();
    term_print("Default colors: Hello, World!\n");
    term_parse_and_print("\033[31mRed foreground\033[0m\n");  // Red text
    term_parse_and_print("\033[44mBlue background\033[0m\n"); // White on blue
    term_parse_and_print("Back to normal.\n");
}
