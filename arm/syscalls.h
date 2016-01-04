/* define SYSCALL(N, NAME, arg list...) */
SYSCALL(0, halt)
SYSCALL(1, yield)
SYSCALL(2, uart, const char *buf, unsigned flags)
SYSCALL(3, sleep, uint32_t val, uint32_t flags)
SYSCALL(4, spawn, const char *name, unsigned flags)
