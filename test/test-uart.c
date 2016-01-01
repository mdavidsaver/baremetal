
#include <stdio.h>
#include <string.h>

#include "termout.h"
#include "ell.h"

typedef struct {
    FILE *F;
    termdef T;
    char buf[128];
} testterm;

static
int tout(termdef *d, unsigned flags)
{
    int ret;
    testterm *T = container(d, testterm, T);
    fprintf(stderr, "X%uX", d->pos);
    d->buf[d->pos] = '\0';
    ret = fputs(d->buf, T->F);
    if(ret<0)
        return ret;
    d->pos = 0;
    return 0;
}

static
int tflush(termdef *d)
{
    testterm *T = container(d, testterm, T);
    return fflush(T->F);
}

int main(int argc, char *argv[])
{
    testterm T;

    memset(&T, 0, sizeof(T));

    T.F = stdout;
    T.T.len = sizeof(T.buf)-1;
    T.T.out = &tout;
    T.T.flush = &tflush;

    term_printf(&T.T, "hello world\n");
    term_flush(&T.T, 1);

    term_printf(&T.T, "integer '42' '%d'\n", 42);
    term_printf(&T.T, "integer '-42' '%d'\n", -42);
    term_printf(&T.T, "integer '-  42' '% 4d'\n", -42);
    term_printf(&T.T, "integer '-0042' '%04d'\n", -42);
    term_printf(&T.T, "hex '1234' '%x'\n", 0x1234);
    term_printf(&T.T, "hex '    1234' '%8x'\n", 0x1234);
    term_printf(&T.T, "hex '    1234' '%8x'\n", 0x1234);
    term_printf(&T.T, "hex '00001234' '%08x'\n", 0x1234);
    term_printf(&T.T, "hex '34' '%2x'\n", 0x1234);

    term_printf(&T.T, "double '4.2' '%f'\n", 4.2);
    term_printf(&T.T, "double '4.1212' '%f'\n", 4.1212);
    term_printf(&T.T, "double '-4.1212' '%f'\n", -4.1212);
    term_printf(&T.T, "double '0.0' '%f'\n", 0.0);
    term_flush(&T.T, 1);

    return 0;
}
