#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

static pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;
static double sum;

typedef struct Mydata {
    int i;
    int base_x;
    pthread_mutex_t *lock;
} Data;