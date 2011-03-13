#include "misc.h"

/* string length */
int mystrlen(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; ++i) {}
    return i;
}

/* copy string 'from' to 'to', writing at most 'len' characters */
int mystrncpy(char *to, const char *from, int len)
{
    int i;

    for (i = 0; i < len && from[i] != '\0'; i++)
        to[i] = from[i];
    to[i] = '\0';
    if (i == len && from[i] != '\0')
        return -1;

    return i;
}

/* compare 'str1' to 'str2' on at most 'len' chars, 0 if equal, non-0 otherwise */
int mystrncmp(const char *str1, const char *str2, int len)
{
    int i;

    for (i = 0; i < len && str1[i] != '\0' && str2[i] != '\0'; i++)
        if (str1[i] != str2[i])
            return -1;
    /* check that both ended */
    if (i < len && str1[i] != str2[i])
        return -1;

    return 0;
}

void mymemcpy(char *to, const char *from, unsigned int size)
{
    while (size--)
        *(to++) = *(from++);
}
