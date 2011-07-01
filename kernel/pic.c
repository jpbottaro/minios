#include <minios/i386.h>
#include <minios/idt.h>
#include "pic.h"

#define PIC1_PORT 0x20
#define PIC2_PORT 0xA0


__inline __attribute__((always_inline)) void reset_intr_pic1(void) { outb(0x20, 0x20); } 
__inline __attribute__((always_inline)) void reset_intr_pic2(void) { outb(0x20, 0x20); outb(0xA0, 0x20); } 


void reset_pic() {
	outb(PIC1_PORT+0, 0x11); /* IRQs activas x flanco, cascada, y ICW4 */
	outb(PIC1_PORT+1, 0x20); /* Addr */
	outb(PIC1_PORT+1, 0x04); /* PIC1 Master, Slave ingresa Int.x IRQ2 */
	outb(PIC1_PORT+1, 0x01); /* Modo 8086 */
	outb(PIC1_PORT+1, 0xFF); /* Enmasca todas! */

	outb(PIC2_PORT+0, 0x11); /* IRQs activas x flanco, cascada, y ICW4 */
	outb(PIC2_PORT+1, 0x28); /* Addr */
	outb(PIC2_PORT+1, 0x02); /* PIC2 Slave, ingresa Int x IRQ2 */
	outb(PIC2_PORT+1, 0x01); /* Modo 8086 */
	outb(PIC2_PORT+1, 0xFF); /* Enmasca todas! */
}

extern void handler_void();

void enable_pic() {
	outb(PIC1_PORT+1, 0x00);
	outb(PIC2_PORT+1, 0x00);

    /* Ignore the 0x27 interrupt, its just the PIC being an asshole */
    idt_register(0x27, handler_void, DEFAULT_PL);
}

void disable_pic() {
	outb(PIC1_PORT+1, 0xFF);
	outb(PIC2_PORT+1, 0xFF);
}
