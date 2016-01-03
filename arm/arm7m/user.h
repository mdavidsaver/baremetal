#ifndef USER_H
#define USER_H

#include "common.h"

#define SYSCALL(N, NAME, ...) \
    int sys_##NAME (__VA_ARGS__);
#include "syscalls.h"
#undef SYSCALL

#ifndef __KERNEL__
#define halt sys_halt
#define yield sys_yield
#define printk() cant_user_printk_in_user()
#endif

int vprintf(const char *fmt, va_list args) __attribute__((format(printf,1,0)));
int printf(const char *fmt, ...) __attribute__((format(printf,1,2)));
int flush(void);

int msleep(unsigned val);

#endif // USER_H
