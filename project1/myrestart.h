#include <errno.h>
#include <ucontext.h>
#include "main.c"

#ifndef MINI_DMTCP2_MYRESTART_H
#define MINI_DMTCP2_MYRESTART_H

void changeStack();
void restoreMemory(char * file_to_read);
int main(int argc, char* argv[]);

#endif //MINI_DMTCP2_MYRESTART_H