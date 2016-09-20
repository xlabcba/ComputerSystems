#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ucontext.h>

#ifndef MINI_DMTCP_MAIN_H
#define MINI_DMTCP_MAIN_H

#define HEADERS_MAX_NUM 50  // max number of lines of headers
#define HEADERS_MAX_LEN 500  // max length of a header
#define MAPS_MAX_SIZE HEADERS_MAX_NUM * HEADERS_MAX_LEN

typedef enum {stack = 1, vsyscall = 2, others = 3} header_type;

struct header_struct
{
    unsigned long long memo_start;
    unsigned long long memo_end;
    int protection;
    header_type h_type;
};

const char STACK[] = "[stack]";
const char VSYSCALL[] = "[vsyscall]";
const char SPACE[2] = " ";
const char PNTR_HYPHEN[2] = "-";
const char CHAR_HYPHEN = '-';
const char CHAR_NULL = '\0';
const size_t STRUCT_SIZE = sizeof(struct header_struct);

void getCurrMaps();
void getHeaders();
void getHeaderStructs();
void saveAll(char * file_to_save);
void chk_ptr();

// helper function: read hexadecimal
unsigned long long readhex (char *value)
{
    unsigned long long v;
    v = 0;
    int counter = 0;
    char c;
    while (1) {
        c =  *(value + counter);
        if ((c >= '0') && (c <= '9')) c -= '0';
        else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
        else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
        else break;
        v = v * 16 + c;
        counter += 1;
    }
    return v;
}

#endif //MINI_DMTCP_MAIN_H