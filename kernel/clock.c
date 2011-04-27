#include <minios/i386.h>
#include <sys/types.h>
#include "clock.h"

/* init_timer taken from James Molly in
 * http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs%20and%20the%20PIT.html
 */
void clock_init(u32_t frequency)
{
   /* The value we send to the PIT is the value to divide it's input clock
    * (1193180 Hz) by, to get our required frequency. Important to note is
    * that the divisor must be small enough to fit into 16-bits.
    */
   u32_t divisor = 1193180 / frequency;

   /* Send the command byte */
   outb(0x43, 0x36);

   /* Divisor has to be sent byte-wise, so split here into upper/lower bytes */
   unsigned char l = (unsigned char) (divisor & 0xFF);
   unsigned char h = (unsigned char) ( (divisor>>8) & 0xFF );

   /* Send the frequency divisor */
   outb(0x40, l);
   outb(0x40, h);
}
