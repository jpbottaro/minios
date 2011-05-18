#ifndef __KBD_H__
#define __KBD_H__

#define MAX_KEYS 0x100

#include <minios/sched.h>

struct kbd_s {
    char buffer[MAX_KEYS];
    unsigned int pos;
    unsigned int end;

    waiting_list_t list;
};

/* interrupt handler */
void kbd_intr();

void kbd_init(struct kbd_s *k);

void kbd_key(unsigned char scancode);
void kbd_currentkbd(struct kbd_s *k);
int kbd_getline(struct kbd_s *k, char *line, unsigned int n);

#endif /* __KBD_H__ */
