#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/select.h>

const char * SEQUENTIAL = "sequential";
const char * SELECT = "select";
const char * POLL = "poll";
const char * EPOLL= "epoll";
