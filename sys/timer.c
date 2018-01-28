#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/inout.h>

int tick = 0;
int hr = 0, mins = 0, secs = 0;


void timer_isr() {
    int flag = 0, flag_min = 0;
	tick++;
	outb(0x20, 0x20);
    if (tick % 18 == 0) {
        secs++;
        flag = 1;
    } 
    
    if (flag) {
        if (secs % 60 == 0) {
            mins++;
            flag_min = 1;
            secs = 0;
        }
        if ((flag_min) && (mins % 60 == 0)) {
            hr++;
            mins = 0;
        }
        //print_timer(hr, mins, secs);
    }
}

void init_timer(uint32_t frequency) {
		
	// Send the command byte.
	outb(0x43, 0x36);

	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	uint8_t low_byte = (uint8_t)(0xFF);
	uint8_t high_byte = (uint8_t)( 0xFF );

	// Send the frequency divisor.
	outb(0x40, low_byte);
	outb(0x40, high_byte); 

}
