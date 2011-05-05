#include <minios/dev.h>
#include <minios/misc.h>
#include <minios/sched.h>
#include "kbd.h"
#include "vga.h"

waiting_list_t kbd_list;

/* array taken from http://www.osdever.net/bkerndev/Docs/keyboard.htm */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9                     */
  '9', '0', '-', '=','\b',                          /* Backspace             */
 '\t',                                              /* Tab                   */
  'q', 'w', 'e', 'r',                               /* 19                    */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']','\n',      /* Enter key             */
    0,                                              /* 29 - Control          */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39                    */
 '\'', '`',   0,                                    /* Left shift            */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                /* 49                    */
  'm', ',', '.', '/',   0,                          /* Right shift           */
  '*',
    0,                                              /* Alt                   */
  ' ',                                              /* Space bar             */
    0,                                              /* Caps lock             */
    0,                                              /* 59 - F1 key ... >     */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,                                              /* < ... F10             */
    0,                                              /* 69 - Num lock         */
    0,                                              /* Scroll Lock           */
    0,                                              /* Home key              */
    0,                                              /* Up Arrow              */
    0,                                              /* Page Up               */
  '-',
    0,                                              /* Left Arrow            */
    0,
    0,                                              /* Right Arrow           */
  '+',
    0,                                              /* 79 - End key          */
    0,                                              /* Down Arrow            */
    0,                                              /* Page Down             */
    0,                                              /* Insert Key            */
    0,                                              /* Delete Key            */
    0,   0,   0,
    0,                                              /* F11 Key               */
    0,                                              /* F12 Key               */
    0,                                              /* Other keys undefined  */
};

char kbd_buffer[MAX_KEYS];
unsigned int pos = 0, end = 0;

/* save key in a buffer */
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

/* get a line from the screen */
int kbd_getline(char *line, unsigned int n)
{
    unsigned int i, j, max;

    i = 0;
    j = pos;
    max = MIN(MAX_KEYS - 1, n);
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
void kbd_key(unsigned char scancode)
{
    char key;

    if (scancode > 128)
        return;

    key = kbdus[scancode];

    if (key) {
        print_key(key);
        buffer_key(key);
        if (key == '\n')
            sched_unblock(&kbd_list);
    }
}
