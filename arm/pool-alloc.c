
#include "common.h"

typedef struct mempool mempool;

#define BLOCKS_PER_PAGE 64

struct mempool
{
    unsigned allocsize;
    mempool *nextpage;
    unsigned nblocks;
    uint32_t blockused[BLOCKS_PER_PAGE/32];
};

static
void mempool_block_init(mempool *pool)
{
    unsigned i;
    /* mark extra bits as used */
    for(i=pool->nblocks; i<BLOCKS_PER_PAGE; i++)
    {
        pool->blockused[i/32] |= (1<<(i%32));
    }
}

mempool *mempool_create(unsigned allocsize)
{
    mempool *ret = NULL;
    const unsigned minalloc = PAGE_SIZE/(8*sizeof(ret->blockused));

    if(allocsize>=PAGE_SIZE/2) return ret;
    else if(allocsize<minalloc) allocsize=minalloc;

    /* round up to multiple of 8 */
    allocsize = 1+((allocsize-1)|7);

    ret = page_alloc();

    if(!ret) return ret;

    ret->allocsize = allocsize;
    ret->nblocks = (PAGE_SIZE-sizeof(*ret))/allocsize;

    if(ret->nblocks<2) {
        page_free(ret);
        return NULL;
    }

    mempool_block_init(ret);

    return ret;
}

void mempool_destroy(mempool *pool)
{
    mempool *orig = pool;
    unsigned warn = 0;
    mempool *next;
    for(next=pool?pool->nextpage:NULL ;
        pool;
        pool=next, next=next?next->nextpage:NULL)
    {
        unsigned i;
        unsigned skip = 0;
        for(i=0; i<NELEM(pool->blockused); i++)
        {
            if(pool->blockused[i])
            {
                if(!warn) {
                    printk(0, "mempool %p has allocations when destoryed, leaking\n", orig);
                    warn = 1;
                }
                skip = 1;
                continue;
            }
        }
        if(!skip)
            page_free(pool);
    }
}

void* mempool_zmalloc(mempool *pool)
{
    mempool *nextpage;
    for(;pool; pool=pool->nextpage)
    {
        unsigned i;
        for(i=0; i<NELEM(pool->blockused); i++)
        {
            unsigned j;
            uint32_t temp;

            if(pool->blockused[i]==0xffffffff)
                continue;

            for(j=0, temp=pool->blockused[i]; j<32; j++, temp>>=1)
            {
                if(!(temp&1))
                {
                    char *buf = (char*)(pool+1);
                    pool->blockused[i] |= (1<<j);
                    buf += pool->allocsize*i*j;
                    assert(buf<PAGE_SIZE+(char*)pool);
                    assert(pool->allocsize+buf<PAGE_SIZE+(char*)pool);
                    memset(buf, 0, pool->allocsize);
                    return buf;
                }
            }
        }
    }
    nextpage = mempool_create(pool->allocsize);
    if(nextpage){
        char *buf;

        pool->nextpage = nextpage;

        nextpage->blockused[0] |= 1;
        buf = (char*)(pool+1);
        memset(buf, 0, pool->allocsize);
        return buf;
    }
    return NULL;
}

void mempool_free(mempool *pool, void *buf)
{
    char *cbuf = buf;
    mempool *prev;
    for(prev=pool; pool; prev=pool, pool=pool->nextpage)
    {
        unsigned idx;
        char *base = (char*)(pool+1);
        if(cbuf<base || (cbuf-base)>=PAGE_SIZE)
            continue;

        idx = (cbuf-base)/pool->allocsize;
        assert(idx<BLOCKS_PER_PAGE);
        assert(idx<pool->nblocks);

        pool->blockused[idx/32] &= ~(1<<(idx%32));

        if(pool->blockused[idx/32]==0)
        {
            unsigned i;
            uint32_t nonzero = 0;
            for(i=0; i<NELEM(pool->blockused); i++)
                nonzero |= pool->blockused[i];
            if(nonzero==0 && prev!=pool) {
                /* page is empty, free if not the first */
                prev->nextpage = pool->nextpage;
                page_free(pool);
            }
        }

        return;
    }

    printk(0, "mempool %p does not contain %p, leaking\n", pool, buf);
}
