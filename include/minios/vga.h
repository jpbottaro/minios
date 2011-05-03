#ifndef _VGA_H
#define _VGA_H

#include <sys/types.h>

void vga_init(void);

int vga_pwrite(u16_t r, u16_t c, const char* msg, int n);
int vga_write(const char* msg, int n);
int vga_pprintf(u16_t r, u16_t c, const char* format, ...);
//                                        __attribute__ ((format (printf, 3, 4)));
int vga_printf(const char* format, ...);

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
