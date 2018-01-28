#include <sys/idt.h>
#include <sys/kprintf.h>
#include <unistd.h>
#include <string.h>
#include <sys/paging.h>
#include <sys/page_alloc.h>
#include <sys/process.h>
#include <sys/system.h>
#include <sys/tarfs.h>
#include <dirent.h>
#include <sys/memset.h>

struct fd_node *head;
int fd_count = 1;
extern void sys_exit();
extern void sys_ps();
extern DIR* sys_chdir(char *path);
extern volatile struct task *current;

void page_fault_handler_isr(struct idt_regs *regs) {
    //kprintf("Page fault occurred.");
    uint64_t addr=0;
    //tmp_addr; 
    uint64_t page_addr=0, phys_addr=0;
    __asm__ volatile("movq %%cr2, %0;" 
                     :"=r"(addr)
                     :
                     );
    //while(1);
    //if (addr >= STACK_START_ADDR && addr <= STACK_START_ADDR+PAGE_SIZE) {
    //    addr = STACK_START_ADDR;
    //}     
    //kprintf("\nPage fault for addr :%p", addr);
    //if (addr &0x105){
    //    tmp_addr = get_phys_addr_of_present_virt_page(addr);
    //    tmp_addr = ((tmp_addr/0x1000))*0x1000;
   //}
    page_addr = (uint64_t)kmalloc(PAGE_SIZE);
    phys_addr = __get_phys_addr(page_addr);
    map_virtual_to_physical_addr_userspace(addr, phys_addr);
    //if (addr & 0x100) {
    //    memcpy((void *)phys_addr, (void *)tmp_addr, PAGE_SIZE);
    //    kprintf("\ncow bit was set.");
    //}
}
int sys_write(uint64_t fd, uint64_t buf, uint64_t count) {
    kprintf("%s",(char*)buf);
    return 0;
}

uint64_t sys_mmap(uint64_t addr, uint32_t size, uint64_t flags) {
    kprintf("sys_map : addr [%x] len [%d] flags [%d]", addr, size, flags);
    struct mm_struct *mm=NULL;
    struct vm_area_struct *vma=NULL;
    uint64_t ret_addr=addr;

    //TODO: Check whether given address lies given process address space or not.

    mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct));
    mm->mmap = NULL;

    vma = (struct vm_area_struct *)kmalloc(sizeof(struct vm_area_struct));
    if (mm->mmap == NULL) {
        mm->mmap = vma;
    }
    vma->start = addr;
    vma->end = addr + PAGE_SIZE;
    vma->mm = mm;
    vma->vm_flags = 2;
    vma->vm_next = NULL;
    vma->vm_file = NULL;
    return ret_addr;
}

int sys_read(uint64_t fd, uint64_t buf, uint64_t count) {
    //int finalcount = 0;
    //kprintf("\nIn sys_read");
    if(fd == 0) {
        update_buffer((char*)buf, count);
    } else {
        kprintf("\n Only read from terminal is supported!");
    }

    return count;
}

int sys_fork() {

    //copy parent thread entries to child thread entries
    int child = fork_child();
    return child;
}

int sys_execve(uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    exec_child(arg1, arg2, arg3);
    return 1;
}

void sys_yield() {
    wrapper_switch();
}


int sys_open(char *arg1, uint64_t arg2) {

    struct elf_node *node = search_node(arg1);
    struct fd_node *ptr;
    struct fd_node *temp;
    if(node == NULL) {
        return -1;
    }
    if(head == NULL) {
        head = (struct fd_node *)kmalloc(sizeof(struct fd_node));
        head->node = node;
        head->fd = fd_count;
        head->next = NULL;
    }
    else {
        temp = (struct fd_node *)kmalloc(sizeof(struct fd_node));
        temp->node = node;
        temp->fd = fd_count;
        temp->next = NULL;
        ptr = head;
        while(ptr->next != NULL) {
            ptr = ptr->next;
        }
        ptr->next = temp;
    }
    int fd = fd_count;
    fd_count++;
    return fd;
}

struct elf_node *search_fd_list(uint64_t fd) {
     struct fd_node *ptr = head;
     while(ptr != NULL) {
         if(ptr->fd == fd) {
             return (ptr->node);
         }
         ptr=ptr->next;
     }
     return NULL;
}

int sys_read_generic(uint64_t fd, uint64_t addr, uint64_t length) {
       uint64_t start;
       uint64_t end;
       if(fd == 0) {
           length = sys_read(fd, addr, length);
       }
       else {
            struct elf_node *node = search_fd_list(fd);
            if(node == NULL) {
                return -1;
            }

            start = node->last_read;
            end = node->header_address + 512 + node->filesize;

            uint64_t sizeleft = end - start;
            if(length < sizeleft)
                memcpy((void *)addr,(void *)start, length);
            else {
                length = sizeleft;
                memcpy((void *)addr,(void *)start, length);
            }
            node->last_read = node->last_read + length;
       }
       return length;
}

int sys_close(int fd) {
   struct fd_node *ptr = head;
   struct fd_node *prev = NULL;
   if(head->fd == fd) {
       head=head->next;
       return 0;
   }
   while((ptr->fd != fd) && (ptr != NULL)) {
       if(ptr != NULL) {
        prev = ptr;
        ptr = ptr->next;
       } 
   }
   if(ptr != NULL && prev!=NULL) {
        prev->next = ptr->next;
   }
   else {
		return -1;
	}
   return 0;
}

DIR *sys_opendir(char *arg1) {
    struct elf_node *node = search_node(arg1);
    if(node == NULL) {
        return NULL;
    }
    if(node->type != DIRECTORY_TYPE) {
        return NULL;
    }
    DIR *newdir = (DIR*)kmalloc(sizeof(DIR));
    newdir->node = node;
    newdir->child_no = 0;
    return newdir;
}

struct dirent *sys_readdir(DIR *dirp) {
    //for(int i=0; i<NAME_MAX+1; i++) {
        dirp->dentry.d_name[0]='\0';
    //}
    if(dirp->node->child[dirp->child_no]->filename[0] != '\0') {
        strcpy(dirp->node->child[dirp->child_no]->filename, dirp->dentry.d_name);
        dirp->child_no++;
    } else {
        return NULL;
    }
    return &dirp->dentry;
}

int sys_closedir(DIR *dirp) {
    if(dirp->node->type != DIRECTORY_TYPE) {
        return -1;
    }
    dirp->node = NULL;
    return 0;
}

int sys_listf(char *arg1) {
    if(strcmp(arg1,".") == 0)
        strcpy((char*)(current->filename), arg1);

    DIR *dir = sys_opendir(arg1);
    volatile struct dirent *d;
    d = sys_readdir(dir);
    while(d != NULL) {
        kprintf("%s\n",d->d_name);
        d = sys_readdir(dir);
    }
    return 0;
}

int sys_catf(char *arg1) {
    int fd = 0, bytes_read=0;
    char buf[2];
    fd = sys_open(arg1, O_RDONLY);
    while(1) {
        bytes_read = sys_read_generic(fd,(uint64_t)buf,1);
        if(bytes_read == 0)
            break;
        //write(1, buf, bytes_read);
        kprintf("%s",buf);
    }
    sys_close(fd);
    return 0;
}

void syscall_handler(struct idt_regs *regs) {
    //kprintf("Syscall handler called for syscall no :%d", regs->rax);
    int syscall_no=0;
    uint64_t arg1=0, arg2=0, arg3=0;
    uint64_t ret=0;
    __asm__ volatile (
        "movq %%rax, %0;"
        "movq %%rbx, %1;"
        "movq %%rcx, %2;"
        "movq %%rdx, %3;"
        :"=m"(syscall_no),"=m"(arg1),"=m"(arg2),"=m"(arg3)
        :
        :"rax", "rsi", "rcx", "rbx", "rdx", "rdi"
    );
    if (syscall_no == 1) {
        ret = sys_write(arg1, arg2, arg3);
    } else if (syscall_no == 2) {
        ret = sys_mmap(arg1, arg2, arg3);
    } else if (syscall_no == 3) {
        ret = sys_read_generic(arg1, arg2, arg3);
    } else if (syscall_no == 4) {
        ret = sys_fork();
    } else if (syscall_no == 5) {
        ret = sys_execve(arg1, arg2, arg3);
    } else if (syscall_no == 6) {
        sys_yield();
    } else if (syscall_no == 7) {
        ret = sys_open((char*)arg1, arg2);
    } else if (syscall_no == 8) {
		ret = sys_close(arg1);
	} else if (syscall_no == 9) {
        ret = (uint64_t)sys_opendir((char *)arg1);
    } else if (syscall_no == 10) {
        ret = (uint64_t)sys_readdir((DIR*)arg1);
    } else if (syscall_no == 11) {
        ret = (uint64_t)sys_closedir((DIR*)arg1);
    } else if (syscall_no == 12) {
        ret = sys_listf((char *)arg1);
    } else if (syscall_no == 13) {
        ret = sys_catf((char *)arg1);
    } else if (syscall_no == 14) {
        sys_exit();
    } else if (syscall_no == 15) {
        sys_ps();
    } else if (syscall_no == 16) {
        sys_chdir((char *)arg1);
    }
    __asm__ volatile ("movq %0,%%rax;"
                      :
                      :"r"(ret)
                      :"rax"
                     );
}
