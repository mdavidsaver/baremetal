#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdarg.h>

void memset(void *addr, int val, uint32_t cnt);

void printk(const char *fmt, ...) __attribute__((format(__printf__,1,2)));
void vprintk(const char *fmt, va_list) __attribute__((format(__printf__,1,0)));

#endif /* COMMON_H */
