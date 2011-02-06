#ifndef __MISC_H__
#define __MISC_H__

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

int mystrncpy(const char *from, char *to, int len);
int mystrncmp(const char *str1, const char *str2, int len);
void mymemcpy(char *from, char *to, unsigned int size);

#endif /* __MISC_H__ */
