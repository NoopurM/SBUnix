#ifndef _PAGING_H
#define _PAGING_H

#include <sys/defs.h>
#include <sys/page_alloc.h>

#define VIRT_ADDR_START 0xffffffff00000000
#define PHYS_ADDR_START 0x0
#define PAGE_RIGHTS 0x07

struct pml4_t {
    uint64_t table_entries[512];
};

struct pdpt_t {
    uint64_t table_entries[512];
};

struct pdt_t {
    uint64_t table_entries[512];
};

struct pte_t {
    uint64_t table_entries[512];
};

int init_paging(uint64_t physbase, uint64_t physfree); 
void load_cr3();
void setup_identity_paging(uint64_t max_physaddr, uint64_t physfree);
void * kmalloc(uint32_t size);
void set_cr3(struct pml4_t *cr3);
uint64_t get_cr3();
void *share_kernel_space();
void map_virtual_to_physical_addr(uint64_t virtaddr, uint64_t physaddr);
void map_virtual_to_physical_addr_userspace(uint64_t virtaddr, uint64_t physaddr);
void copy_child_page_table(uint64_t child_cr3);
uint64_t get_phys_addr_of_present_virt_page(uint64_t virtaddr);
#endif
