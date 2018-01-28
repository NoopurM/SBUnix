#include <sys/defs.h>
/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

void idt_install();
void PIC_remap(int offset1, int offset2);
void PIC_end_of_intr(unsigned char irq);
void init_timer(uint32_t frequency);
void keyboard_handler();
void update_buffer(void *buff_addr, int count);
//void outb(uint16_t port, uint8_t val);
