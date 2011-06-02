#ifndef _VGA_H
#define _VGA_H

#include <sys/types.h>

struct video_char_s {
    char letter;
    char color;
} __attribute__((__packed__));

void vga_clear(void);
void vga_copy_vram(struct video_char_s video[25][80]);
void vga_move_cursor(int x, int y);
void vga_print_key(u16_t r, u16_t c, char key);
void vga_scrollup_vram();
int vga_write(u16_t r, u16_t c, const char* msg, int n);
int vga_printf(u16_t r, u16_t c, const char* format, ...);

/* Paleta de 16 colores */
#define VGA_FC_BLACK   0x00
#define VGA_FC_BLUE    0x01
#define VGA_FC_GREEN   0x02
#define VGA_FC_RED     0x03
#define VGA_FC_CYAN    0x04
#define VGA_FC_MAGENTA 0x05
#define VGA_FC_BROWN   0x06
#define VGA_FC_WHITE   0x07

#define VGA_FC_LIGHT   0x08
#define VGA_FC_BLINK   0x80

#define VGA_BC_BLACK   0x00
#define VGA_BC_BLUE    0x10
#define VGA_BC_GREEN   0x20
#define VGA_BC_CYAN    0x30
#define VGA_BC_RED     0x40
#define VGA_BC_MAGENTA 0x50
#define VGA_BC_BROWN   0x60
#define VGA_BC_WHITE   0x70

#endif
