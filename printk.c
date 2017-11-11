
#ifndef HOST_BUILD
#  include "common.h"
#else
#  include <stdint.h>
#  include <stdarg.h>

void putc_test(char c);
void puts_test(const char *s);
#define putc putc_test
#define puts puts_test
#endif

const char hexchars[] = "0123456789ABCDEF";

typedef struct format {
    unsigned char width;
    char fill;
    char modifier;
    char type;
} format;
#define FORMAT_INIT {8, '\0', '\0', '\0'}

static
void show_unsigned(format *info, unsigned long num)
{
    char buf[9], *cur = &buf[8];
    unsigned base = info->type=='x' ? 16 : 10;

    if(!num) {
        *--cur = '0';
        info->width--;
    }
    for(;info->width && num; info->width--) {
        *--cur = hexchars[num%base];
        num = num/base;
    }
    if(info->fill) {
        while(info->width--) {
            *--cur = info->fill;
        }
    }
    buf[8] = '\0';
    puts(cur);
}

static
void show_signed(format *info, long num)
{
    if(num<0) {
        putc('-');
        num = -num;
    }
    show_unsigned(info, num);
}

void vprintk(const char *fmt, va_list args)
{
    char c;
    while((c=*fmt++)!='\0') {
        switch(c) {
        default:
            putc(c);
            break;
        case '%': {
            format info = FORMAT_INIT;
            while(!info.type && (c=*fmt++)!='\0') {
                switch(c) {
                case '%':
                    putc(c);
                    info.type = c;
                    break;
                case ' ':
                case '0':
                    info.fill = c;
                    break;
                case '1' ... '8':
                    info.width = c - '0'; // don't support widths longer than 8
                    break;
                case 'l':
                case 'z':
                    if(info.modifier)
                        goto oops; // don't support long long modifer
                    info.modifier = c;
                    break;
                case 'x':
                case 'u': {
                    info.type = c;
                    unsigned long num;
                    if(info.modifier=='\0') {
                        num = va_arg(args, unsigned);
                    } else if(info.modifier=='l') {
                        num = va_arg(args, unsigned long);
                    } else {
                        goto oops;
                    }
                    show_unsigned(&info, num);
                }
                    break;
                case 'd': {
                    info.type = c;
                    long num;
                    if(info.modifier=='\0') {
                        num = va_arg(args, int);
                    } else if(info.modifier=='l') {
                        num = va_arg(args, long);
                    } else {
                        goto oops;
                    }
                    show_signed(&info, num);
                }
                    break;
                case 's': {
                    info.type = c;
                    const char * v = va_arg(args, const char*);
                    puts(v);
                }
                    break;
                case 'c': {
                    info.type = c;
                    char num = va_arg(args, int);
                    putc(num);
                }
                    break;
                case '9':
                case '\0':
                default:
                    goto oops;
                    return;
                }
            }
        }
            break;
        }
    }
    return;

oops:
    putc('!');
}

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
