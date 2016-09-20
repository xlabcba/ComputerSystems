#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    /*
    size_t size = 12;
    void *mem = malloc(size);
    printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
    assert(mem != NULL);
    free(mem);
    printf("Successfully free'd %zu bytes from addr %p\n", size, mem);
    return 0;
     */

    void *mem[40];
    printf("I am here!\n");
    size_t size = 456;
    printf("********************I am here two!*******************\n");
    for (int i = 0; i < 34; i++) {
        printf("I AM %d\n", i);
        mem[i] = malloc(size);
        printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem[i]);
    }
    printf("********************testing!*******************\n");

    mem[34] = malloc(600);
    printf("I am here 0\n");

    mem[34] = realloc(mem[34], 27);
    printf("I am here 1\n");

    mem[34] = realloc(mem[34], 1001);

    printf("I am here\n");
    mem[35] = calloc(4, 60);
    mem[36] = calloc(10, 60);
    printf("I am here two\n");
    mem[37] = malloc(574);
    mem[38] = malloc(575);
    mem[39] = malloc(576);
    printf("I am here three, 37: %p\n", mem[38]);



    printf("********************end of testing!*******************\n");


    /*
    for (int j = 0; j < 37; j++) {
        printf("I AM %d\n", j);
        free(mem[j]);
        printf("Successfully free'd %zu bytes from addr %p\n", size, mem[j]);
    }
     */
    free(mem[23]);
    free(mem[14]);
    free(mem[12]);
    free(mem[15]);
    free(mem[8]);
    free(mem[3]);
    free(mem[34]);
    free(mem[22]);
    free(mem[1]);
    free(mem[6]);
    free(mem[0]);
    free(mem[7]);
    free(mem[2]);
    free(mem[19]);
    free(mem[26]);
    free(mem[32]);
    free(mem[20]);
    free(mem[21]);
    free(mem[5]);
    free(mem[29]);
    free(mem[28]);
    free(mem[11]);
    free(mem[4]);
    free(mem[27]);
    free(mem[10]);
    free(mem[30]);
    free(mem[31]);
    free(mem[16]);
    free(mem[13]);
    free(mem[24]);
    free(mem[17]);
    free(mem[18]);
    free(mem[9]);
    free(mem[25]);
    free(mem[36]);
    free(mem[35]);
    free(mem[33]);
    free(mem[37]);
    free(mem[38]);
    free(mem[39]);

    /*printFreeList();*/
    printf("Successfully free ALL\n");

    malloc_stats();

    return 0;

}