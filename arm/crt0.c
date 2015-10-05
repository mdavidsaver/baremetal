
#include <stddef.h>

#include "common.h"

/* Due to link ordering (crt0 is last)
 * must declare weak symbols for almost any
 * used here
 */

extern char __TMC_END__; /* implicit from linker? */

/* see linker script */
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));
extern void (*__fini_array_start []) (void) __attribute__((weak));
extern void (*__fini_array_end []) (void) __attribute__((weak));

void abort(void) __attribute__((weak));
void puts(const char *str) __attribute__((weak));

typedef struct {
    void (*fn)(void*);
    void *arg;
} atexit_handle;

static atexit_handle global_dtors[8];

void __run_init(void)
{
    size_t i, N;

    N = __preinit_array_end-__preinit_array_start;
    for (i=0; i<N; i++)
        __preinit_array_start[i]();

    N = __init_array_end-__init_array_start;
    for (i=0; i<N; i++)
        __init_array_start[i]();
}

void __run_fini(void)
{
    size_t i, N;

    for(i=0; i<NELEM(global_dtors); i++)
    {
        if(global_dtors[i].fn)
            (*global_dtors[i].fn)(global_dtors[i].arg);
    }

    N = __fini_array_end-__fini_array_start;
    for (i=0; i<N; i++)
        __fini_array_start[i]();
}

/* G++ places calls to this function into the global ctor
 * wrapper functions found in the .init_array section.
 * This this function will be called from within __run_init()
 */
int __aeabi_atexit(void *obj, void (*dtor)(void*), void *handle)
{
    unsigned i;
    /* since we don't support dynamic linking or DSOs there
     * will only ever be one handle
     */
    if(handle!=&__TMC_END__) {
        puts("__aeabi_atexit w/ invalid handle\n");
        abort();
        return -1;
    }
    for(i=0; i<NELEM(global_dtors); i++)
    {
        if(global_dtors[i].fn)
            continue;
        global_dtors[i].fn = dtor;
        global_dtors[i].arg = obj;
        return 0;
    }
    puts("Too many c++ global ctors\n");
    return -1;
}
