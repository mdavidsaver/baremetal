#ifndef ELL_H
#define ELL_H

typedef struct ELLLIST ELLLIST;

typedef struct ELLNODE {
    struct ELLNODE *prev, *next;
#ifndef NDEBUG
    ELLLIST *list;
#endif
} ELLNODE;

struct ELLLIST {
    ELLNODE head;
};

#ifdef NDEBUG
#define ELLNODE_INIT {0,0}
#else
#define ELLNODE_INIT {0,0,0}
#endif

#define ELLLIST_INIT {ELLNODE_INIT}

#define ellFirst(plist) ((plist)->head.next)
#define ellLast(plist) ((plist)->head.prev)

#define ellNext(pnode) ((pnode)->next)
#define ellPrev(pnode) ((pnode)->prev)

#define ellEmpty(plist) (ellFirst(plist)==NULL)

void ellPushFront(ELLLIST *L, ELLNODE *N);
void ellPushBack(ELLLIST *L, ELLNODE *N);
ELLNODE *ellPopFront(ELLLIST *L);
ELLNODE *ellPopBack(ELLLIST *L);
void ellRemove(ELLLIST *L, ELLNODE *N);

void ellInsertBefore(ELLLIST *L, ELLNODE *P, ELLNODE *N);
void ellInsertAfter(ELLLIST *L, ELLNODE *P, ELLNODE *N);

void ellConcat(ELLLIST *to, ELLLIST *from);

#define container(ptr, structure, member) ({ \
    const __typeof__((((structure*)0))->member) *P = (ptr); \
    (structure*)(((char*)P) - offsetof(structure,member)); \
})

#define foreach_ell(var, plist) \
    for(var = ellFirst(plist); var; var = ellNext(var))

#define foreach_ell_safe(var, save, plist) \
    for(var = ellFirst(plist), save = var ? ellNext(var) : NULL; var; var = save, save = save ? ellNext(save) : NULL)

#define foreach_ell_pop_front(var, plist) \
    while((var = ellPopFront(plist))!=NULL)

#define foreach_ell_pop_back(var, plist) \
    while((var = ellPopBack(plist))!=NULL)

#endif // ELL_H
