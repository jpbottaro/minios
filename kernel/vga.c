#include <minios/misc.h>
#include <minios/i386.h>
#include <minios/vga.h>
#include <stdarg.h>

#define MAX_LINE 0x100

struct pos_s {
    int x;
    int y;
};

struct video_char_s (*vram)[25][80] = (struct video_char_s (*)[25][80]) 0xB8000;
unsigned int x = 0, y = 0;

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

    move_cursor(0, 0);
}

/* print 1 key in the screen */
void print_key(struct pos_s *pos, char key)
{
    switch (key) {
        case '\n':
            pos->x = 0;
            if (pos->y == 24)
                scroll_up_vram();
            else
                pos->y++;
            break;
        case '\b':
            if (pos->x > 0) {
                pos->x--;
                (*vram)[pos->y][pos->x].letter = 0;
            }
            break;
        case '\t':
            key = ' ';
        default:
            (*vram)[pos->y][pos->x].letter = key;
            (*vram)[pos->y][pos->x].color = VGA_FC_WHITE;
            pos->x++;
            if (pos->x == 80) {
                pos->x = 0;
                if (pos->y == 24)
                    scroll_up_vram();
                else
                    pos->y++;
            }
    }
    move_cursor(pos->x, pos->y);
}

void vga_print_key(u16_t r, u16_t c, char key)
{
    struct pos_s pos = {.x = c, .y = r};
    print_key(&pos, key);
}

int vga_write(u16_t r, u16_t c, const char* msg, int n)
{
    int i;
    struct pos_s pos;

    pos.x = c;
    pos.y = r;
    for (i = 0; i < n; ++i)
        print_key(&pos, msg[i]);

    move_cursor(pos.x, pos.y);
    return i;
}

int vga_printf(u16_t r, u16_t c, const char* format, ...)
{
    const char *p, *str;
    u32_t i, j, len;
    va_list ap;
    int pad;
    char buf[30];
    struct pos_s pos;

    pos.x = c;
    pos.y = r;
    va_start(ap, format);
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
                        print_key(&pos, ' ');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && str[j] != '\0'; ++j) {
                        print_key(&pos, str[j]);
                        ++len;
                    }
                    break;
                case 'i':
                    j = va_arg(ap, int);
                    myitoa(j, buf, 10);
                    if (pad > 0)
                        pad -= mystrlen(buf);
                    while (pad-- > 0) {
                        print_key(&pos, '0');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && buf[j] != '\0'; ++j) {
                        print_key(&pos, buf[j]);
                        ++len;
                    }
                    break;
                case 'x':
                    j = va_arg(ap, int);
                    myitoa(j, buf, 16);
                    if (pad > 0)
                        pad -= mystrlen(buf);
                    while (pad-- > 0) {
                        print_key(&pos, '0');
                        ++len;
                    }
                    for (j = 0; j < MAX_LINE && buf[j] != '\0'; ++j) {
                        print_key(&pos, buf[j]);
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
            print_key(&pos, p[i]);
            ++len;
        }
    }
    va_end(ap);

    return len;
}
