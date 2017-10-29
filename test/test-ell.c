
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ell.h"

static
void testPFPF(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    assert(ellPopFront(&list)==NULL);
    assert(ellPopBack(&list)==NULL);
    ellPushFront(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
#endif

    assert(ellPopFront(&list)==&node);

    assert(list.head.next==NULL);
    assert(list.head.prev==NULL);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
#endif
}

static
void testPFPB(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushFront(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
#endif

    assert(ellPopBack(&list)==&node);

    assert(list.head.next==NULL);
    assert(list.head.prev==NULL);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
#endif
}

static
void testPBPF(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    assert(ellPopFront(&list)==NULL);
    ellPushBack(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
#endif

    assert(ellPopFront(&list)==&node);

    assert(list.head.next==NULL);
    assert(list.head.prev==NULL);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
#endif
}

static
void testPBPB(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
#endif

    assert(ellPopBack(&list)==&node);

    assert(list.head.next==NULL);
    assert(list.head.prev==NULL);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
#endif
}


static
void testPF2PF(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLNODE node2 = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushFront(&list, &node2);
    ellPushFront(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node2);
    assert(node.next==&node2);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==&node);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==&list);
#endif

    assert(ellPopFront(&list)==&node);

    assert(list.head.next==&node2);
    assert(list.head.prev==&node2);
    assert(node.next==NULL);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
    assert(node2.list==&list);
#endif
}

static
void testPB2PB(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLNODE node2 = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);
    ellPushBack(&list, &node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node2);
    assert(node.next==&node2);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==&node);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==&list);
#endif

    assert(ellPopBack(&list)==&node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==NULL);
#endif
}

static
void testRemove1(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node.next==NULL);
    assert(node.prev==NULL);

    ellRemove(&list, &node);

    assert(list.head.next==NULL);
    assert(list.head.prev==NULL);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
#endif
}

static
void testRemoveHead(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLNODE node2 = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);
    ellPushBack(&list, &node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node2);
    assert(node.next==&node2);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==&node);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==&list);
#endif

    ellRemove(&list, &node);

    assert(list.head.next==&node2);
    assert(list.head.prev==&node2);
    assert(node.next==NULL);
    assert(node.prev==NULL);
#ifndef NDEBUG
    assert(node.list==NULL);
    assert(node2.list==&list);
#endif
}

static
void testRemoveTail(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLNODE node2 = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);
    ellPushBack(&list, &node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node2);
    assert(node.next==&node2);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==&node);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==&list);
#endif

    ellRemove(&list, &node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node);
    assert(node2.next==NULL);
    assert(node2.prev==NULL);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==NULL);
#endif
}

static
void testRemoveMiddle(void)
{
    ELLNODE node = ELLNODE_INIT;
    ELLNODE node2 = ELLNODE_INIT;
    ELLNODE node3 = ELLNODE_INIT;
    ELLLIST list = ELLLIST_INIT;

    ellPushBack(&list, &node);
    ellPushBack(&list, &node2);
    ellPushBack(&list, &node3);

    assert(list.head.next==&node);
    assert(list.head.prev==&node3);
    assert(node.next==&node2);
    assert(node.prev==NULL);
    assert(node2.next==&node3);
    assert(node2.prev==&node);
    assert(node3.next==NULL);
    assert(node3.prev==&node2);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==&list);
    assert(node3.list==&list);
#endif

    ellRemove(&list, &node2);

    assert(list.head.next==&node);
    assert(list.head.prev==&node3);
    assert(node.next==&node3);
    assert(node.prev==NULL);
    assert(node2.next==NULL);
    assert(node2.prev==NULL);
    assert(node3.next==NULL);
    assert(node3.prev==&node);
#ifndef NDEBUG
    assert(node.list==&list);
    assert(node2.list==NULL);
    assert(node3.list==&list);
#endif
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    testPFPF();
    testPFPB();
    testPBPF();
    testPBPB();
    testPF2PF();
    testPF2PF();
    testPB2PB();
    testRemove1();
    testRemoveHead();
    testRemoveTail();
    testRemoveMiddle();
    printf("Done\n");
    return 0;
}
