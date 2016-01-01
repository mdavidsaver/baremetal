#ifndef TERMOUT_H
#define TERMOUT_H

#include <stddef.h>
#include <stdarg.h>

typedef struct termdef termdef;

typedef int (*term_out_fn)(termdef *d, unsigned flags);
typedef int (*term_in_fn)(termdef *d, char *buf, unsigned blen, unsigned flags);
typedef int (*term_flush_fn)(termdef *d);

struct termdef {
    term_out_fn out;
    term_in_fn in;
    term_flush_fn flush;
    unsigned pos, len;
    char buf[1];
};

int term_puts(termdef *t, const char *s);
int term_putc(termdef *t, char c);
int term_vprintf(termdef *t, const char *fmt, va_list args) __attribute__((format(printf,2,0)));
int term_printf(termdef *t, const char *fmt, ...) __attribute__((format(printf,2,3)));
int term_flush(termdef *t, unsigned hw);

#endif // TERMOUT_H
