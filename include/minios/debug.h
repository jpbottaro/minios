#ifndef _DEBUG_H
#define _DEBUG_H

#include <minios/isr.h>
#include <sys/types.h>

void debug_init(void);
void debug_panic(const char *msg);
void debug_kernelpanic(const u32_t* stack, const exp_state* expst);

#include <minios/vga.h>
#include <minios/i386.h>

#ifndef NDEBUG
#define __mac_xstr(s) #s
#define __mac_str(s) __mac_xstr(s)
#define kassert(EXP) { if (!(EXP)) { cli(); vga_write(0, 0, "Assertion failed at " \
  __mac_str(__FILE__)":"__mac_str(__LINE__)": "#EXP, \
  VGA_BC_RED | VGA_FC_WHITE | VGA_FC_LIGHT); hlt(); while(1); } }
#else
#define kassert(EXP) {}
#endif

#endif /* _DEBUG_H */
