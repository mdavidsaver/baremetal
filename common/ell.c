#include "common.h"
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
    }
    if(!L->head.prev)
        L->head.prev = N;
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
    }
    if(!L->head.next)
        L->head.next = N;
}

ELLNODE *ellPopFront(ELLLIST *L)
{
    ELLNODE *ret = L->head.next;
    if(ret) {
        assert(ret->list==L);
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
        L->head.prev = NULL;
    }
    if(N->prev) {
        N->prev->next = N->next;
    } else {
        assert(L->head.next==N);
        L->head.next = NULL;
    }
    N->prev = N->next = NULL;
#ifndef NDEBUG
    N->list = NULL;
#endif
}
