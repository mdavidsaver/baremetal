/* Macros for ARM v7-A and -R */
#ifndef ARM_H
#define ARM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ARM_MODE_USR 16
#define ARM_MODE_FIQ 17
#define ARM_MODE_IRQ 18
#define ARM_MODE_SVC 19
#define ARM_MODE_ABT 23
#define ARM_MODE_UND 27
#define ARM_MODE_SYS 31
#define ARM_MODE_MASK 0x1f

/* from atag.c */
int processATAG(uint32_t*);
extern uint32_t board_id;
extern const char *cmd_line;

#ifdef __cplusplus
}
#endif

#endif // ARM_H
