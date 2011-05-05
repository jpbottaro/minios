#ifndef __KBD_H__
#define __KBD_H__

#define MAX_KEYS 0x100

extern waiting_list_t kbd_list;

void kbd_key(unsigned char scancode);
int kbd_getline(char *line, unsigned int n);

#endif /* __KBD_H__ */
