#include <minios/sem.h>
#include <minios/idt.h>
#include <minios/fs.h>
#include "con.h"

extern void kbd_intr();

struct console_s console[MAX_CONSOLES];
struct console_s *current_con;

size_t con_read(struct file_s *flip, char *buf, size_t n);
ssize_t con_write(struct file_s *flip, char *buf, size_t n);

static struct file_operations_s ops = {
    .read = con_read,
    .write = con_write
};

void con_switch(int con_num)
{
    con_num %= MAX_CONSOLES;
    fs_make_dev("tty", I_CHAR, DEV_TTY, con_num);
    current_con = &console[con_num];
    kbd_currentkbd(&current_con->kbd);

    /* update video ram with new console */
    vga_copy_vram(current_con->video);
}

void con_init()
{
    int i, j, k;
    char tty[] = "tty0";

    /* manage keyboard interruptions */
    idt_register(33, kbd_intr, DEFAULT_PL);

    /* initialize virtual consoles (MAX_CONSOLES < 10) */
    for (i = 0; i < MAX_CONSOLES; i++) {
        tty[3] = i + '0';
        fs_make_dev(tty, I_CHAR, DEV_TTY, i);
        console[i].i = i;
        console[i].x = 0;
        console[i].y = 0;
        console[i].xlimit = 0;
        for (j = 0; j < 25; j++) {
            for (k = 0; k < 80; k++) {
                console[i].video[j][k].letter = 0;
                console[i].video[j][k].color = VGA_FC_WHITE;
            }
        }
        kbd_init(&console[i].kbd);
    }

    /* make current virtual terminal dev */
    con_switch(0);

    dev_register(DEV_TTY, &ops);
}

void con_left()
{
    con_switch((current_con->i + 1) % MAX_CONSOLES);
}

void con_right()
{
    if (current_con->i == 0)
        con_switch(MAX_CONSOLES - 1);
    else
        con_switch(current_con->i - 1);
}

void scrollup(struct video_char_s video[25][80])
{
    int i, j;

    for (i = 0; i < 24; i++) {
        for (j = 0; j < 80; j++) {
            video[i][j].letter = video[i+1][j].letter;
            video[i][j].color = video[i+1][j].color;
        }
    }

    for (j = 0; j < 80; j++) {
        video[24][j].letter = 0;
        video[24][j].color = VGA_FC_WHITE;
    }
}

/* print 1 key in the screen */
void con_print_key(struct console_s *c, char key)
{
    switch (key) {
        case '\n':
            c->x = c->xlimit = 0;
            if (c->y == 24) {
                scrollup(c->video);
                if (c == current_con)
                    vga_scrollup_vram();
            } else {
                c->y++;
            }
            break;
        case '\b':
            if (c->x > c->xlimit) {
                c->x--;
                c->video[c->y][c->x].letter = 0;
                if (c == current_con)
                    vga_print_key(c->y, c->x, 0);
            }
            break;
        case '\t':
            key = ' ';
        default:
            c->video[c->y][c->x].letter = key;
            c->video[c->y][c->x].color = VGA_FC_WHITE;
            if (c == current_con)
                vga_print_key(c->y, c->x, key);
            c->x++;
            if (c->x == 80) {
                c->x = c->xlimit = 0;
                if (c->y == 24) {
                    scrollup(c->video);
                    if (c == current_con)
                        vga_scrollup_vram();
                } else {
                    c->y++;
                }
            }
    }
    if (c == current_con)
        vga_move_cursor(c->x, c->y);
}

size_t con_read(struct file_s *flip, char *buf, size_t n)
{
    int minor = iminor(flip->f_ino);

    sem_wait(&console[minor].kbd.data);

    return kbd_getline(&console[minor].kbd, buf, n);
}

int con_realwrite(struct console_s *con, const char* msg, int n)
{
    int i;

    for (i = 0; i < n; ++i)
        con_print_key(con, msg[i]);
    con->xlimit = con->x;

    return i;
}

ssize_t con_write(struct file_s *flip, char *buf, size_t n)
{
    int minor = iminor(flip->f_ino);

    return con_realwrite(&console[minor], buf, n);
}
