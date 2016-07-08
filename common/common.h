#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

void memcpy(void *dst, const void *src, size_t count);
void memset(void *dst, int val, size_t count);
int strcmp(const char *A, const char *B);

void putchar(char c);
void puts(const char* msg);
void putval(uint32_t v);

unsigned testcnt;
void testInit(unsigned ntest);
void testeq32(uint32_t expect, uint32_t actual);

#endif /* COMMON_H */
