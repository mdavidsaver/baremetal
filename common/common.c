
#include "common.h"

void memcpy(void *dst, const void *src, size_t count)
{
  char *cdst = dst;
  const char *csrc = src;
  while(count--) *cdst++=*csrc++;
}

void memset(void *dst, int val, size_t count)
{
  char *cdst = dst;
  while(count--) *cdst++=(char)val;
}

int strcmp(const char *A, const char *B)
{
    while(1) {
        char a=*A++, b=*B++;
        if(a<b) return -1;
        else if(a>b) return 1;
        /* a==b */
        else if(a=='\0') return 0;
    }
}

void puts(const char* msg)
{
    char c;
    while( (c=*msg++) )
    {
        putchar(c);
    }
}

void nputs(const char* msg, size_t n)
{
    while(n--) putchar(*msg++);
}

void putval(uint32_t v)
{
    static char hex[] = "0123456789ABCDEF";
    uint8_t n = sizeof(v)*2;

    while(n--) {
        putchar(hex[v>>28]);
        v<<=4;
    }
}

unsigned testcnt;

void testInit(unsigned ntest)
{
    testcnt=0;
    puts("1..");
    putval(ntest);
    puts("\r\n");
}

void testeq32(uint32_t expect, uint32_t actual)
{
  if(expect!=actual)  puts("not ");
  puts("ok ");
  putval(++testcnt);
  puts(" - ");
  putval(expect);
  puts(" == ");
  putval(actual);
  puts("\r\n");
}
