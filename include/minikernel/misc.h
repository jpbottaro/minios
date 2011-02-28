#ifndef __MISC_H__
#define __MISC_H__

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

int mystrncpy(char *to, const char *from, int len);
int mystrncmp(const char *str1, const char *str2, int len);
void mymemcpy(char *to, const char *from, unsigned int size);

#endif /* __MISC_H__ */
