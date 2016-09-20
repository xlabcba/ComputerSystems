#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    while (1) {
        printf(".\t");
        fflush(stdout);
        sleep(1);
    }
}