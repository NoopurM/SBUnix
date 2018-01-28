#include <sys/defs.h>
#include <sys/system.h>
#include <sys/kprintf.h>
#include <sys/inout.h>
#include <sys/idt.h>

void PIC_end_of_intr(unsigned char irq)
{
	if(irq >= 8) {
		outb(PIC2_COMMAND,PIC_EOI);
    }
	outb(PIC1_COMMAND,PIC_EOI);
}

void PIC_remap(int offset1, int offset2)
{
	unsigned char a1, a2;
 
	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);
 
	outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
 
	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}

struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
void idt_load() {
    __asm__ ("lidt %0" : :"m" (idtp));
}

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void idt_set_gate(unsigned char num, unsigned long long base, unsigned short sel, unsigned char flags)
{
    unsigned long long offset_2, offset_3;
	idt[num].offset_1 = base & 0x000000000000ffff;
	offset_2 = base & 0x00000000ffff0000;
    idt[num].offset_2 = offset_2 >> 16;
    offset_3 = base & 0xffffffff00000000;
    idt[num].offset_3 = offset_3 >> 32;
	idt[num].sel = sel;
	idt[num].flags = flags;
    idt[num].always0 = 0;
    idt[num].ist = 0;
}

/* Installs the IDT */
void idt_install()
{
    idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    idtp.base = ((uint64_t)&idt);
    
    //generic isr handler
    for (int i=0;i<32;i++) {
        if (i == 14) {
	        idt_set_gate(i, (unsigned long long)page_fault_handler, 0x08, 0x8E);
        } else if (i == 13) {
	        idt_set_gate(i, (unsigned long long)general_protection_handler, 0x08, 0x8E);
        } else{
            idt_set_gate(i, (unsigned long long)isr_handler, 0x08, 0x8E);
        }
    }
    
    //Timer
	idt_set_gate(32, (unsigned long long)isr32, 0x08, 0x8E);

    //Keyboard
    idt_set_gate(33, (unsigned long long)isr33, 0x08, 0x8E);
    
    //Syscalls 
    idt_set_gate(128, (unsigned long long)isr128, 0x08, 0xEE);

    idt_load();
}

void generic_handler_isr() {
    kprintf("\nDefault fault handler called");
    __asm__ volatile("hlt");
    //while(1);
}

void general_protection_handler_isr(){
    kprintf("\nGeneral protection fault occurred");
    volatile uint64_t addr;
   __asm__ volatile("movq %%cr2, %0;" 
                     :"=r"(addr)
                     :
                     );
 
   __asm__ volatile("movq %%cr3, %0;" 
                     :"=r"(addr)
                     :
                     );
 __asm__ volatile("hlt");
}
