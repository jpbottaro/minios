#include <unistd.h>
#include "misc.h"

#define MAX_LEN 30
#define MAX_BUF 100

void execute(const char *buf);

char extendcmd[MAX_LEN] = "/bin/";
char user[MAX_LEN] = "jpbottaro";
char pwd[MAX_LEN] = "/";
unsigned int pwd_len = 1;

void writePS1();
void execute(const char *buf);
int sh_cmd(const char *cmd, char *const argv[]);
int updatepwd(const char *path);

int main()
{
    char buf[MAX_BUF];
    int len;

    while (1) {
        writePS1();

        len = read(STDIN_FILENO, buf, MAX_BUF);

        execute(buf);
    }
}

void writePS1()
{
    int i;
    char *s, buf[MAX_LEN * 2 + 2];

    i = 0;
    s = user;
    while (*s != '\0')
        buf[i++] = *(s++);
    buf[i++] = ':';
    s = pwd;
    while (*s != '\0')
        buf[i++] = *(s++);
    buf[i++] = '$';
    buf[i++] = '\0';

    write(STDOUT_FILENO, buf, i);
}

#define MAX_LINE 100
#define MAX_ARGS 20

void execute(const char *buf)
{
    int i, pid, len;
    char *s;
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    /* cmd points to the program's path */
    len = mystrncpy(line, buf, MAX_LINE);
    s = line;

    /* make argv */
    i = 0;
    if (*s != '\0' && !space(*s))
        argv[i++] = s;
    while (s - line < len) {
        if (space(*s)) {
            *s = '\0';
            if (!space(*(s + 1)) && *(s + 1) != '\0')
                argv[i++] = s + 1;
        }
        s++;
    }
    argv[i] = NULL;

    /* no command? */
    if (i == 0)
        return;

    /* if cmd is not a shell cmd, find binary and execute it */
    if (!sh_cmd(argv[0], argv)) {
        /* create process and wait for it to finish */
        mystrncpy(extendcmd + sizeof("/bin/") - 1, argv[0], MAX_LEN);
        if ( (pid = newprocess(argv[0], argv)) < 0 &&
             (pid = newprocess(extendcmd, argv)) < 0) {
            write(STDOUT_FILENO, "Program not found\n",
                          sizeof("Program not found\n"));
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

/* some commands like 'cd' are handled by the shell here */
int sh_cmd(const char *cmd, char *const argv[])
{
    if (mystrncmp(cmd, "cd", sizeof("cd")) == 0) {
        if (argv[1] != NULL) {
            if (chdir(argv[1]) == 0)
                updatepwd(argv[1]);
            else
                write(STDOUT_FILENO, "Directory not found\n",
                        sizeof("Directory not found\n"));
        }
    } else if (mystrncmp(cmd, "pwd", sizeof("pwd")) == 0) {
        write(STDOUT_FILENO, pwd, sizeof(pwd));
        write(STDOUT_FILENO, "\n", 1);
    } else if (mystrncmp(cmd, "exit", sizeof("exit")) == 0) {
        exit(1);
    } else {
        return 0;
    }

    return 1;
}

/* update pwd acording to the given path */
int updatepwd(const char *path)
{
    const char *begin, *end;
    char *s;
 
    begin = path;
    if (*begin == '/')
        pwd_len = 1;
	while (*begin == '/') begin++;
    for (end = begin; *end != '/' && *end != '\0'; ++end) {}

    while (*begin != '\0') {
        if (mystrncmp(begin, "..", 2) == 0) {
            /* remove last dir */
            if (pwd_len > 1) {
                for (s = pwd + pwd_len - 2; *s != '/'; --s) {}
                pwd_len = s - pwd + 1;
                *(s + 1) = '\0';
            }
        } else if (mystrncmp(begin, ".", 1) == 0) {
            /* nothing */
        } else {
            /* add dir to pwd */
            if (pwd_len + end - begin + 1 > MAX_LEN)
                return -1;
            mystrncpy(pwd + pwd_len, begin, end - begin);
            *(pwd + pwd_len + (end - begin)) = '/';
            *(pwd + pwd_len + (end - begin) + 1) = '\0';
            pwd_len += end - begin + 1;
        }

        /* go to the next component */
        begin = end;
	    while (*begin == '/') begin++;
        for (end = begin; *end != '/' && *end != '\0'; ++end) {}
    }

    return 1;
}
