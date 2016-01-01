#ifndef USER_H
#define USER_H

#define SYSCALL(N, NAME, ...) \
    int sys_##NAME (__VA_ARGS__);
#include "syscalls.h"
#undef SYSCALL

#endif // USER_H
