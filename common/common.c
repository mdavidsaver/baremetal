
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

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}

void vprintk(const char *fmt, va_list args)
{
    char c;
    while ( (c=*fmt++)!='\0' ) {
        if(c!='%') {
            putchar(c);
        } else {
            int done=0;
            while(!done) {
                c = *fmt++;

                switch(c) {
                case '%': putchar(c); done=1; break;
                case 'x': {
                    unsigned int V = va_arg(args, unsigned int);
                    putval(V);
                    done = 1;
                    break;
                }
                case 'p': {
                    const void *V = va_arg(args, void*);
                    puts("0x");
                    putval((unsigned)V);
                    done = 1;
                    break;
                }
                case 's': {
                    const char *V = va_arg(args, void*);
                    puts(V ? V : "(null)");
                    done = 1;
                    break;
                }
                default:
                    putchar('?');
                    done = 1;
                    break;
                }
            }
        }
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
