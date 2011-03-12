#include <unistd.h>
#include "misc.h"

#define MAX_PS 50

void execute(const char *buf);

char PS1[MAX_PS] = "jpbottaro:/$";
int PS1len = sizeof("jpbottaro:/$");

int main()
{
    char buf[MAX_PS];
    int len;

    while (1) {
        write(STDOUT_FILENO, PS1, PS1len);

        len = read(STDIN_FILENO, buf, MAX_PS);

        execute(buf);
    }
}

#define MAX_LINE 100
#define MAX_ARGS 20

void execute(const char *buf)
{
    unsigned int i, pid;
    char *s, *cmd;
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    /* cmd points to the program's path */
    mystrncpy(line, buf, MAX_LINE);
    s = line;
    while (*s == ' ') s++;
    cmd = s;
    while (*s != ' ') s++;
    *(s++) = '\0';

    /* make argv */
    i = 1;
    argv[0] = s;
    while (*s != '\0') {
        if (*s == ' ') {
            *s = '\0';
            argv[i++] = s + 1;
        }
        s++;
    }
    argv[i] = 0;

    /* create process and wait for it to finish */
    pid = newprocess(cmd, argv);
    waitpid(pid, NULL, 0);
}
