#ifndef TESTME_H
#define TESTME_H

#include <stdint.h>
#include <stdarg.h>

void testInit(unsigned ntests);
void testOk(int c, const char *msg, ...) __attribute__((format(__printf__,2,3)));
void testVOk(int c, const char *msg, va_list) __attribute__((format(__printf__,2,0)));
void testPass(const char *msg, ...) __attribute__((format(__printf__,1,2)));
void testFail(const char *msg, ...) __attribute__((format(__printf__,1,2)));
void testDiag(const char *msg, ...) __attribute__((format(__printf__,1,2)));

void testEqI(uint32_t expect, uint32_t actual, const char *msg, ...) __attribute__((format(__printf__,3,4)));

#endif /* TESTME_H */
