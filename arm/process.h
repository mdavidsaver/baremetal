#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef struct {
    char *stack_bottom, *stack_top;
    char *data_start, *data_end, *data_load;
    char *bss_start, *bss_end;
    char *preinit_array_start, *preinit_array_end;
    char *init_array_start, *init_array_end;
    char *fini_array_start, *fini_array_end;
    unsigned int super:1;
    const char name[];
} process_config;

typedef struct {
    const process_config *info;
    
    ELLLIST threads;
    
#ifdef ARM7M
    uint32_t mpu_settings[2*MPU_USER_REGIONS];
#endif
} process;

typedef struct {
    ELLNODE procnode;
    process *proc;

    ELLNODE schednode;
    uint8_t prio;

    unsigned holdcount;

    void *SP;
} thread;

extern volatile thread *current_thread;

#endif /* PROCESS_H */
