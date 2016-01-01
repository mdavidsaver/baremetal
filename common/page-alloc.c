
#include "common.h"

//#define PAGE_DEBUG 1

/* start of physical RAM */
char __ram_start;
/* first byte in RAM after code and static data */
char __after_all_load;

char *start_of_heap = &__after_all_load;

typedef struct page_info {
    struct page_info *next, *prev;
    unsigned int allocd:1;
} page_info;

page_info *page_info_base;
size_t page_info_count, pages_free;
char *page_start;

page_info *first_free;

void *early_alloc(size_t size, unsigned align)
{
    char *ret;
    if(!start_of_heap)
        return NULL; /* page_alloc_setup() already called */
    align--;
    ret = start_of_heap;
    if(align&(size_t)ret) {
        ret = (char*)(1 + (align|(size_t)ret));
    }
    start_of_heap = ret+size;
    memset(ret, 0, size);
    return ret;
}

void early_free(char *ptr, size_t asize)
{
    if(ptr+asize==start_of_heap) {
        /* really free if no other allocates made, less alignment */
        start_of_heap = ptr;
    } else {
        /* lost */
    }
}

void page_alloc_setup(void)
{
    char *startaddr = _PAGE_UP(start_of_heap),
         *endaddr = _PAGE_UP((&__ram_start)+RamSize);
    size_t info_per_page = PAGE_SIZE/sizeof(page_info),
           dynsize = endaddr-startaddr,
           npages = dynsize/PAGE_SIZE,
           ninfos = 0,
           pages_for_info = 0,
           i;

    start_of_heap = NULL; /* disable early_alloc */

    {
        while(ninfos<npages) {
            ninfos += info_per_page;
            npages--;
            pages_for_info++;
        }
    }

#ifdef PAGE_DEBUG
    printk("startaddr %p\n", startaddr);
    printk("endaddr %p\n", endaddr);
    printk("dynsize %u\n", (unsigned)dynsize);
    printk("info_per_page %u\n", (unsigned)info_per_page);
    printk("npages %u\n", (unsigned)npages);
    printk("ninfos %u\n", (unsigned)ninfos);
    printk("pages_for_info %u\n", (unsigned)pages_for_info);
    printk("sizeof(page_info) 0x%x\n", (unsigned)sizeof(page_info));
#endif

    assert(npages>0);
    assert(pages_for_info>0);
    assert(ninfos>=npages);

    page_info_base = (void*)startaddr;
    pages_free = page_info_count = npages;

    startaddr += pages_for_info*PAGE_SIZE;
    page_start = startaddr;

#ifdef PAGE_DEBUG
    printk("page_info_base %p\n", page_info_base);
    printk("page_info_count %u\n", (unsigned)page_info_count);
    printk("page_start %p\n", page_start);
#endif

    page_info_base[0].prev = NULL;
    page_info_base[0].next = &page_info_base[1];
    for(i=1; i<page_info_count-1; i++)
    {
        page_info_base[i].prev = &page_info_base[i-1];
        page_info_base[i].next = &page_info_base[i+1];
    }
    page_info_base[page_info_count-1].next = NULL;
    page_info_base[page_info_count-1].prev = &page_info_base[page_info_count-2];

    first_free = &page_info_base[0];
}


void* page_alloc(void)
{
    page_info *info;
    unsigned mask = irq_mask();

    info = first_free;
    if(info) {
        /* pop from head of list */
        first_free = info->next;
        if(info->next)
            info->next->prev = NULL;
        info->next = info->prev = NULL;
        assert(!info->allocd);
        info->allocd = 1;
        assert(pages_free>0);
        pages_free--;
    }

    irq_restore(mask);

    if(!info) {
#ifdef PAGE_DEBUG
        printk("page_alloc() -> NULL\n");
#endif
        return NULL;
    }

    assert(info>=page_info_base);
    assert((size_t)(info-page_info_base)<page_info_count);
    {
        char *page;
        size_t idx = (info-page_info_base);
        assert(idx<page_info_count);
        page = page_start + idx*PAGE_SIZE;
        memset(page, 0, PAGE_SIZE);
#ifdef PAGE_DEBUG
        printk("page_alloc() -> %p (%u %p)\n", page, (unsigned)idx, info);
#endif
        return page;
    }
}

void page_free(void* addr)
{
    unsigned mask;
    size_t idx;
    page_info *info;

    if(!addr) {
#ifdef PAGE_DEBUG
        printk("page_free(NULL)\n");
#endif
        return;
    }

    if(addr<(void*)page_start) goto badaddr;
    idx = (addr-(void*)page_start)/PAGE_SIZE;
    if(idx>=page_info_count) goto badaddr;

    info = &page_info_base[idx];

#ifdef PAGE_DEBUG
        printk("page_free(%p)  (%u %p)\n", addr, (unsigned)idx, info);
#endif

    mask = irq_mask();
    if(!info->allocd) {
        printk("Already free'd %p\n", addr);
        halt();
    }
    assert(info->allocd);
    info->allocd = 0;
    pages_free++;
    assert(pages_free<=page_info_count);

    info->next = first_free;
    first_free = info;
    if(info->next)
        info->next->prev = info;

    irq_restore(mask);

    return;
badaddr:
    printk("Can't free non-heap %p.  leaking\n", addr);
}

size_t page_free_count(void)
{
    size_t ret;
    unsigned mask;
    mask = irq_mask();
    ret = pages_free;
    irq_restore(mask);
    return ret;
}
