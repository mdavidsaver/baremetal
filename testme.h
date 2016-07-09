#ifndef TESTME_H
#define TESTME_H

#include <stdint.h>

void testInit(unsigned ntests);
void testOk(int c, const char *msg);
void testPass(const char *msg);
void testFail(const char *msg);
void testDiag(const char *msg);

void testEqI(uint32_t expect, uint32_t actual, const char *msg);

#endif /* TESTME_H */
