
#include "common.h"

//#define PAGE_DEBUG 1

/* start of physical RAM */
char __ram_start;
/* first byte in RAM after code and static data */
char __after_all_load;

typedef struct page_info {
    struct page_info *next, *prev;
    unsigned int allocd:1;
} page_info;

page_info *page_info_base;
size_t page_info_count, pages_free;
char *page_start;

page_info *first_free;

void page_alloc_setup(void)
{
    char *startaddr = _PAGE_UP(&__after_all_load),
         *endaddr = _PAGE_UP((&__ram_start)+RamSize);
    size_t info_per_page = PAGE_SIZE/sizeof(page_info),
           dynsize = endaddr-startaddr,
           npages = dynsize/PAGE_SIZE,
           ninfos = 0,
           pages_for_info = 0,
           i;

    {
        while(ninfos<npages) {
            ninfos += info_per_page;
            npages--;
            pages_for_info++;
        }
    }

#ifdef PAGE_DEBUG
    printk(0, "startaddr %p\n", startaddr);
    printk(0, "endaddr %p\n", endaddr);
    printk(0, "dynsize %u\n", (unsigned)dynsize);
    printk(0, "info_per_page %u\n", (unsigned)info_per_page);
    printk(0, "npages %u\n", (unsigned)npages);
    printk(0, "ninfos %u\n", (unsigned)ninfos);
    printk(0, "pages_for_info %u\n", (unsigned)pages_for_info);
#endif

    assert(npages>0);
    assert(pages_for_info>0);
    assert(ninfos>=npages);

    page_info_base = (void*)startaddr;
    pages_free = page_info_count = npages;

    startaddr += pages_for_info*PAGE_SIZE;
    page_start = startaddr;

#ifdef PAGE_DEBUG
    printk(0, "page_info_base %p\n", page_info_base);
    printk(0, "page_info_count %u\n", (unsigned)page_info_count);
    printk(0, "page_start %p\n", page_start);
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
        first_free = info->next;
        if(info->next)
            info->next->prev = NULL;
        info->next = info->prev = NULL;
        assert(!info->allocd);
        info->allocd = 1;
        assert(pages_free>0);
        pages_free--;
    }

    irq_unmask(mask);

    if(!info) return NULL;

    assert(info>=page_info_base);
    assert((size_t)(info-page_info_base)<page_info_count);
    {
        char *page;
        size_t idx = (info-page_info_base)/sizeof(*info);
        assert(idx<page_info_count);
        page = page_start + idx*PAGE_SIZE;
        memset(page, 0, PAGE_SIZE);
        return page;
    }
}

void page_free(void* addr)
{
    unsigned mask;
    size_t idx;
    page_info *info;

    if(!addr) return;

    if(addr<(void*)page_start) goto badaddr;
    idx = (addr-(void*)page_start)/PAGE_SIZE;
    if(idx>=page_info_count) goto badaddr;

    info = &page_info_base[idx];

    mask = irq_mask();
    assert(info->allocd);
    info->allocd = 0;
    pages_free++;
    assert(pages_free<=page_info_count);

    info->next = first_free;
    first_free = info;
    if(info->next)
        info->next->prev = info;

    irq_unmask(mask);

    return;
badaddr:
    printk(0, "Can't free non-heap %p.  leaking\n", addr);
}

size_t page_free_count(void)
{
    size_t ret;
    unsigned mask;
    mask = irq_mask();
    ret = pages_free;
    irq_unmask(mask);
    return ret;
}
