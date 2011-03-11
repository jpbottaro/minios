#ifndef __KEYBOARDSCREEN_H__
#define __KEYBOARDSCREEN_H__

#define MAX_LINE 200

void keyboard(unsigned char scancode);
void print(const char *str, unsigned int n);
int get_line(char *line, unsigned int n);

#endif /* __KEYBOARDSCREEN_H__ */
