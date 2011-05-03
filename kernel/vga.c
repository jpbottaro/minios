#include <minios/misc.h>
#include <minios/i386.h>
#include <stdarg.h>
#include "vga.h"

#define MAX_LINE 0x100

struct video_char_s {
    char letter;
    char color;
} __attribute__((__packed__));

struct video_char_s (*vram)[25][80] = (struct video_char_s (*)[25][80]) 0xB8000;
unsigned int x = 0, y = 0, xlimit = 0;

/* move the video ram 1 row up */
void scroll_up_vram()
{
    int i, j;

    for (i = 0; i < 24; i++) {
        for (j = 0; j < 80; j++) {
            (*vram)[i][j].letter = (*vram)[i+1][j].letter;
            (*vram)[i][j].color = (*vram)[i+1][j].color;
        }
    }

    for (j = 0; j < 80; j++) {
        (*vram)[24][j].letter = 0;
        (*vram)[24][j].color = VGA_FC_WHITE;
    }
}

/* move cursor to (x,y), we asume the ports are 0x3D4-0x3D5 */
void move_cursor(int x, int y)
{
    unsigned short pos;

    pos = (y * 80) + x;
    /* low register */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    /* high register */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

/* init vga and clear screen */
void vga_init()
{
    int i, j;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 80; j++) {
            (*vram)[i][j].letter = 0;
            (*vram)[i][j].color = VGA_FC_WHITE;
        }
    }

    x = xlimit = y = 0;
    move_cursor(0, 0);
}

/* print 1 key in the screen */
void print_key(char key)
{
    switch (key) {
        case '\n':
            x = xlimit = 0;
            if (y == 24)
                scroll_up_vram();
            else
                y++;
            break;
        case '\b':
            if (x > xlimit) {
                x--;
                (*vram)[y][x].letter = 0;
            }
            break;
        case '\t':
            key = ' ';
        default:
            (*vram)[y][x].letter = key;
            (*vram)[y][x].color = VGA_FC_WHITE;
            x++;
            if (x == 80) {
                x = xlimit = 0;
                if (y == 24)
                    scroll_up_vram();
                else
                    y++;
            }
    }
    move_cursor(x, y);
}

int vga_pwrite(u16_t r, u16_t c, const char* msg, int n)
{
    int i;

    x = c;
    y = r;
    for (i = 0; i < n; ++i)
        print_key(msg[i]);
    xlimit = x;

    return i;
}

int vga_write(const char* msg, int n)
{
    int i;

    for (i = 0; i < n && msg[i] != '\0'; ++i)
        print_key(msg[i]);
    xlimit = x;

   return i;
}

int real_printf(const char* format, va_list ap)
{   
    const char *p, *str;
    u32_t i, j, len;
    int pad;
    char buf[30];

    p = format;
    len = 0;
    for (i = 0; i < MAX_LINE && p[i] != '\0'; ++i) {
        if (p[i] == '%') {
            pad = 0;
            again:
            switch (p[++i]) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    pad = 10 * pad + p[i] - '0';
                    goto again;
                case 's':
                    str = va_arg(ap, const char *);
                    if (pad > 0)
                        pad -= mystrlen(str);
                    while (pad-- > 0) {
                        print_key(' ');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && str[j] != '\0'; ++j) {
                        print_key(str[j]);
                        ++len;
                    }
                    break;
                case 'i':
                    j = va_arg(ap, int);
                    myitoa(j, buf, 10);
                    if (pad > 0)
                        pad -= mystrlen(buf);
                    while (pad-- > 0) {
                        print_key('0');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && buf[j] != '\0'; ++j) {
                        print_key(buf[j]);
                        ++len;
                    }
                    break;
                case 'x':
                    j = va_arg(ap, int);
                    myitoa(j, buf, 16);
                    if (pad > 0)
                        pad -= mystrlen(buf);
                    while (pad-- > 0) {
                        print_key('0');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && buf[j] != '\0'; ++j) {
                        print_key(buf[j]);
                        ++len;
                    }
                    break;
                case '\0':
                    --i;
                    break;
                default:
                    break;
            }
        } else {
            print_key(p[i]);
            ++len;
        }
    }

    xlimit = x;
    return len;
}

int vga_pprintf(u16_t r, u16_t c, const char* format, ...)
{
    va_list ap;
    int len;
    
    x = c;
    y = r;
    va_start(ap, format);
    len = real_printf(format, ap);
    va_end(ap);

    return len;
}

/* print a null terminated string in the screen */
int vga_printf(const char* format, ...)
{
    va_list ap;
    int len;
    
    va_start(ap, format);
    len = real_printf(format, ap);
    va_end(ap);

    return len;
}
