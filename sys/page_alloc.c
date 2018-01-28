#include <sys/page_alloc.h>

struct Page *head=NULL;
struct Page *tail=NULL;

uint64_t __get_phys_addr(uint64_t virtaddr) {
    return (virtaddr - KERNBASE);
}

uint64_t __get_virt_addr(uint64_t physaddr) {
    return (physaddr + KERNBASE);
}

void init_phys_mem(uint64_t physbase, uint64_t physfree, uint64_t start, uint64_t end){
    int npages = 0;
    kprintf("physbase: %p, physfree: %p, base: %p, size: %p", physbase, physfree, start, end);
    
    if(start <= physbase && end >= physfree){
         npages = ((physbase - start)/ PAGE_SIZE) + ((end - physfree)/PAGE_SIZE);
         __create_page_list(KERNBASE+physfree, npages);
    }
    else if(start <= physbase && end <= physfree && end >= physbase){
         npages = (physbase - start)/ PAGE_SIZE;
         __create_page_list(KERNBASE+physfree, npages);
    }
    else if(start >= physbase && end >= physfree && start <= physfree) {
         npages = (end - physfree)/PAGE_SIZE;
         __create_page_list(KERNBASE+physfree, npages);
    }
    else {
        npages = (end - start)/PAGE_SIZE; 
        __create_page_list(KERNBASE+physfree, npages);
    }
    //__print_free_page_list(head);
}
/*
 * This will allocate a single node of linked list which will hold given address
 */
struct Page * __alloc_page(uint64_t addr) {
    struct Page *ptr=NULL;
    ptr = (struct Page *)addr;
    ptr->next = NULL;
    return ptr;
};

/*
 * This function divies the given address range in pages. Returns 0 on success and -1 on failure.
 */
int __create_page_list(uint64_t start_node_addr, int npages) {

    struct Page *tmp_page=NULL;
    int ret = 0;
    kprintf("\nCreating page list of length %d", npages);
    
    //If head is null no free list of pages is created yet.
    if (head == NULL) {
        head = __alloc_page(start_node_addr);
        tail = head;
        npages -= 1;
    }
    while(npages) {
        tmp_page = __alloc_page(((uint64_t)tail+PAGE_SIZE));
        tail->next = tmp_page;
        tail = tail->next; 
        npages--;
    }
    return ret;
}

struct Page *__get_pages(uint32_t npages) {
    struct Page *itr=NULL, *oldHead=head;
    int cnt=1;
    for(itr=head; cnt<npages; itr=itr->next, cnt++);
    head = itr->next;
    itr->next = NULL;
    return oldHead;
}

int __free_pages(struct Page *free_page) {
    struct Page *itr=NULL;
    int ret = 0;
    tail->next = free_page;
    for(itr=tail; itr->next!=NULL; itr=itr->next);
    tail = itr;
    return ret;
}

/*
 * Function added for unit testing.
 */
void __print_free_page_list(struct Page *page_list_start) {
    struct Page *itr=NULL;
    int cnt = 0;
    kprintf("\n");
    for(itr=page_list_start; itr!=tail; itr=itr->next) {
        kprintf("%x ", (uint64_t)itr);
        cnt++;
    }
    kprintf("\n### No of free pages :%d", cnt+1);
    kprintf("\n");
}

#if 0
//Added for unit testing.
int main() {
   int ret = -1;
   uint64_t start_addr = 0x0;
   uint32_t length = 651264;
   head = NULL;
   struct Page *ptr=NULL, *ptr1=NULL, *ptr2=NULL;
   ret = __create_page_list(start_addr, length);
   if (ret == 0) {
        //printf("Free page list created successfully for range (%" PRIx64 ", %" PRId32 ")", start_addr, length);
        __print_free_page_list(head);
   }

   start_addr = 0x100000;
   length = 409600; 
   ret = __create_page_list(start_addr, length);
   if (ret == 0) {
        printf("Free page list created successfully for range (%" PRIx64 ", %" PRId32 ")", start_addr, length);
        __print_free_page_list(head);
   }
   ptr = __get_pages(5);
   if (ptr) {
        __print_free_page_list(head);
        __print_free_page_list(ptr);
   }
   ptr1 = __get_pages(5);
   if (ptr1) {
        __print_free_page_list(head);
        __print_free_page_list(ptr1);
   }
   ptr2 = __get_pages(3);
   if (ptr2) {
        __print_free_page_list(head);
        __print_free_page_list(ptr2);
   }
   ret = __free_pages(ptr1);
   if (ret == 0) {
        __print_free_page_list(head);
   }
   ret = __free_pages(ptr);
   if (ret == 0) {
        __print_free_page_list(head);
   }
   ret = __free_pages(ptr2);
   if (ret == 0) {
        __print_free_page_list(head);
   }
}
#endif
