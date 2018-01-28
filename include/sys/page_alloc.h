#ifndef _PAGE_ALLOC_H
#define _PAGE_ALLOC_H

#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/paging.h>
#define PAGE_SIZE   4096
#define KERNBASE 0xffffffff80000000

struct Page {
    struct Page *next;
};

void init_phys_mem(uint64_t physbase, uint64_t physfree, uint64_t start, uint64_t end);
struct Page * __alloc_page(uint64_t addr);
int __create_page_list(uint64_t physfree, int npages);
struct Page *__get_pages(uint32_t npages);
int __free_pages(struct Page *free_page);
void __print_free_page_list(struct Page *page_list_start);
uint64_t __get_phys_addr(uint64_t virtaddr);

#endif
