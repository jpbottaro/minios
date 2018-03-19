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

void mymemset(char *to, char c, unsigned int size)
{
    while (size--)
        *(to++) = c;
}

void mymemcpy(char *to, const char *from, unsigned int size)
{
    while (size--)
        *(to++) = *(from++);
}

/* taken from http://www.velocityreviews.com/forums/t317059-itoa-in-pure-c.html */
/* made by user Peter Nilsson */
char *myitoa(unsigned int i, char *to, int base)
{
    static const char symb[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'A', 'B', 'C', 'D', 'E', 'F'};
    char *p = to;
    char *q = to;

    do {
        *q++ = symb[i % base];
    } while (i /= base);

    for (*q = 0; p < --q; p++) {
        char c = *p;
        *p = *q;
        *q = c;
    }

    return to;
}
