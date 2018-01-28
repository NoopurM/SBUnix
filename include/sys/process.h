#include <sys/defs.h>
#include <sys/page_alloc.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/memcpy.h>

#define HEAP_START_ADDR 0x08000000
#define STACK_START_ADDR 0xF0000000

struct task{
   uint64_t kstack[512];
   uint64_t pid;
   uint64_t ustack_rsp; //This pointing to vma stack allocated.
   uint64_t kstack_rsp;
   uint64_t rip;
   uint64_t cr3;
   volatile struct task *next;
   enum { RUNNING, SLEEPING, ZOMBIE } state;
   int exit_status;
   struct mm_struct *mm;
   char filename[50];
};

struct mm_struct {
    struct vm_area_struct *mmap; //pointer to the head of the list of memory region objects
    struct vm_area_struct *current;  //pointer to current vma
    uint64_t start_code, end_code; //initial and final address of executable code
    uint64_t start_data, end_data; //initial and final address of initialized data
    uint64_t start_brk, brk; //initial address of the heap, and current final address of the heap
    uint64_t start_stack; //initial address of user mode stack
    uint64_t arg_start, arg_end; //initial and final address of command line argument
    uint64_t env_start, env_end; //initial and final address of environment variables
    uint64_t rss; //number of page frames allocated to the process
    uint64_t total_vm; //size of the process address space(number of pages)
    uint64_t locked_vm; //number of locked pages that cannot be swapped out

};

struct file {
    uint64_t start;
    uint64_t pgoff;
    uint64_t size;
};

struct vm_area_struct {
       struct mm_struct *mm; //pointer to the memory descriptor that owns the region
       uint64_t start; //first linear address inside the region
       uint64_t end; //first linear address after the region
       struct vm_area_struct *vm_next; //next region in the process list
       uint64_t vm_flags; //flags of the region
       uint64_t type; //type of segment its refering to
       struct file *vm_file; //pointer to the file object of the mapped file
};


struct task *thread1;
struct task *thread2;

void init_kernel_threads();
void schedule();
void context_switch(volatile struct task *thread1, volatile struct task *thread2);
void kthreads_context_switch_func();
void user_proc_loader();
void user_testfunc();
void switch_to_ring_3();
struct task * init_user_process(void *file_name);
size_t write(int fd, const void *buf, size_t size);
int fork_child();
void exec_child();
void wrapper_switch();
