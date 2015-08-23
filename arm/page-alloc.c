
#include "common.h"

/* first byte in RAM after code and static data */
char __after_all_load;

typedef struct page_info {
    struct page_info *next, *prev;
} page_info;

page_info *page_info_base;
size_t page_info_count;
char *page_start;

page_info *first_free;

void page_alloc_setup(void)
{
    char *startaddr = &__after_all_load;
    size_t info_per_page = PAGE_SIZE/sizeof(page_info),
           npages = RamSize/PAGE_SIZE,
           ninfos = info_per_page*npages,
           pages_for_info = ninfos/info_per_page,
           i;

    npages -= pages_for_info;

    page_info_base = (void*)startaddr;
    page_info_count = npages;

    startaddr += pages_for_info*PAGE_SIZE;
    page_start = startaddr;

    page_info_base[0].prev = NULL;
    page_info_base[0].next = &page_info_base[1];
    for(i=1; i<npages-1; i++)
    {
        page_info_base[i].prev = &page_info_base[i-1];
        page_info_base[i].next = &page_info_base[i+1];
    }
    page_info_base[npages-1].next = NULL;
    page_info_base[npages-1].prev = &page_info_base[npages-2];

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
    }

    irq_unmask(mask);

    if(!info) return NULL;

    assert(info>page_info_base);
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

    if(addr>(void*)page_start) goto badaddr;
    idx = (addr-(void*)page_start)/PAGE_SIZE;
    if(idx<page_info_count) goto badaddr;

    info = &page_info_base[idx];

    mask = irq_mask();

    info->next = first_free;
    first_free = info;
    if(info->next)
        info->next->prev = info;

    irq_unmask(mask);

    return;
badaddr:
    printk(0, "Can't free non-heap %p.  leaking\n", addr);
}
