#ifndef __PIC_H__
#define __PIC_H__

void reset_pic(void);
void enable_pic();
void disable_pic();

__inline __attribute__((always_inline)) void reset_intr_pic1(void);
__inline __attribute__((always_inline)) void reset_intr_pic2(void);

#endif
