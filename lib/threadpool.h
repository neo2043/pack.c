#ifndef THREADPOOL
#define THREADPOOL

#include <pthread.h>
#include <stdatomic.h>

typedef struct {
    pthread_cond_t jobqueue_wait_cond;
    pthread_mutex_t jobqueue_wait_mutex;
} jobqueue_sync_t;

typedef struct {
    void *arg;
    struct job_t *next;
    void *(*thread_func)(void *arg);
} job_t;

typedef struct {
    job_t *front;
    job_t *rear;
    int len;
    pthread_mutex_t jobqueue_mutex;
    jobqueue_sync_t *jobqueue_sync;
} jobqueue_t;

typedef struct {
    pthread_t *thread;
    _Atomic(int) num_threads_alive;
    _Atomic(int) num_threads_working;
    _Atomic(bool) keep_threadpool_alive;
    pthread_cond_t threadpool_thread_idle_cond;
    pthread_mutex_t threadpool_thread_count_mutex;
    jobqueue_t *jobqueue;
} threadpool_t;


threadpool_t *threadpool_t_init(int thread_num);
void threadpool_add_work(const threadpool_t *threadpool, void *(*worker_func)(void *arg), void *arg);
void threadpool_wait(threadpool_t *threadpool);
void threadpool_destroy(threadpool_t *threadpool);
int get_thread_alive(threadpool_t *threadpool);

#endif