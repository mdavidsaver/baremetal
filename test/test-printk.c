
#include <assert.h>
#include <stdio.h>
#include <string.h>

void printk(const char *fmt, ...) __attribute__((format(__printf__,1,2)));
void vprintk(const char *fmt, va_list) __attribute__((format(__printf__,1,0)));

void putc_test(char c)
{
    putc(c, stdout);
    fflush(stdout);
}

void puts_test(const char *s)
{
    printf("%s", s);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("====\n");
    printk("Text only\n");
    printk("String \"hello world\" \"%s\"\n", "hello world");
    printk("literal %% percent\n");

    printf("====\n");
    printk("unsigned 0 = '%u'\n", 0u);
    printk("unsigned 123 = '%u'\n", 123u);
    printk("unsigned long 123 = '%lu'\n", 123ul);
    printk("pad unsigned 0123 = '%04u'\n", 123u);
    printk("pad unsigned 00123 = '%05u'\n", 123u);

    printf("====\n");
    printk("hex 123 = '%x'\n", 0x123);
    printk("hex long 123 = '%lx'\n", 0x123l);
    printk("hex pad 0123 = '%04x'\n", 0x123);
    printk("hex pad 00123 = '%05x'\n", 0x123);

    printf("====\n");
    printk("decimal 123 = '%d'\n", 123);
    printk("decimal -123 = '%d'\n", -123);

    return 0;
}
