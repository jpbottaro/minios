extern int write(int fd, const void *buf, unsigned int n);

#define MAX_PS 128

int main()
{
    char PS1[MAX_PS] = "jpbottaro:/$";

    write(0, PS1, sizeof("jpbottaro:/$"));

    for(;;) {}
}
