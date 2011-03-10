#ifndef __KEYBOARDSCREEN_H__
#define __KEYBOARDSCREEN_H__

#define MAX_LINE 200

void keyboard(unsigned char scancode);
void get_line(char *line, unsigned int n);
void print(const char *str, unsigned int n);

#endif /* __KEYBOARDSCREEN_H__ */
