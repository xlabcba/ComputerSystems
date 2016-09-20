#include "master.h"
#include "threadpool.h"

unsigned long long calcFac(int n){
    unsigned long long result;
    result = 1;
    for(int i = 2; i <= n; i++){
        result *= i;
    }
    return result;
}

unsigned long long calcPow(int x, int n){
    unsigned long long result;
    result = 1;
    for(n; n > 0; n--){
        result *= x;
    }
    return result;
}

double calcResult(int x, int n){
    unsigned long long fraction, numerator;
    double result;
    fraction = calcPow(x,n);
    numerator = calcFac(n);
    result = (double) fraction / numerator;
    return result;
}

void worker(Data *data){

    pthread_mutex_t *lock = data->lock;

    pthread_mutex_lock(&sum_mutex);

    int n = data->i;
    int x = data->base_x;

    double result = calcResult(x,n);

    printf("Thread %u working on worker %d : %d^%d / %d! : %f\n", (int)pthread_self(), n, x, n, n, result);

    sum = sum + result;

    pthread_mutex_unlock(&sum_mutex);

}



void doThreadPool(char * t,  char * x, char * n){

    int t_num = atoi(t);
    int max_n = atoi(n);
    int base_x = atoi(x);


    Threadpool* tp = tp_init(t_num);

    for(int i = 0; i < max_n; i++){

        Data *data = (Data *) malloc(sizeof(Data));
        data->i = i;
        data->base_x = base_x;
        data->lock = &sum_mutex;

        tp_add(tp, (void *) worker, (void *) data);

    }

    tp_join(tp);

    tp_end(tp);
}

int main(int argc, char *argv[]){

    char * t;
    char * x;
    char * n;

    t = argv[2]; // get number char *
    x = argv[4]; // get base char *
    n = argv[6]; // get factor char *

    doThreadPool(t, x, n);

    printf("sum: %f\n", sum);

    printf("Accomplished!\n");
    return 0;
}
