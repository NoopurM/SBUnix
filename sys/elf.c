#include <sys/elf64.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include <sys/paging.h>

void load_elf_file(struct task *new_proc, void *elf_start_addr) {
    struct vm_area_struct *last_vma = NULL; 
    uint64_t addr;
    //uint64_t elf_start_addr_header = (uint64_t)elf_start_addr + 512;
    struct Elf64_Ehdr *elf_header = (struct Elf64_Ehdr *)elf_start_addr;
    struct Elf64_Phdr *elf_prgm_header = (struct Elf64_Phdr *)((uint64_t)elf_header + elf_header->e_phoff);
    
    new_proc->rip = elf_header->e_entry;
 
    new_proc->mm->mmap = NULL;
    new_proc->mm->current = NULL;
    //kprintf("\nprgm header phnum :%d",elf_header->e_phnum);
    for (int i=0; i < elf_header->e_phnum; i++) {
        //kprintf("\nprgm header start addr :%p size :%d", elf_prgm_header->p_vaddr, elf_prgm_header->p_memsz);
        //kprintf("\nprgm header flags :%d", elf_prgm_header->p_flags);
        if (elf_prgm_header->p_type == 1){
            struct vm_area_struct *vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
            if (new_proc->mm->mmap == NULL) {
                new_proc->mm->mmap = vma;
            } else {
                new_proc->mm->current = vma;
                last_vma->vm_next = vma;
            }
            vma->start = elf_prgm_header->p_vaddr;
            vma->end = elf_prgm_header->p_vaddr + elf_prgm_header->p_memsz;
            int npages = 0;
            if ((vma->end - vma->start) % PAGE_SIZE) {
                npages = (vma->end - vma->start)/PAGE_SIZE + 1;
            } else {
                npages = (vma->end - vma->start)/PAGE_SIZE;
            }
            addr = vma->start;
            while(npages--){
                uint64_t page_addr = (uint64_t)kmalloc(PAGE_SIZE);
                uint64_t phys_addr = __get_phys_addr(page_addr);
                addr = 0x00000000000000 | addr;
                map_virtual_to_physical_addr_userspace(addr, phys_addr);
                addr += PAGE_SIZE;
            }
            vma->mm = new_proc->mm;
            vma->vm_flags = elf_prgm_header->p_flags;
            
            //If file if read and write or read and execuatable permissions copy code segment.
            if (vma->vm_flags == 5 || vma->vm_flags == 6) {
                new_proc->mm->start_code = elf_prgm_header->p_vaddr;
                new_proc->mm->end_code = elf_prgm_header->p_vaddr + elf_prgm_header->p_memsz;
                vma->vm_file = (struct file *)kmalloc(sizeof(struct file));
                vma->vm_file->start = (uint64_t)elf_header;
                vma->vm_file->pgoff = elf_prgm_header->p_offset;
                vma->vm_file->size = elf_prgm_header->p_filesz;
                memcpy((void *)vma->start, (void *)(vma->vm_file->start + vma->vm_file->pgoff), vma->vm_file->size);
                //TODO: bss and type required??
            }
            last_vma = vma;
        }
        elf_prgm_header++;
    }

    //Allocate heap space for process of one page size
    struct vm_area_struct *vma_heap = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
    if (new_proc->mm->mmap == NULL) {
        new_proc->mm->mmap = vma_heap;
    } else {
        new_proc->mm->current = vma_heap;
        last_vma->vm_next = vma_heap;
    }
    vma_heap->mm = new_proc->mm;
    vma_heap->start = HEAP_START_ADDR;
    new_proc->mm->start_brk = new_proc->mm->brk = HEAP_START_ADDR;
    vma_heap->end = HEAP_START_ADDR + PAGE_SIZE;
    vma_heap->vm_flags = 0x6;
    last_vma = vma_heap;
    
    //Allocate stack space for process of one page size
    struct vm_area_struct *vma_stack = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
    if (new_proc->mm->mmap == NULL) {
        new_proc->mm->mmap = vma_stack;
    } else {
        new_proc->mm->current = vma_stack;
        last_vma->vm_next = vma_stack;
    }
    vma_stack->mm = new_proc->mm;
    //vma_stack->start = STACK_START_ADDR - PAGE_SIZE;
    vma_stack->start = STACK_START_ADDR + PAGE_SIZE;
    //new_proc->mm->start_stack = STACK_START_ADDR - PAGE_SIZE;
    new_proc->mm->start_stack = STACK_START_ADDR + PAGE_SIZE;
    vma_stack->end = STACK_START_ADDR;
    
    uint64_t page_addr = (uint64_t)kmalloc(PAGE_SIZE);
    uint64_t phys_addr = __get_phys_addr(page_addr);
    //kprintf("\nstck :%x", phys_addr);
    //map_virtual_to_physical_addr_userspace(vma_stack->end - PAGE_SIZE, phys_addr);
    map_virtual_to_physical_addr_userspace(vma_stack->end, phys_addr);
 
    vma_stack->vm_flags = 0x6;
    
    //new_proc->ustack_rsp = vma_stack->end - 16;
    new_proc->ustack_rsp = vma_stack->start- 16;
}
