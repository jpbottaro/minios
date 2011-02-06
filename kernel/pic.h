#ifndef __PIC_H__
#define __PIC_H__

void reset_pic(void);
void enable_pic();
void disable_pic();

static __inline __attribute__((always_inline)) void outb(int port, unsigned char data);

__inline __attribute__((always_inline)) void reset_intr_pic1(void);
__inline __attribute__((always_inline)) void reset_intr_pic2(void);

#endif
