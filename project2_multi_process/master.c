#include "master.h"

void doSequential(char * worker_path, char * x, char * n){

    int max_n = atoi(n);
    int base_x = atoi(x);
    int pipefds[max_n][2];
    double buffer;
    double sum = 0;
    pid_t cpid;

    // create all the pipes
    for(int i = 0; i < max_n; i++){
        if(pipe(pipefds[i]) < 0){
            fprintf(stderr, "pipe failed\n");
            exit(1);
        }
        //printf("cycle: %d, maxfd: %d, pipefds read-end: %d\n", k, maxfd, pipefds[k][0]);
    }

    // do fork: connect write-end and exec on child & wait on parent
    for(int j = 0; j < max_n; j++){

        cpid = fork();

        if (cpid < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (cpid == 0){ // enter child
            close(pipefds[j][0]); // close read-end
            dup2(pipefds[j][1],1); // connect write-end to stdout
            char * str = malloc(16);
            snprintf(str, 16, "%d",  j); // change integer to string to pass to worker
            execl(worker_path, worker_path, "-x", x, "-n", str, "fromMaster", NULL);
            close(pipefds[j][1]);
        } else{ // enter parent
            wait(NULL);
            close(pipefds[j][1]); // close write-end
            read(pipefds[j][0], &buffer, sizeof(double)); // read read-end
            sum += buffer;
            printf("worker %d: %d^%d / %d! : %f\n", j, base_x, j, j, buffer);
            close(pipefds[j][0]);
        }
    }

    printf("sum: %f\n", sum);
}

void doSelect(char * worker_path, char * x, char * n){

    int max_n = atoi(n);
    int base_x = atoi(x);
    int pipefds[max_n][2];
    int toRead[max_n];
    int result = 0;
    int counter = 0;
    int maxfd = 0;
    double buffer;
    double sum = 0;
    fd_set readset;
    pid_t cpid;

    // create all the pipes
    for(int i = 0; i < max_n; i++){
        if(pipe(pipefds[i]) < 0){
            fprintf(stderr, "pipe failed\n");
            exit(1);
        }
        toRead[i] = 1;
        //printf("cycle: %d, maxfd: %d, pipefds read-end: %d\n", k, maxfd, pipefds[k][0]);
    }

    // do fork: connect write-end and exec on child
    for(int j = 0; j < max_n; j++){

        cpid = fork();

        if (cpid < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (cpid == 0){
            close(pipefds[j][0]); // close read-end
            dup2(pipefds[j][1],1); // connect write-end to stdout
            char * str = malloc(16);
            snprintf(str, 16, "%d",  j); // change integer to string
            execl(worker_path, worker_path, "-x", x, "-n", str, "fromMaster", NULL);
            close(pipefds[j][1]);
        } else{}
    }

    // do select
    do{

        // reset fd_set
        FD_ZERO(&readset);
        for(int k = 0; k < max_n; k++){
            if(toRead[k] == 1){
                FD_SET(pipefds[k][0], &readset);
                maxfd = (maxfd > pipefds[k][0]) ? maxfd : pipefds[k][0];
            }
        }

        // select
        result = select(maxfd + 1, &readset, NULL, NULL, NULL);
        // int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout);

        // check select result
        if(result == -1){
            fprintf(stderr, "select failed at count %d\n", counter);
            exit(1);
        } else{
            for(int l = 0; l < max_n; l++){
                if(FD_ISSET(pipefds[l][0], &readset)){
                    close(pipefds[l][1]);
                    read(pipefds[l][0], &buffer, sizeof(double));
                    toRead[l] = 0;
                    sum += buffer;
                    counter += 1;
                    printf("worker %d: %d^%d / %d! : %f\n", l, base_x, l, l, buffer);
                    close(pipefds[l][0]);
                }
            }
        }

    } while(counter < max_n);

    printf("sum: %f\n", sum);
}

void doPoll(char * worker_path, char * x, char * n){

    int max_n = atoi(n);
    int base_x = atoi(x);
    int pipefds[max_n][2];
    int result = 0;
    int counter = 0;
    double buffer;
    double sum = 0;
    struct pollfd pfds[max_n];
    nfds_t nfds = 0;
    pid_t cpid;

    // create all the pipes
    for(int i = 0; i < max_n; i++){
        if(pipe(pipefds[i]) < 0){
            fprintf(stderr, "pipe failed\n");
            exit(1);
        }
        //printf("cycle: %d, maxfd: %d, pipefds read-end: %d\n", k, maxfd, pipefds[k][0]);
    }

    // do fork: connect write-end and exec on child
    for(int j = 0; j < max_n; j++){

        cpid = fork();

        if (cpid < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (cpid == 0){
            close(pipefds[j][0]); // close read-end
            dup2(pipefds[j][1],1); // connect write-end to stdout
            char * str = malloc(16);
            snprintf(str, 16, "%d",  j); // change integer to string
            execl(worker_path, worker_path, "-x", x, "-n", str, "fromMaster", NULL);
            close(pipefds[j][1]);
        } else{}
    }

    // set poll fds
    for(int k = 0; k < max_n; k++){
        pfds[k].fd = pipefds[k][0];
        pfds[k].events = POLLIN;
        nfds += 1;
    }

    // do poll
    do{

        // do poll
        result = poll(pfds, nfds, -1);
        // int poll(struct pollfd *fds, nfds_t nfds, int timeout);

        // check result
        if(result == -1){
            fprintf(stderr, "poll failed at count %d\n", counter);
            exit(1);
        } else {
            for(int l = 0; l < max_n; l++){
                if(pfds[l].revents & POLLIN){
                    close(pipefds[l][1]);
                    read(pipefds[l][0], &buffer, sizeof(double));
                    sum += buffer;
                    counter += 1;
                    close(pipefds[l][0]);
                    printf("worker %d: %d^%d / %d! : %f\n", l, base_x, l, l, buffer);
                }
            }
        }
    }while(counter < max_n);

    printf("sum: %f\n", sum);
}

void doEPoll(char * worker_path, char * x, char * n){

    int max_n = atoi(n);
    int base_x = atoi(x);
    int toRead[max_n];
    int pipefds[max_n][2];
    int nfds;
    int result = 0;
    int counter = 0;
    int epollfd;
    double buffer;
    double sum = 0;
    struct epoll_event ev, events[max_n];
    pid_t cpid;

    // create all the pipes
    for(int i = 0; i < max_n; i++){
        if(pipe(pipefds[i]) < 0){
            fprintf(stderr, "pipe failed\n");
            exit(1);
        }
        toRead[i] = 1;
        //printf("cycle: %d, maxfd: %d, pipefds read-end: %d\n", k, maxfd, pipefds[k][0]);
    }

    // do fork: connect write-end and exec on child
    for(int j = 0; j < max_n; j++){

        cpid = fork();

        if (cpid < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (cpid == 0){
            close(pipefds[j][0]); // close read-end
            dup2(pipefds[j][1],1); // connect write-end to stdout
            char * str = malloc(16);
            snprintf(str, 16, "%d",  j); // change integer to string
            execl(worker_path, worker_path, "-x", x, "-n", str, "fromMaster", NULL);
            close(pipefds[j][1]);
        } else{}
    }

    // create epollfd
    epollfd = epoll_create(max_n + 1);
    if (epollfd == -1){
        fprintf(stderr, "epoll create failed\n");
        exit(1);
    }

    // add pipes
    for(int k = 0; k < max_n; k++){
        ev.events = EPOLLIN;
        ev.data.fd = pipefds[k][0];
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefds[k][0], &ev) == -1) {
            // int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
            fprintf(stderr, "epoll add failed at cycle: %d\n", k);
            exit(1);
        }
    }

    // do epoll
    do{
        // do epoll_wait
        result = epoll_wait(epollfd, events, max_n, -1);
        // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

        // check result
        if(result == -1){
            fprintf(stderr, "epoll failed at count %d\n", counter);
            exit(1);
        } else {
            for(int l = 0; l < result; l++){
                for(int m = 0; m < max_n; m++){
                    if(events[l].data.fd == pipefds[m][0] && toRead[m] == 1){
                        close(pipefds[m][1]);
                        read(pipefds[m][0], &buffer, sizeof(double));
                        toRead[m] = 0;
                        sum += buffer;
                        counter += 1;
                        close(pipefds[m][0]);
                        printf("worker %d: %d^%d / %d! : %f\n", m, base_x, m, m, buffer);
                    }
                }
            }
        }
    }while(counter < max_n);

    close(epollfd);
    printf("sum: %f\n", sum);
}

int main(int argc, char *argv[]){

    char * worker_path;
    char * mechanism;
    char * x;
    char * n;

    worker_path = argv[2];
    mechanism = argv[4];
    x = argv[6]; // get base char *
    n = argv[8]; // get factor char *

    if(strcmp(mechanism, SEQUENTIAL) == 0) {
        doSequential(worker_path, x, n);
    } else if (strcmp(mechanism, SELECT) == 0) {
        doSelect(worker_path, x, n);
    } else if (strcmp(mechanism, POLL) == 0) {
        doPoll(worker_path, x, n);
    } else if (strcmp(mechanism, EPOLL) == 0) {
        doEPoll(worker_path, x, n);
    } else {
        fprintf(stderr, "Illegal Mechanism\n");
        exit(1);
    }
    printf("Accomplished!\n");
    return 0;
}
