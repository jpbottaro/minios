#ifndef __KEYBOARDSCREEN_H__
#define __KEYBOARDSCREEN_H__

#define MAX_KEYS 0x100

void keyboard(unsigned char scancode);
int get_line(char *line, unsigned int n);

#endif /* __KEYBOARDSCREEN_H__ */
