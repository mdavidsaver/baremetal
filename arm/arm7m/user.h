#ifndef USER_H
#define USER_H

#define SYSCALL(N, NAME, ...) \
    int sys_##NAME (__VA_ARGS__);
#include "syscalls.h"
#undef SYSCALL

#ifndef __KERNEL__
#define halt sys_halt
#define yield sys_yield
#endif

int vprintf(const char *fmt, va_list args) __attribute__((format(printf,2,0)));
int printf(const char *fmt, ...) __attribute__((format(printf,2,3)));

#endif // USER_H
