#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

#define SYSTIMER_RATE 1000000 /* Hz */
#define SYSTICK_RATE 10 /* Hz */

typedef struct systick_cb {
    /* internal */
    struct systick_cb *next, *prev;
    /* user */
    void (*cb)(struct systick_cb*);
} systick_cb;

void systick_setup(void);

uint32_t systick_get(void);
uint32_t systimer_get(void);

int systick_add(struct systick_cb*);
int systick_del(struct systick_cb*);

void systimer_spin(uint32_t wait);

#endif // SYSTICK_H
