#ifndef __KBD_H__
#define __KBD_H__

#define MAX_KEYS 0x100
#define KEY_LEFT  1
#define KEY_RIGHT 2

#include <minios/sem.h>

struct kbd_s {
    char buffer[MAX_KEYS];
    unsigned int pos;
    unsigned int end;

    sem_t data;
};

/* interrupt handler */
void kbd_intr();
void kbd_init(struct kbd_s *k);
void kbd_key(unsigned char scancode);
void kbd_currentkbd(struct kbd_s *k);
int kbd_getline(struct kbd_s *k, char *line, unsigned int n);

#endif /* __KBD_H__ */
