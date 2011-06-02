#ifndef __CON_H__
#define __CON_H__

#include <minios/dev.h>
#include <minios/vga.h>
#include <sys/types.h>
#include "kbd.h"

#define MAX_CONSOLES 3

struct console_s {
    struct kbd_s kbd;
    struct video_char_s video[25][80];
    int i;
    int x;
    int xlimit;
    int y;
};

void con_init();

void con_print_key(struct console_s *c, char key);
void con_switch(int con_num);
void con_left();
void con_right();

extern struct console_s *current_con;

#endif
