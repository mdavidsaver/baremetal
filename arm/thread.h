#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread thread;

typedef unsigned thread_id;
#define INVALID_THREAD ((thread_id)-1)

typedef void*(*thread_func)(void*);

typedef struct {
    unsigned prio;
    unsigned int user:1;
    unsigned int joinable:1;
} thread_options;

void thread_setup(void);

thread_id thread_create(thread_options* opt,
                        thread_func fn,
                        void *user);

/* mark thread is not runable, returns zero on success */
int thread_suspend(thread_id threadidx);
/* mark thread is runable, returns zero on success */
int thread_resume(thread_id threadidx);

thread_id thread_current(void);

void thread_sleep(uint32_t systicks);

#ifdef __cplusplus
}
#endif

#endif // THREAD_H
