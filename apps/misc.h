#ifndef _MISC_H
#define _MISC_H

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#define space(a) ((a) == ' ' || (a) == '\t' || (a) == '\n')

int mystrlen(const char *str);
int mystrncpy(char *to, const char *from, int len);
int mystrncmp(const char *str1, const char *str2, int len);
void mymemcpy(char *to, const char *from, unsigned int size);
int atoi(char *s);

#endif /* _MISC_H */
