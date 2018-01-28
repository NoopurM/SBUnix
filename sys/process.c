#include <sys/process.h>
#include <unistd.h>
#include <sys/paging.h>
#include <sys/memset.h>
#include <sys/tarfs.h>
#include <sys/elf64.h>
#include <sys/idt.h>

extern char * strcpy(char *src, char *dest); 
extern size_t strlen(const char *s);
extern void fork_isr(); 
extern void syscall_handler(struct idt_regs *regs); 
void load_elf_file(struct task *new_proc, void *elf_start_addr);
volatile struct task *current;
struct task *kthread_1, *kthread_2;
int pid=0;

void init_kernel_threads() {

    kthread_1 = (struct task *)kmalloc(sizeof(struct task));
    kthread_2 = (struct task *)kmalloc(sizeof(struct task));

    kthread_1->kstack[511] = (uint64_t )&kthreads_context_switch_func;
    kthread_2->kstack[511] = (uint64_t )&user_proc_loader;

    kthread_1->kstack_rsp = (uint64_t)(&(kthread_1->kstack[511]));
    kthread_2->kstack_rsp = (uint64_t)(&(kthread_2->kstack[512-16]));

    kthread_1->cr3 = get_cr3();
    kthread_2->cr3 = get_cr3();

    current = kthread_1;
    void *file_name = (void *)"Init_thread";
    memcpy((void*)kthread_1->filename, file_name, strlen((char *)file_name));
    kthread_1->pid = pid++;
    kthread_2->pid = pid++;

}

void wrapper_switch() {
    // new tasks are added at the head of the list

    set_tss_rsp((void *)(current->kstack_rsp-16));
    set_cr3((struct pml4_t *)current->cr3);
    context_switch(current->next, current); // so switch in reverse order in list
    //set cr3
}

void context_switch(volatile struct task *kthread_1, volatile struct task *kthread_2) {

    //set_tss_rsp((void *)(kthread_2->kstack_rsp-16));
    __asm__ volatile (
        "pushq %rax;"
        "pushq %rbx;"
        "pushq %rcx;"
        "pushq %rdx;"
        "pushq %rdi;"
        "pushq %rsi;"
        "pushq %rbp;"
        "pushq %r8;"
        "pushq %r9;"
        "pushq %r10;"
        "pushq %r11;"
        "pushq %r12;"
        "pushq %r13;"
        "pushq %r14;"
        "pushq %r15;"
    );


    //save register rsp in previous thread's kernel stack page	
	__asm__ volatile(
        "movq %%rsp, %0"
        :"=m"(kthread_1->kstack_rsp)
        :
        :"memory"
    );

	
 // save next thread's kernel stack page in the register rsp
	__asm__ volatile(
       "movq %0, %%rsp"
       :
       :"r"(kthread_2->kstack_rsp)
    );

	__asm__ volatile (
        "popq %r15;"
		"popq %r14;"
		"popq %r13;"
		"popq %r12;"
		"popq %r11;"
		"popq %r10;"
		"popq %r9;"
		"popq %r8;"
		"popq %rbp;"
		"popq %rsi;"
		"popq %rdi;"
		"popq %rdx;"
		"popq %rcx;"
		"popq %rbx;"
		"popq %rax;"
    );

	__asm__ volatile (
        "retq"
    );

}

void kthreads_context_switch_func(){
    set_cr3((struct pml4_t *)kthread_2->cr3);
    context_switch(kthread_1, kthread_2);
}

void user_proc_loader(){
    set_cr3((struct pml4_t *)current->cr3);
    switch_to_ring_3();
}

void mmap(uint64_t buf, uint64_t size, uint64_t flags) {
    int syscall_no = 2;
    volatile uint64_t ptr=0;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(ptr)
        :"0"(syscall_no),"b"((uint64_t)(buf)),"c"((uint64_t)(size)),"d"((uint64_t)(flags))
        :"cc","memory"
    );
}

void switch_to_ring_3(){

    //align up
    set_tss_rsp((void *)(current->kstack_rsp-16));

    __asm__ volatile (
    "movq $0x23, %rax;"
    );
    __asm__ volatile (
    "movq %rax, %ds;"
    );
    __asm__ volatile (
    "movq %rax, %es;"
    );
    __asm__ volatile (
    "movq %rax, %fs;"
    );
    __asm__ volatile (
    "movq %rax, %gs;"
    );

    __asm__ volatile (
    "movq %0, %%rax;"
    :
    :"r"(current->ustack_rsp)
    :"cc","memory"
    );

    //SS
    __asm__ volatile (
    "pushq $0x23;"
    );

    //RSP
    __asm__ volatile (
    "pushq %rax;"
    );

    //Flags
    __asm__ volatile (
    "pushfq;"
    );

    __asm__ volatile (
    "popq %rax"
    );

    __asm__ volatile (
    "orq $0x200, %rax;"
    );

    __asm__ volatile (
    "pushq %rax"
    );

    //CS
    __asm__ volatile (
    "pushq $0x2B;"
    );

    //RIP
    __asm__ volatile (
    "pushq %0;"
    :
    :"r"(current->rip)
    );

    __asm__ volatile (
    "iretq;"
    );
}

struct task * init_user_process(void *file_name) {
    struct task *new_proc = (struct task *)kmalloc(sizeof(struct task));
    memcpy((void*)new_proc->filename, file_name, strlen((char *)file_name));
    new_proc->cr3 = (uint64_t)share_kernel_space();


    set_cr3((struct pml4_t *)new_proc->cr3);
    struct mm_struct *mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct)); 
    new_proc->mm = mm;
 
    new_proc->kstack_rsp = (uint64_t)(&(new_proc->kstack[512-16]));

    //load exe
    struct posix_header_ustar *elf_file = get_elf_file_from_tarfs(new_proc->filename);
    //kprintf("\nElf file :%s", elf_file->name);
    load_elf_file(new_proc, (void *)(elf_file+1));


    //make the new process as the current process
    volatile struct task *tmp = current;
    current = new_proc;
    current->next = tmp;
    return new_proc;
}

int fork_child(){
    volatile struct task *child_proc = (struct task*)kmalloc(sizeof(struct task));
    memcpy((void*)child_proc->filename, "child_proc", strlen("child_proc"));
    child_proc->cr3 = (uint64_t)share_kernel_space();
    child_proc->pid = pid++;
    copy_child_page_table(child_proc->cr3);

    //RR: No need to set CR3 since we not mapping any page, we are just copy structures data into physical pages.
    //set_cr3((struct pml4_t *)child_proc->cr3);

    /*Copy all fds */
   
    /*Copy mm content of parent process.*/
    child_proc->mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct)); 
    memcpy((void *)(child_proc->mm), (void *)(current->mm), sizeof(struct mm_struct)); 
    child_proc->mm->mmap = NULL;

    /*Copy vmas */
    struct vm_area_struct *parent_vma = current->mm->mmap;
    struct vm_area_struct *last_vma=NULL;
    while(parent_vma != NULL) {
        struct vm_area_struct *child_vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
        if (child_proc->mm->mmap == NULL) {
            child_proc->mm->mmap = child_vma;
        } else {
            child_proc->mm->current = child_vma;
            last_vma->vm_next = child_vma;
        }
        memcpy(child_vma, parent_vma, sizeof(struct vm_area_struct));
        if(parent_vma->vm_file) {
            child_vma->vm_file = (struct file *)kmalloc(sizeof(struct file));
            memcpy(child_vma->vm_file, parent_vma->vm_file, sizeof(struct file));
        }
        last_vma = child_vma;
        parent_vma = parent_vma->vm_next;
        child_vma = NULL;
    }


    /*Copy the content of stack.*/
    // Create your kstack 5 values for iretq, then rax, then 16 0's for yield pops then context switch func addr,
    // then 16 0's for context switch pops.
    child_proc->kstack[509] = current->kstack[511-18]; //Just make sure that there are 18 0's in kstack otherwise change
    //its value.
    child_proc->kstack[508] = current->kstack[511-18-1];
    child_proc->kstack[507] = current->kstack[511-18-2];
    child_proc->kstack[506] = current->kstack[511-18-3];
    child_proc->kstack[505] = current->kstack[511-18-4];
    
    child_proc->kstack[504] = 0;
    child_proc->kstack[509-14-6] = (uint64_t )&fork_isr;
    child_proc->kstack_rsp = (uint64_t)(&(child_proc->kstack[509-14-6-15]));
    child_proc->ustack_rsp = current->ustack_rsp;
    //set_cr3((struct pml4_t *)parent->cr3);
    
    child_proc->next = current;
    current = child_proc;
    return child_proc->pid;
}

void exec_child(char *filename, char **argv, char **envp) {
    //kprintf("\nCreating user process :%s",filename);
    char argv_strs[6][64];
    //char envp_strs[10][50]={0};
    uint64_t argc=0, envp_len = 0;
    int i,j;
    envp_len = envp_len;

    /*Copy the arguments passed.*/
    for(i = 0; i < 10; ++i) {
        memset(argv_strs[i], '\0', 1);
    }
	strcpy(filename, argv_strs[0]);
    argc++;
    if (argv) {
        for(i=0, j=1; argv[i] != NULL; i++, j++, argc++) {
            strcpy(argv[i], argv_strs[j]);
        }
    }
    /*if (envp) {
        for(j=0; envp[j] !=NULL; j++, envp_len++) {
            strcpy(envp[j], envp_strs[j]);
        }
    }*/
    
    struct task *user_proc = init_user_process(filename);
    
    /*Push those arguments to user stack of process and update ustack_rsp.*/
	void *ptr = (void *)(STACK_START_ADDR + 0x1000 - 16 - sizeof(argv_strs));
    memcpy(ptr, (void *)argv_strs, sizeof(argv_strs));

	i=argc;
    for(; i>0; i--){
		*(uint64_t*)(ptr - 8*i) = (uint64_t)ptr + (argc-i)*64;
	}

    ptr = ptr - 8*argc;
    user_proc->ustack_rsp = (uint64_t)ptr;
    
	switch_to_ring_3();
    return;
}

void sys_exit(){
    volatile struct task *temp;
    temp = current;
    current = current->next;
    temp->next = NULL;
    set_tss_rsp((void *)(current->kstack_rsp-16));
    set_cr3((struct pml4_t *)current->cr3);
    context_switch(temp, current);
}

void sys_ps() {
    volatile struct task *ptr = current;
    while(ptr->next != NULL) {
        kprintf("%d %s\n",ptr->pid, ptr->filename);
        ptr = ptr->next;
    }
    kprintf("%d %s\n",ptr->pid, ptr->filename); // last process in list
}

int sys_chdir(char *arg1) {
        strcpy(arg1, (char *)(current->filename));
        return 0;
}
