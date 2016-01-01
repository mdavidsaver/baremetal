/* define SYSCALL(N, NAME, arg list...) */
SYSCALL(0, halt)
SYSCALL(1, yield)
SYSCALL(2, uart, const char *buf, unsigned flags)
