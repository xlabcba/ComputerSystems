//
// Created by LiXie on 16/4/26.
//

#include "threadpool.h"


// Initialize thread pool
Threadpool* tp_init(int t_num){

    hereIsThread = 1;

    if (t_num < 0){
        t_num = 0;
    }

    // Create thread pool
    Threadpool* tp;
    tp = (Threadpool*)malloc(sizeof(Threadpool));
    if (tp == NULL){
        fprintf(stderr, "fail to allocate memory for thread pool\n");
        return NULL;
    }
    tp->t_num_total   = 0;
    tp->t_num_inuse = 0;

    // Initialize task q
    if (task_q_init(tp) == -1){
        fprintf(stderr, "fail to allocate memory for task queue\n");
        free(tp);
        return NULL;
    }

    // Create threads in pool
    tp->threads = (Thread**)malloc(t_num * sizeof(Thread *));
    if (tp->threads == NULL){
        fprintf(stderr, "fail to allocate memory for threads\n");
        task_q_free(tp);
        free(tp->task_q);
        free(tp);
        return NULL;
    }

    pthread_mutex_init(&(tp->thread_lock), NULL);
    pthread_cond_init(&tp->all_finished, NULL);

    // Initialize threads
    int n;
    for (n=0; n<t_num; n++){
        thread_init(tp, &tp->threads[n]);
        printf("Initiated thread No.%d in thread pool \n", n);
    }

    // Wait until all threads initialized
    while (tp->t_num_total != t_num) {}

    return tp;
}


// Add task to thread pool
int tp_add(Threadpool* tp, void *(*fctn)(void*), void* data){
    Task* newTask;

    newTask=(Task*)malloc(sizeof(Task));
    if (newTask==NULL){
        fprintf(stderr, "tp_add(): Could not allocate memory for new task\n");
        return -1;
    }

    newTask->fctn=fctn;
    newTask->data=data;

    pthread_mutex_lock(&tp->task_q->rwmutex);
    task_q_push(tp, newTask);
    pthread_mutex_unlock(&tp->task_q->rwmutex);

    return 0;
}


// Wait until all tasks finished
void tp_join(Threadpool* tp){
    pthread_mutex_lock(&tp->thread_lock);
    while (tp->task_q->len || tp->t_num_inuse) {
        pthread_cond_wait(&tp->all_finished, &tp->thread_lock);
    }
    pthread_mutex_unlock(&tp->thread_lock);
}


// End thread pool
void tp_end(Threadpool* tp){

    if (tp == NULL) return ;

    volatile int threads_total = tp->t_num_total;

    // Each thread stop loop
    hereIsThread = 0;

    // Kill idle thread
    double TIMEOUT = 1.0;
    time_t start, end;
    double tpassed = 0.0;
    time (&start);
    while (tpassed < TIMEOUT && tp->t_num_total){

        // start sema post all
        pthread_mutex_lock(&tp->task_q->sema->mutex);
        tp->task_q->sema->v = 1;
        pthread_cond_broadcast(&tp->task_q->sema->cond);
        pthread_mutex_unlock(&tp->task_q->sema->mutex);
        // end sema post all

        time (&end);
        tpassed = difftime(end,start);

    }

    // Post to all
    while (tp->t_num_total){

        // start sema post all
        pthread_mutex_lock(&tp->task_q->sema->mutex);
        tp->task_q->sema->v = 1;
        pthread_cond_broadcast(&tp->task_q->sema->cond);
        pthread_mutex_unlock(&tp->task_q->sema->mutex);
        // end sema post all

        sleep(1);

    }

    // Free all
    task_q_free(tp);
    free(tp->task_q);

    int n;
    for (n=0; n < threads_total; n++){
        thread_free(tp->threads[n]);
    }

    free(tp->threads);
    free(tp);
}

// Initialize a thread in thread pool
static int thread_init (Threadpool* tp, Thread** thread){

    *thread = (Thread*)malloc(sizeof(Thread));
    if (thread == NULL){
        fprintf(stderr, "fail to allocate memory for thread\n");
        return -1;
    }

    (*thread)->tp = tp;

    pthread_create(&(*thread)->pthread, NULL, (void *)thread_exec, (*thread));
    pthread_detach((*thread)->pthread);
    return 0;
}

// Thread loop to do tasks until tp_end
static void* thread_exec(Thread* thread){

    // Created thread
    Threadpool* tp = thread->tp;

    // Mark thread as alive
    pthread_mutex_lock(&tp->thread_lock);
    tp->t_num_total += 1;
    pthread_mutex_unlock(&tp->thread_lock);

    while(hereIsThread){

        // start sema wait
        pthread_mutex_lock(&tp->task_q->sema->mutex);
        while (tp->task_q->sema->v != 1) {
            pthread_cond_wait(&tp->task_q->sema->cond, &tp->task_q->sema->mutex);
        }
        tp->task_q->sema->v = 0;
        pthread_mutex_unlock(&tp->task_q->sema->mutex);
        // end sema wait

        if (hereIsThread){

            pthread_mutex_lock(&tp->thread_lock);
            tp->t_num_inuse++;
            pthread_mutex_unlock(&tp->thread_lock);

            // Dequeue task from q and execute it
            void*(*fctn)(void* data);
            void*  data;
            pthread_mutex_lock(&tp->task_q->rwmutex);
            Task* task = task_q_pull(tp);
            pthread_mutex_unlock(&tp->task_q->rwmutex);
            if (task) {
                fctn = task->fctn;
                data  = task->data;
                fctn(data);
                free(task);
            }

            pthread_mutex_lock(&tp->thread_lock);
            tp->t_num_inuse--;
            if (!tp->t_num_inuse) {
                pthread_cond_signal(&tp->all_finished);
            }
            pthread_mutex_unlock(&tp->thread_lock);

        }
    }
    pthread_mutex_lock(&tp->thread_lock);
    tp->t_num_total --;
    pthread_mutex_unlock(&tp->thread_lock);

    return NULL;
}

// Free a thread
static void thread_free (Thread* thread){
    free(thread);
}


// Initialize task q
static int task_q_init(Threadpool* tp){

    tp->task_q = (Task_q*)malloc(sizeof(Task_q));
    if (tp->task_q == NULL){
        return -1;
    }
    tp->task_q->len = 0;
    tp->task_q->head = NULL;
    tp->task_q->tail  = NULL;

    tp->task_q->sema = (Sema*)malloc(sizeof(Sema));
    if (tp->task_q->sema == NULL){
        return -1;
    }

    pthread_mutex_init(&(tp->task_q->rwmutex), NULL);

    // start initiate sema to 0
    pthread_mutex_init(&(tp->task_q->sema->mutex), NULL);
    pthread_cond_init(&(tp->task_q->sema->cond), NULL);
    tp->task_q->sema->v = 0;
    // end initiate sema to 0

    return 0;
}

// Enqueue new task
static void task_q_push(Threadpool* tp, Task* newTask){

    newTask->prev = NULL;

    if (tp->task_q->len == 0) {
        tp->task_q->head = newTask;
        tp->task_q->tail  = newTask;
    } else {
        tp->task_q->tail->prev = newTask;
        tp->task_q->tail = newTask;
    }

    tp->task_q->len++;

    // start sema post
    pthread_mutex_lock(&tp->task_q->sema->mutex);
    tp->task_q->sema->v = 1;
    pthread_cond_signal(&tp->task_q->sema->cond);
    pthread_mutex_unlock(&tp->task_q->sema->mutex);
    // end sema post

}


// Dequeue the 1st task
static Task* task_q_pull(Threadpool* tp){

    Task* task = tp->task_q->head;

    if (tp->task_q->len == 0) {

        return task;

    } else if (tp->task_q->len == 1) {

        tp->task_q->head = NULL;
        tp->task_q->tail  = NULL;
        tp->task_q->len = 0;
        return task;

    } else {
        tp->task_q->head = task->prev;
        tp->task_q->len--;

        // start sema post
        pthread_mutex_lock(&tp->task_q->sema->mutex);
        tp->task_q->sema->v = 1;
        pthread_cond_signal(&tp->task_q->sema->cond);
        pthread_mutex_unlock(&tp->task_q->sema->mutex);
        // end sema post

        return task;
    }
}

// Free queue resources
static void task_q_free(Threadpool* tp){

    while(tp->task_q->len){
        free(task_q_pull(tp));
    }

    tp->task_q->head = NULL;
    tp->task_q->tail  = NULL;

    // start reset sema to 0
    pthread_mutex_init(&(tp->task_q->sema->mutex), NULL);
    pthread_cond_init(&(tp->task_q->sema->cond), NULL);
    tp->task_q->sema->v = 0;
    // end reset sema to 0

    tp->task_q->len = 0;

    free(tp->task_q->sema);
}