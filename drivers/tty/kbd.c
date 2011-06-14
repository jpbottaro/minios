#include <minios/misc.h>
#include <minios/dev.h>
#include <minios/sem.h>
#include <sys/queue.h>
#include "kbd.h"
#include "con.h"

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
    KEY_LEFT,                                       /* Left Arrow            */
    0,
    KEY_RIGHT,                                      /* Right Arrow           */
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

struct kbd_s *current_kbd;

void kbd_init(struct kbd_s *k)
{
    k->pos = 0;
    k->end = 0;
    sem_init(&k->data, 0);
}

/* save key in a buffer */
void kbd_buffer_key(struct kbd_s *k, char key)
{
    switch (key) {
        case '\b':
            k->end = (k->end == k->pos) ? k->end :
                     (k->end == 0)      ? MAX_KEYS - 1 :
                                          k->end - 1;
            break;
        case KEY_LEFT: case KEY_RIGHT:
            break;
        default:
            k->buffer[k->end++] = key;
            k->end %= MAX_KEYS;
            if (key == '\n') {
                k->buffer[k->end++] = '\0';
                k->end %= MAX_KEYS;
            }
    }
}

/* get a line from the screen */
int kbd_getline(struct kbd_s *k, char *line, unsigned int n)
{
    unsigned int i, j, max;

    i = 0;
    j = k->pos;
    max = MIN(MAX_KEYS - 1, n);
    while (i < max && j != k->end && k->buffer[j] != '\0') {
        line[i++] = k->buffer[j++];
        j %= MAX_KEYS;
    }

    line[i] = '\0';

    k->pos = (j != k->end && k->buffer[j] == '\0') ? j + 1: j;
    k->pos %= MAX_KEYS;

    return i + 1;
}

void kbd_currentkbd(struct kbd_s *k)
{
    current_kbd = k;
}

/* manage keyboard interruptions */
void kbd_key(unsigned char scancode)
{
    char key;

    if (scancode > 128)
        return;

    key = kbdus[scancode];

    if (key) {
        con_print_key(current_con, key);
        kbd_buffer_key(current_kbd, key);
        if (key == '\n')
            sem_signal(&current_kbd->data);
    }
}
