#ifndef __KEYBOARDSCREEN_H__
#define __KEYBOARDSCREEN_H__

#define MAX_LINE 200

void clear_screen();
void keyboard(unsigned char scancode);
int print(const char *str, unsigned int n);
int get_line(char *line, unsigned int n);

#endif /* __KEYBOARDSCREEN_H__ */
