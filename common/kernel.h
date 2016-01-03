#ifndef KERNEL_H
#define KERNEL_H

#ifndef __KERNEL__
# define __KERNEL__ 1
# ifdef __USER__
#  error When including both kernel.h and user.h, kernel.h must appear first
# endif
#endif

#include "common.h"


/* Shutdown the system from init.S */
void halt(void);

/* from page-alloc.c */
void page_alloc_setup(void);

/* from printk.c */
int vprintk(const char *fmt, va_list args) __attribute__((format(printf,1,0)));
int printk(const char *fmt, ...) __attribute__((format(printf,1,2)));

/* from irq.c */

typedef void(*isrfunc)(void);

void irq_setup(void);
void irq_show(void);
int isr_install(unsigned vect, isrfunc fn);
int isr_enable(unsigned vect);
int isr_disable(unsigned vect);
int isr_active(void);

unsigned irq_mask(void);
void irq_restore(unsigned m);

/* from page-alloc.c */
void *early_alloc(size_t size, unsigned flags);
void early_free(char *ptr, size_t asize);
void* page_alloc(void);
void page_free(void* addr);


#endif // KERNEL_H
