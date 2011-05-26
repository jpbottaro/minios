#ifndef __i386_H__
#define __i386_H__

#define LS_INLINE static __inline __attribute__((always_inline))

LS_INLINE void lcr0(unsigned int val);
LS_INLINE unsigned int rcr0(void);
LS_INLINE void lcr1(unsigned int val);
LS_INLINE unsigned int rcr1(void);
LS_INLINE void lcr2(unsigned int val);
LS_INLINE unsigned int rcr2(void);
LS_INLINE void lcr3(unsigned int val);
LS_INLINE unsigned int rcr3(void);
LS_INLINE void lcr4(unsigned int val);
LS_INLINE unsigned int rcr4(void);
LS_INLINE void tlbflush(void);

LS_INLINE unsigned int resp(void);

LS_INLINE void ltr(unsigned short sel);
LS_INLINE unsigned short rtr(void);
LS_INLINE void hlt(void);

LS_INLINE void cli(void);
LS_INLINE void sti(void);

LS_INLINE void breakpoint(void);

LS_INLINE unsigned int reax(void);
LS_INLINE unsigned int rebx(void);
LS_INLINE unsigned int recx(void);
LS_INLINE unsigned int redx(void);
LS_INLINE unsigned int res(void);
LS_INLINE unsigned int rds(void);
LS_INLINE unsigned int rfs(void);
LS_INLINE unsigned int rgs(void);
LS_INLINE unsigned int rss(void);
LS_INLINE unsigned int redi(void);
LS_INLINE unsigned int resi(void);
LS_INLINE unsigned int rebp(void);
LS_INLINE unsigned int cmpxchg(volatile unsigned int *addr,
                                        unsigned int oldval,
                                        unsigned int newval);

LS_INLINE unsigned char inb(int port) {
    unsigned char val;
    __asm __volatile("inb %%dx,%0" : "=a" (val) : "d" (port));
    return val;
}

LS_INLINE void outb(int port, unsigned char data) {
    __asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

LS_INLINE void lcr0(unsigned int val) {
    __asm __volatile("movl %0,%%cr0" : : "r" (val));
}

LS_INLINE unsigned int rcr0(void) {
    unsigned int val;
    __asm __volatile("movl %%cr0,%0" : "=r" (val));
    return val;
}

LS_INLINE void lcr1(unsigned int val) {
    __asm __volatile("movl %0,%%cr1" : : "r" (val));
}

LS_INLINE unsigned int rcr1(void) {
    unsigned int val;
    __asm __volatile("movl %%cr1,%0" : "=r" (val));
    return val;
}

LS_INLINE void lcr2(unsigned int val) {
    __asm __volatile("movl %0,%%cr2" : : "r" (val));
}

LS_INLINE unsigned int rcr2(void) {
    unsigned int val;
    __asm __volatile("movl %%cr2,%0" : "=r" (val));
    return val;
}

LS_INLINE void lcr3(unsigned int val) {
    __asm __volatile("movl %0,%%cr3" : : "r" (val));
}

LS_INLINE unsigned int rcr3(void) {
    unsigned int val;
    __asm __volatile("movl %%cr3,%0" : "=r" (val));
    return val;
}

LS_INLINE void lcr4(unsigned int val) {
    __asm __volatile("movl %0,%%cr4" : : "r" (val));
}

LS_INLINE unsigned int rcr4(void) {
    unsigned int cr4;
    __asm __volatile("movl %%cr4,%0" : "=r" (cr4));
    return cr4;
}
 
LS_INLINE unsigned int resp(void) {
    unsigned int esp;
    __asm __volatile("movl %%esp,%0" : "=r" (esp));
    return esp;
}

LS_INLINE void tlbflush(void) {
    unsigned int cr3;
    __asm __volatile("movl %%cr3,%0" : "=r" (cr3));
    __asm __volatile("movl %0,%%cr3" : : "r" (cr3));
}

LS_INLINE void ltr(unsigned short sel) {
    __asm __volatile("ltr %0" : : "r" (sel));
}

LS_INLINE unsigned short rtr(void) {
    unsigned short sel;
    __asm __volatile("str %0" : "=r" (sel) : );
    return sel;
}

LS_INLINE void hlt(void) {
    __asm __volatile("hlt" : : );
}

LS_INLINE void cli(void) {
    __asm __volatile("cli" : : );
}


LS_INLINE void sti(void) {
    __asm __volatile("sti" : : );
}

LS_INLINE void breakpoint(void) {
    __asm __volatile("xchg %%bx, %%bx" : :);
}

LS_INLINE unsigned int reax(void) {
    unsigned int val;
    __asm __volatile("movl %%eax,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rebx(void) {
    unsigned int val;
    __asm __volatile("movl %%ebx,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int recx(void) {
    unsigned int val;
    __asm __volatile("movl %%ecx,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int redx(void) {
    unsigned int val;
    __asm __volatile("movl %%edx,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int res(void) {
    unsigned int val;
    __asm __volatile("movl %%es,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rds(void) {
    unsigned int val;
    __asm __volatile("movl %%ds,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rfs(void) {
    unsigned int val;
    __asm __volatile("movl %%fs,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rgs(void) {
    unsigned int val;
    __asm __volatile("movl %%gs,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rss(void) {
    unsigned int val;
    __asm __volatile("movl %%ss,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int redi(void) {
    unsigned int val;
    __asm __volatile("movl %%edi,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int resi(void) {
    unsigned int val;
    __asm __volatile("movl %%esi,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int rebp(void) {
    unsigned int val;
    __asm __volatile("movl %%ebp,%0" : "=r" (val));
    return val;
}

LS_INLINE unsigned int cmpxchg(volatile unsigned int *addr,
                                        unsigned int oldval,
                                        unsigned int newval)
{
    unsigned char result;

    // The + in "+m" denotes a read-modify-write operand.
    __asm __volatile("lock cmpxchgl %3, %1 \n setzb %0" :
                     "=qm" (result), "+m" (*addr) :
                     "a" (oldval), "r" (newval));
    return result;
}

#endif
