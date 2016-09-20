//
// Created by LiXie on 16/4/26.
//

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

/* Binary semaphore */
typedef struct sema {
    pthread_mutex_t mutex;
    pthread_cond_t   cond;
    int v;
} Sema;

/* Task */
typedef struct task{
    struct task*  prev;
    void*  (*fctn)(void* data);
    void*  data;
} Task;

/* Task Queue */
typedef struct task_q{
    pthread_mutex_t rwmutex; // used for queue access
    Task  *head;
    Task  *tail;
    Sema  *sema;             // flag as binary semaphore
    int   len;
} Task_q;

/* Thread */
typedef struct thread{
    pthread_t pthread;       // pointer to this thread
    struct threadpool* tp;   // pointer to its threadpool
} Thread;


/* Threadpool */
typedef struct threadpool{
    Thread**   threads;                  // pointer to threads
    volatile int t_num_total;            // total threads number
    volatile int t_num_inuse;            // using threads number
    pthread_mutex_t  thread_lock;        // lock single thread
    pthread_cond_t  all_finished;        // condition for tp_join
    Task_q*  task_q;                     // pointer to task queue
} Threadpool;

static volatile int hereIsThread;

// start thread pool functions
Threadpool* tp_init(int t_num);
int tp_add(Threadpool*, void *(*fctn)(void*), void* data);
void tp_join(Threadpool*);
void tp_end(Threadpool*);
// end thread pool functions

// start thread functions
static int  thread_init(Threadpool* tp, Thread** thread);
static void* thread_exec(Thread* thread);
static void  thread_free(Thread* thread);
// end thread functions

// start task queue functions
static int   task_q_init(Threadpool* tp);
static void  task_q_push(Threadpool* tp, Task* newTask);
static struct task* task_q_pull(Threadpool* tp);
static void  task_q_free(Threadpool* tp);
// end task queue functions