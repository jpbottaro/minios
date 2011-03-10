#include "pic.h"
#include "i386.h"

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

void enable_pic() {
	outb(PIC1_PORT+1, 0x00);
	outb(PIC2_PORT+1, 0x00);
}

void disable_pic() {
	outb(PIC1_PORT+1, 0xFF);
	outb(PIC2_PORT+1, 0xFF);
}
