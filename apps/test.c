#include <unistd.h>

#define MAX_PS 50

int main()
{
    char PS1[MAX_PS] = "jpbottaro:/$";
    char buf[MAX_PS];
    int len;

    while (1) {
        write(1, PS1, sizeof("jpbottaro:/$"));

        len = read(0, buf, MAX_PS);

        buf[len - 1] = '\n';
        buf[len] = '\0';

        write(1, buf, len + 1);
    }
}
