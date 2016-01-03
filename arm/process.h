#ifndef PROCESS_H
#define PROCESS_H

#include "cpu.h"

#ifdef ARM7M
/* how registers are stored when a thread is not active */

/* the following are automatically stacked by the CPU on exception entry */
#define FRAME_R0 0
#define FRAME_R1 1
#define FRAME_R2 2
#define FRAME_R3 3
#define FRAME_R12 4
/* SP==R13 LR==R14, PC==R15 */
#define FRAME_LR 5
#define FRAME_PC 6
#define FRAME_PSR 7
/* additional registers we stack when we decide to context switch.
 * These are written to the stack *without* updating SP
 */
#define FRAME_R4 -8
#define FRAME_R5 -7
#define FRAME_R6 -6
#define FRAME_R7 -5
#define FRAME_R8 -4
#define FRAME_R9 -3
#define FRAME_R10 -2
#define FRAME_R11 -1
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>

#include "ell.h"
#include "systick.h"

typedef struct process process;
typedef struct process_config process_config;
typedef struct thread thread;
typedef struct thread_config thread_config;

struct process_config {
    char *memory_start, *memory_end;
    char *data_start, *data_end, *data_load;
    char *bss_start, *bss_end;
    char *preinit_array_start, *preinit_array_end;
    char *init_array_start, *init_array_end;
    char *fini_array_start, *fini_array_end;
    thread *threads;
    unsigned int super:1;
    const char *name;
};

struct process {
    const process_config * const info;
    unsigned int initialized;

#ifdef ARM7M
    uint32_t mpu_settings[2*MPU_USER_REGIONS];
#endif
};

struct thread_config {
    uint8_t prio;
    int (*entry)(const char*);
    void *stack;
    uint32_t stack_size;
    unsigned int autostart:1;
    const char *name;
};

struct thread {
    const thread_config * const info;
    process * const proc;

    ELLNODE schednode;

    unsigned holdcount;
    unsigned active:1;

    char *frame;

    systick_cb waiter;
    uint32_t timeout_ticks;
};

extern thread *thread_scheduler[2]; /* {current, next} */

#define thread_scheduler ((thread* volatile*)thread_scheduler)

void thread_resume(thread*);
void thread_suspend(thread*);
void thread_yield(thread*);

#endif /*  __ASSEMBLER__ */

#endif /* PROCESS_H */
