
#include <stddef.h>

#ifndef HOST_BUILD
#  include "common.h"
#define assert(E) do{}while(0)
#else
#  include <assert.h>
#endif
#include "ell.h"

void ellPushFront(ELLLIST *L, ELLNODE *N)
{
    assert(!N->list);
    N->prev = NULL;
    N->next = L->head.next;
#ifndef NDEBUG
    N->list = L;
#endif
    L->head.next = N;
    if(N->next) {
        N->next->prev = N;
    } else {
        assert(!L->head.prev);
        L->head.prev = N;
    }
}

void ellPushBack(ELLLIST *L, ELLNODE *N)
{
    assert(!N->list);
    N->prev = L->head.prev;
    N->next = NULL;
#ifndef NDEBUG
    N->list = L;
#endif
    L->head.prev = N;
    if(N->prev) {
        N->prev->next = N;
    } else {
        assert(!L->head.next);
        L->head.next = N;
    }
}

ELLNODE *ellPopFront(ELLLIST *L)
{
    ELLNODE *ret = L->head.next;
    if(ret) {
        assert(ret->list==L);
        assert(ret->prev==NULL);
        L->head.next = ret->next;
        if(ret->next) {
            ret->next->prev = NULL;
        } else {
            assert(L->head.prev==ret);
            L->head.prev = NULL;
        }
        ret->next = ret->prev = NULL;
#ifndef NDEBUG
        ret->list = NULL;
#endif
    }
    return ret;
}

ELLNODE *ellPopBack(ELLLIST *L)
{
    ELLNODE *ret = L->head.prev;
    if(ret) {
        assert(ret->list==L);
        assert(ret->next==NULL);
        L->head.prev = ret->prev;
        if(ret->prev) {
            ret->prev->next = NULL;
        } else {
            assert(L->head.next==ret);
            L->head.next = NULL;
        }
        ret->next = ret->prev = NULL;
#ifndef NDEBUG
        ret->list = NULL;
#endif
    }
    return ret;
}

void ellRemove(ELLLIST *L, ELLNODE *N)
{
    assert(N->list==L);
    if(N->next) {
        N->next->prev = N->prev;
    } else {
        assert(L->head.prev==N);
        L->head.prev = N->prev;
    }
    if(N->prev) {
        N->prev->next = N->next;
    } else {
        assert(L->head.next==N);
        L->head.next = N->next;
    }
    N->prev = N->next = NULL;
#ifndef NDEBUG
    N->list = NULL;
#endif
}

void ellConcat(ELLLIST *to, ELLLIST *from)
{
    if(!from->head.next) { /* from is empty */
        assert(!from->head.prev);

    } else if(!to->head.prev) { /* to is empty */
        assert(!to->head.next);
        to->head = from->head;
        from->head.next = from->head.prev = NULL;

    } else {
        /* both not empty */
        to->head.prev->next = from->head.next;
        from->head.next->prev = to->head.prev;
        to->head.prev = from->head.prev;

        from->head.next = from->head.prev = NULL;
    }
}

// insert N before P
// P==NULL appends
void ellInsertBefore(ELLLIST *L, ELLNODE *P, ELLNODE *N)
{
    if(P==NULL) {
        /* append to tail */
        N->prev = L->head.prev;
        N->next = NULL;
        L->head.prev = N;
    } else {
        N->prev = P->prev;
        N->next = P;
        P->prev = N;
    }
    
    if(N->prev==NULL) {
        L->head.next = N;
    } else {
        N->prev->next = N;
    }
}

// insert N after P
// P==NULL prepends
void ellInsertAfter(ELLLIST *L, ELLNODE *P, ELLNODE *N)
{
    if(P==NULL) {
        /* prepend to head */
        N->prev = NULL;
        N->next = L->head.next;
        L->head.next = N;
    } else {
        N->prev = P;
        N->next = P->next;
        P->next = N;
    }
    
    if(N->next==NULL) {
        L->head.prev = N;
    } else {
        N->next->prev = N;
    }
}
