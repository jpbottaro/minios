#include "misc.h"

/* copy string 'from' to 'to', writing at most 'len' characters */
int mystrncpy(const char *from, char *to, int len)
{
    int i;

    for (i = 0; i < len && from[i] != '\0'; i++)
        to[i] = from[i];
    if (i == len && from[i] != '\0')
        return -1;
    to[i] = '\0';

    return 0;
}

/* compare 'str1' to 'str2' on at most 'len' chars, 0 if equal, non-0 otherwise */
int mystrncmp(const char *str1, const char *str2, int len)
{
    int i;

    for (i = 0; i < len && *str1 != '\0'; i++) {
        if (*str1 != *str2)
            return -1;
        str1++; str2++;
    }
    /* check that both ended and 'len' was not passed */
    if (i == len || *str2 != '\0')
        return -1;

    return 0;
}

void mymemcpy(char *from, char *to, unsigned int size)
{
    while (size--) {
        *from = *to;
        from++; to++;
    }
}
