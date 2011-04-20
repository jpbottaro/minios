#include <minios/dev.h>
#include <minios/misc.h>
#include <minios/i386.h>
#include <minios/sched.h>
#include "keyboardscreen.h"

/* array taken from http://www.osdever.net/bkerndev/Docs/keyboard.htm */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

#define MAX_KEYS 0x100
#define WHITE    0x0F

struct video_char_s {
    char letter;
    char color;
} __attribute__((__packed__));

char kbd_buffer[MAX_KEYS];
unsigned int pos = 0, end = 0;

struct video_char_s (*vram)[25][80] = (struct video_char_s (*)[25][80]) 0xB8000;
unsigned int x = 0, y = 0, xlimit = 0;

/* move the video ram 1 row up */
void scroll_up_vram()
{
    int i, j;

    for (i = 0; i < 24; i++) {
        for (j = 0; j < 80; j++) {
            (*vram)[i][j].letter = (*vram)[i+1][j].letter;
            (*vram)[i][j].color = (*vram)[i+1][j].color;
        }
    }

    for (j = 0; j < 80; j++) {
        (*vram)[24][j].letter = 0;
        (*vram)[24][j].color = WHITE;
    }
}

/* move cursor to (x,y), we asume the ports are 0x3D4-0x3D5 */
void move_cursor(int x, int y)
{
    unsigned short pos;

    pos = (y * 80) + x;
    /* low register */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    /* high register */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

/* clear screen... */
void clear_screen()
{
    int i, j;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 80; j++) {
            (*vram)[i][j].letter = 0;
            (*vram)[i][j].color = WHITE;
        }
    }

    x = xlimit = y = 0;
    move_cursor(0, 0);
}

/* print 1 key in the screen */
void print_key(char key)
{
    switch (key) {
        case '\n':
            x = xlimit = 0;
            if (y == 24)
                scroll_up_vram();
            else
                y++;
            break;
        case '\b':
            if (x > xlimit) {
                x--;
                (*vram)[y][x].letter = 0;
            }
            break;
        case '\t':
            key = ' ';
        default:
            (*vram)[y][x].letter = key;
            (*vram)[y][x].color = WHITE;
            x++;
            if (x == 80) {
                x = xlimit = 0;
                if (y == 24)
                    scroll_up_vram();
                else
                    y++;
            }
    }
    move_cursor(x, y);
}

/* save key in a buffer */
/* XXX what out for strange \b behaviour... */
void buffer_key(char key)
{
    if (key == '\b') {
        end = (end == pos) ? end : (end == 0) ? MAX_KEYS - 1 : end - 1;
    } else {
        kbd_buffer[end++] = key;
        end %= MAX_KEYS;
        if (key == '\n') {
            kbd_buffer[end++] = '\0';
            end %= MAX_KEYS;
        }
    }
}

/* print a null terminated string in the screen */
int print(const char *str, unsigned int n)
{
    const char *p;
    unsigned int i;

    p = str;
    for (i = 0; i < n && p[i] != '\0'; i++)
        print_key(p[i]);
    xlimit = x;
    return i;
}

/* get a line from the screen */
int get_line(char *line, unsigned int n)
{
    unsigned int i, j, max;

    i = 0;
    j = pos;
    max = MIN(MAX_LINE - 1, n);
    while (i < max && j != end && kbd_buffer[j] != '\0') {
        line[i++] = kbd_buffer[j++];
        j %= MAX_KEYS;
    }

    line[i] = '\0';

    pos = (j != end && kbd_buffer[j] == '\0') ? j + 1: j;
    pos %= MAX_KEYS;

    return i + 1;
}

/* manage keyboard interruptions */
void keyboard(unsigned char scancode)
{
    char key;

    if (scancode > 128)
        return;

    key = kbdus[scancode];

    if (key) {
        print_key(key);
        buffer_key(key);
        if (key == '\n')
            unblock_process(DEV_STDIN);
    }
}
