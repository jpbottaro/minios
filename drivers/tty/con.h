#ifndef __CON_H__
#define __CON_H__

#include <minios/dev.h>
#include <minios/vga.h>
#include <sys/types.h>
#include "kbd.h"

#define MAX_CONSOLES 5

struct console_s {
    struct kbd_s kbd;
    struct video_char_s video[25][80];
    int x;
    int xlimit;
    int y;
};

void con_init();

void con_print_key(struct console_s *c, char key);

extern struct console_s *current_con;

#endif
