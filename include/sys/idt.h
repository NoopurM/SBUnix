#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)
#define PIC_EOI 0x20

#define ICW1_INIT   0x10
#define ICW1_ICW4   0x01
#define ICW4_8086   0x01

/* Defines an IDT entry */
struct idt_entry {
    uint16_t offset_1;
    uint16_t sel;        /* Our kernel segment goes here! */
    uint8_t ist;
    uint8_t flags;       /* Set using the above table! */
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t always0;     /* This will ALWAYS be set to 0! */
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct idt_regs {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    int err_code;
    uint64_t rflags;
    uint64_t cs;
    uint64_t ss;
} __attribute__((packed));

void idt_load();
void idt_set_gate(unsigned char num, unsigned long long base, unsigned short sel, unsigned char flags);
void idt_install();
void generic_handler_isr();
void PIC_end_of_intr(unsigned char irq);
void PIC_remap(int offset1, int offset2);

extern void general_protection_handler();
extern void page_fault_handler();
extern void isr_handler();
extern void isr32();
extern void isr33();
extern void isr128();

#endif
