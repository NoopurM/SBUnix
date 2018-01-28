#include <sys/paging.h>
#include <sys/memset.h>
#include <sys/memcpy.h>

uint64_t cr3;
struct pml4_t *pml4=NULL;
extern struct Page *head;

int __get_index(uint64_t virt_addr, int shift_bits) {
    int index = -1;
    virt_addr = virt_addr >> shift_bits;
    index = virt_addr & 0x1FF; 
    return index;
}

void load_cr3() {
    __asm__ volatile("movq %0, %%cr3":: "b"(cr3));
}

void set_cr3(struct pml4_t *cr3) {
    uint64_t newcr3 = (uint64_t)cr3;
    __asm__ volatile("movq %0, %%cr3":: "b"(newcr3));
}

uint64_t get_cr3() {
    uint64_t cr3=0;
    __asm__ volatile ("movq %%cr3, %0":"=r"(cr3));
    return cr3;
}

int init_paging(uint64_t physbase, uint64_t physfree) { 
    struct pdpt_t *pdpt=NULL;
    struct pdt_t *pdt=NULL;
    struct pte_t *pte=NULL;
    uint64_t kernmem_start = KERNBASE + physbase;
    int pml4_index, pdpt_index, pdt_index, pte_index;
    uint64_t pdpt_entry, pdt_entry, pte_entry, virtaddr;

    //Allocate 1 page for holding pml4 table.
    virtaddr = (uint64_t)__get_pages(1);
    pml4 = (struct pml4_t *)__get_phys_addr(virtaddr);
    if (pml4 == NULL) {
        kprintf("\nInvalid pml4 allocated.");
        return -1;
    }
    cr3 = (uint64_t)pml4;

    //39-47 bits of virtual address is pml4 index.
    pml4_index = __get_index(kernmem_start, 39);
    virtaddr = (uint64_t)__get_pages(1);
    pdpt = (struct pdpt_t *)__get_phys_addr(virtaddr);
    if (pdpt == NULL) {
        kprintf("\nInvalid pdpt allocated.");
        return -1;
    }
    pdpt_entry = (uint64_t)pdpt | PAGE_RIGHTS;
    pml4->table_entries[pml4_index] = pdpt_entry;

    //30-38 bits virtual address is pdpt index.
    pdpt_index = __get_index(kernmem_start, 30);
    virtaddr = (uint64_t)__get_pages(1);
    pdt = (struct pdt_t *)__get_phys_addr(virtaddr);
    if (pdt == NULL) {
        kprintf("\nInvalid pdt allocated.");
        return -1;
    }
    pdt_entry = (uint64_t)pdt | PAGE_RIGHTS;
    pdpt->table_entries[pdpt_index] = pdt_entry;
  
    //21-29 bits virtual addree is pdt index.
    pdt_index = __get_index(kernmem_start, 21);
    virtaddr = (uint64_t)__get_pages(1);
    pte = (struct pte_t *)__get_phys_addr(virtaddr);
    if (pte == NULL) {
        kprintf("\nInvalid pte allocated.");
        return -1;
    }
    pte_entry = (uint64_t)pte | PAGE_RIGHTS;
    pdt->table_entries[pdt_index] = pte_entry;
   
    //Map kernel memory in physbase to physfree.
    for(; kernmem_start < (KERNBASE + physfree); kernmem_start += PAGE_SIZE, physbase += PAGE_SIZE) {
        pte_index = __get_index(kernmem_start, 12);
        pte->table_entries[pte_index] = physbase | PAGE_RIGHTS; 
    }
    return 0;
}

void map_virtual_to_physical_addr(uint64_t virtaddr, uint64_t physaddr) {
    struct pdpt_t *pdpt=NULL;
    struct pdt_t *pdt=NULL;
    struct pte_t *pte=NULL;
    int pml4_index=0, pdpt_index=0, pdt_index=0, pte_index=0;
    uint64_t pml4_entry=0, pdpt_entry=0, pdt_entry=0, pte_entry=0, virtaddr_page;

    pml4_index = __get_index(virtaddr, 39);
    pml4_entry = pml4->table_entries[pml4_index];
    if (pml4_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdpt = (struct pdpt_t *)(pml4_entry & 0xfffffffffffff000);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pdpt = (struct pdpt_t *)__get_phys_addr(virtaddr_page);
        pdpt_entry = (uint64_t)pdpt | PAGE_RIGHTS;
        pml4->table_entries[pml4_index] = pdpt_entry;
    }
    
    pdpt_index = __get_index(virtaddr, 30);
    pdpt_entry = pdpt->table_entries[pdpt_index];
    if (pdpt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdt = (struct pdt_t *)(pdpt_entry & 0xfffffffffffff000);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pdt = (struct pdt_t *)__get_phys_addr(virtaddr_page);
        pdt_entry = (uint64_t)pdt | PAGE_RIGHTS;
        pdpt->table_entries[pdpt_index] = pdt_entry;
    }
    
    pdt_index = __get_index(virtaddr, 21);
    pdt_entry = pdt->table_entries[pdt_index];
    if (pdt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pte = (struct pte_t *)(pdt_entry & 0xfffffffffffff000);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pte = (struct pte_t *)__get_phys_addr(virtaddr_page);
        pte_entry = (uint64_t)pte | PAGE_RIGHTS;
        pdt->table_entries[pdt_index] = pte_entry;
    }

    pte_index = __get_index(virtaddr, 12);
    pte->table_entries[pte_index] = physaddr | PAGE_RIGHTS;
}

void map_virtual_to_physical_addr_userspace(uint64_t virtaddr, uint64_t physaddr) {
    volatile struct pml4_t *current_pml4=NULL;
    volatile struct pdpt_t *pdpt=NULL;
    volatile struct pdt_t *pdt=NULL;
    volatile struct pte_t *pte=NULL;
    volatile int pml4_index=0, pdpt_index=0, pdt_index=0, pte_index=0;
    volatile uint64_t pml4_entry=0, pdpt_entry=0, pdt_entry=0, pte_entry=0, virtaddr_page;

    current_pml4 = (struct pml4_t *)get_cr3();
    current_pml4 = (struct pml4_t *)(KERNBASE + (uint64_t)current_pml4);

    pml4_index = __get_index(virtaddr, 39);
    pml4_entry = current_pml4->table_entries[pml4_index];
    if (pml4_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdpt = (struct pdpt_t *)(pml4_entry & 0xfffffffffffff000);
        pdpt = (struct pdpt_t *)(KERNBASE + (uint64_t)pdpt);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pdpt = (struct pdpt_t *)__get_phys_addr(virtaddr_page);
        pdpt_entry = (uint64_t)pdpt | PAGE_RIGHTS;
        current_pml4->table_entries[pml4_index] = pdpt_entry;
        pdpt = (struct pdpt_t *)virtaddr_page;
    }
    cr3 = __get_phys_addr((uint64_t)current_pml4);

    pdpt_index = __get_index(virtaddr, 30);
    pdpt_entry = pdpt->table_entries[pdpt_index];
    if (pdpt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdt = (struct pdt_t *)(pdpt_entry & 0xfffffffffffff000);
        pdt = (struct pdt_t *)(KERNBASE + (uint64_t)pdt);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pdt = (struct pdt_t *)__get_phys_addr(virtaddr_page);
        pdt_entry = (uint64_t)pdt | PAGE_RIGHTS;
        pdpt->table_entries[pdpt_index] = pdt_entry;
        pdt = (struct pdt_t *)virtaddr_page;
    }
    
    pdt_index = __get_index(virtaddr, 21);
    pdt_entry = pdt->table_entries[pdt_index];
    if (pdt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pte = (struct pte_t *)(pdt_entry & 0xfffffffffffff000);
        pte = (struct pte_t *)(KERNBASE + (uint64_t)pte);
    } else {
        virtaddr_page = (uint64_t)__get_pages(1);
        pte = (struct pte_t *)__get_phys_addr(virtaddr_page);
        pte_entry = (uint64_t)pte | PAGE_RIGHTS;
        pdt->table_entries[pdt_index] = pte_entry;
        pte = (struct pte_t *)virtaddr_page;
    }

    pte_index = __get_index(virtaddr, 12);
    pte->table_entries[pte_index] = physaddr | PAGE_RIGHTS;
}

/*
 * Map physical addr 0 to maximum availabe physical addr to
 * virtual addr starting from 0xffffffff00000000.
 */
void setup_identity_paging(uint64_t max_physaddr, uint64_t physfree) {
    uint64_t physaddr=0, virtaddr=0;
    
    //We need to map physical addr space to virt addr space before loading cr3 because after
    //that we can not access physical space.
    for(physaddr=physfree, virtaddr=KERNBASE+physfree; physaddr <= max_physaddr; physaddr += PAGE_SIZE, virtaddr += PAGE_SIZE) {
        map_virtual_to_physical_addr(virtaddr, physaddr); 
    }

    //Map video memory physical address to virtual address.
    map_virtual_to_physical_addr((uint64_t)0xffffffff800b8000UL, 0xb8000UL);
}

void * kmalloc(uint32_t size) {
    int npages = 0;
    void *virtaddr;
    if (size % PAGE_SIZE) {
        npages = size/PAGE_SIZE + 1;
    } else {
        npages = size/PAGE_SIZE;
    }
    virtaddr = (void *)__get_pages(npages);

    // First 8 bytes are initailised with the next address, the remaining are 0
    // hence memset not required
    //memset(virtaddr, 0, (npages * PAGE_SIZE));
    return virtaddr;
}

/* It copies kernel page tables to new user process page tables.
 * This will map kernel at same higher location in all user processes.
 */
void * share_kernel_space() {
    uint64_t virtaddr;
    virtaddr = (uint64_t)__get_pages(1);
    struct pml4_t *newpml4 = (struct pml4_t *)__get_phys_addr(virtaddr);
    struct pml4_t *currpml4 = (struct pml4_t *)get_cr3();
    
    currpml4 = (struct pml4_t *)((uint64_t)currpml4 + KERNBASE);

    //Since kernel is mapped at higher addresses it will be always 511st entry.
    ((struct pml4_t*)((uint64_t)newpml4 + KERNBASE))->table_entries[511] = currpml4->table_entries[511];

    return (void*)newpml4;
}

void copy_child_page_table(uint64_t child_cr3) {
    volatile struct pml4_t *child_pml4_phys_addr = (struct pml4_t *)child_cr3;
    volatile struct pml4_t *parent_pml4_phys_addr = (struct pml4_t *)get_cr3();
    volatile struct pdpt_t *parent_pdpt=NULL, *child_pdpt=NULL;
    volatile struct pdt_t *parent_pdt=NULL, *child_pdt=NULL;
    volatile struct pte_t *parent_pte=NULL, *child_pte=NULL;
    volatile uint64_t parent_pml4_entry=0, parent_pdpt_entry=0, parent_pdt_entry=0, parent_pte_entry=0, virtaddr_page,
    parent_phys_page, child_phys_page;
    volatile uint64_t child_pdpt_entry=0, child_pdt_entry=0, child_pte_entry=0;
    int pml4_index, pdpt_index, pdt_index, pte_index;

    for(pml4_index = 0; pml4_index < 511 ; pml4_index++) {
        parent_pml4_entry = ((struct pml4_t *)((uint64_t)parent_pml4_phys_addr + KERNBASE))->table_entries[pml4_index];
        
        if (parent_pml4_entry & 0x1) {
            //Mask first 12 bits which are flags and get address from entry.
            virtaddr_page = (uint64_t)__get_pages(1);
            child_pdpt = (struct pdpt_t *)__get_phys_addr(virtaddr_page);
            child_pdpt_entry = (uint64_t)child_pdpt | PAGE_RIGHTS;
            ((struct pml4_t *)((uint64_t)child_pml4_phys_addr + KERNBASE))->table_entries[pml4_index] = child_pdpt_entry;
            child_pdpt = (struct pdpt_t *)virtaddr_page;
            
            parent_pdpt = (struct pdpt_t *)(parent_pml4_entry & 0xfffffffffffff000);
            for(pdpt_index = 0; pdpt_index < 512; pdpt_index++) {
                parent_pdpt_entry = ((struct pdpt_t *)((uint64_t)parent_pdpt + KERNBASE))->table_entries[pdpt_index];

                if (parent_pdpt_entry & 0x1) {
                    //Mask first 12 bits which are flags and get address from entry.
                    virtaddr_page = (uint64_t)__get_pages(1);
                    child_pdt = (struct pdt_t *)__get_phys_addr(virtaddr_page);
                    child_pdt_entry = (uint64_t)child_pdt | PAGE_RIGHTS;
                    child_pdpt->table_entries[pdpt_index] = child_pdt_entry;
                    child_pdt = (struct pdt_t *)virtaddr_page;
                    
                    parent_pdt = (struct pdt_t *)(parent_pdpt_entry & 0xfffffffffffff000);
                    for(pdt_index = 0; pdt_index < 512; pdt_index++) {
                        parent_pdt_entry = ((struct pdt_t *)((uint64_t)parent_pdt + KERNBASE))->table_entries[pdt_index];

                        if (parent_pdt_entry & 0x1) {
                            //Mask first 12 bits which are flags and get address from entry.
                            virtaddr_page = (uint64_t)__get_pages(1);
                            child_pte = (struct pte_t *)__get_phys_addr(virtaddr_page);
                            child_pte_entry = (uint64_t)child_pte | PAGE_RIGHTS;
                            child_pdt->table_entries[pdt_index] = child_pte_entry;
                            child_pte = (struct pte_t *)virtaddr_page;
                            
                            parent_pte = (struct pte_t *)(parent_pdt_entry & 0xfffffffffffff000);
                            for(pte_index = 0; pte_index<512; pte_index++) {
                                parent_pte_entry = ((struct pte_t *)((uint64_t)parent_pte + KERNBASE))->table_entries[pte_index];
                                if (parent_pte_entry & 0x1) {
                                    parent_phys_page = (uint64_t)parent_pte_entry & 0xfffffffffffff000;
                                    //phys_page = phys_page | 0x105; //setting COW bit and user bit.
                                    virtaddr_page = (uint64_t)__get_pages(1);
                                    child_phys_page = __get_phys_addr(virtaddr_page);
                                    //kprintf("\n %x %d %d %d %d" , parent_phys_page, parent_phys_page+KERNBASE,
                                    //pml4_index, pdpt_index, pdt_index, pte_index);
                                    memcpy((void *)virtaddr_page, (void *)(parent_phys_page+KERNBASE), 4096);
                                    child_pte->table_entries[pte_index] = child_phys_page | 0x7;
                                    //((struct pte_t *)((uint64_t)parent_pte + KERNBASE))->table_entries[pte_index] = phys_page;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
uint64_t get_phys_addr_of_present_virt_page(uint64_t virtaddr) {
    volatile struct pml4_t *current_pml4=NULL;
    volatile struct pdpt_t *pdpt=NULL;
    volatile struct pdt_t *pdt=NULL;
    volatile struct pte_t *pte=NULL;
    int pml4_index=0, pdpt_index=0, pdt_index=0, pte_index=0;
    volatile uint64_t pml4_entry=0, pdpt_entry=0, pdt_entry=0, physaddr;

    current_pml4 = (struct pml4_t *)get_cr3();
    current_pml4 = (struct pml4_t *)(KERNBASE + (uint64_t)current_pml4);

    pml4_index = __get_index(virtaddr, 39);
    pml4_entry = current_pml4->table_entries[pml4_index];

    if (pml4_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdpt = (struct pdpt_t *)(pml4_entry & 0xfffffffffffff000);
        pdpt = (struct pdpt_t *)(KERNBASE + (uint64_t)pdpt);
    } else {
        kprintf("This is impossible!!");
    }

    pdpt_index = __get_index(virtaddr, 30);
    pdpt_entry = pdpt->table_entries[pdpt_index];

    if (pdpt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pdt = (struct pdt_t *)(pdpt_entry & 0xfffffffffffff000);
        pdt = (struct pdt_t *)(KERNBASE + (uint64_t)pdt);
    }    

    pdt_index = __get_index(virtaddr, 21);
    pdt_entry = pdt->table_entries[pdt_index];

    if (pdt_entry & 0x1) {
        //Mask first 12 bits which are flags and get address from entry.
        pte = (struct pte_t *)(pdt_entry & 0xfffffffffffff000);
        pte = (struct pte_t *)(KERNBASE + (uint64_t)pte);
    }

    pte_index = __get_index(virtaddr, 12);
    physaddr = pte->table_entries[pte_index];
    return physaddr;
}
