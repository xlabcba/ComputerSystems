#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <assert.h>

pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
# define MALLOC_LOCK pthread_mutex_lock(&malloc_mutex);
# define MALLOC_UNLOCK pthread_mutex_unlock(&malloc_mutex);

#define HIGH_EXPO 14    // 2^14 = 4 * 4096 bytes = 4 pages
#define LOW_EXPO 6    // 2^6 = 64 bytes > block size (40 bytes) + 8 bytes
#define LARGE_MALLOC_SIZE 512 //limit between buddy and large malloc
//#define INIT_ALIGN_NUMBER 64 //8 * 64 = 512 bytes
//#define ALIGN_UNIT 8 //return pointer mod 8 bytes == 0
#define PAGE_NUMBER 4 // 4 pages for each sbrk call

typedef long long Align;

union block {
    struct {
        unsigned free;
        unsigned kval;
        size_t size;
        union block *prev;
        union block *next;
        union block *base;
        pthread_t tid;
        int magic;
    } meta;
    Align x;
};

typedef union block Block;

static Block *FREELIST[HIGH_EXPO + 1];

static Block *BUDDY_HEAD = NULL;

static Block *LARGE_HEAD = NULL;

static unsigned long long BUDDY_ARENA_COUNT = 0;

static unsigned long long BUDDY_BLOCK_COUNT = 0;

static unsigned long long BUDDY_SPACE_INUSE = 0;

static unsigned long long BUDDY_BLOCK_INUSE = 0;

static unsigned long long LARGE_ARENA_INUSE = 0;

static unsigned long long LARGE_ARENA_FREED = 0;

static unsigned long long LARGE_SPACE_INUSE = 0;

static unsigned long long LARGE_SPACE_FREED = 0;

static unsigned long long BUDDY_MALLOC_REQUEST = 0;

static unsigned long long LARGE_MALLOC_REQUEST = 0;

static unsigned long long BUDDY_FREE_REQUEST = 0;

static unsigned long long LARGE_FREE_REQUEST = 0;


//void buddy_init();
void buddy_new_space();
unsigned get_k(size_t size);
//unsigned align_size(size_t size);
void split(unsigned k);
void *large_malloc(size_t size);
void *buddy_malloc(size_t size);
void large_free(void * ptr);
void buddy_free(void * ptr);

void *mymalloc(size_t size);
void myfree(void *ptr);

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void malloc_stats();

