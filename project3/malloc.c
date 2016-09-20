#include "malloc.h"

/*
void buddy_init()
{
    unsigned k = HIGH_EXPO;
    //void *ptr = sbrk(1 << k);
    void *ptr = sbrk((int)(PAGE_NUMBER * sysconf(_SC_PAGESIZE)));
    if (ptr < 0 || errno == ENOMEM) {
        errno = ENOMEM;
        return;
    }

    FREELIST[k] = (Block *) ptr;
    FREELIST[k]->meta.free = 1;
    FREELIST[k]->meta.kval = k;
    FREELIST[k]->meta.size = (size_t) (1 << k);
    FREELIST[k]->meta.next = NULL;
    FREELIST[k]->meta.prev = NULL;
    FREELIST[k]->meta.tid = pthread_self();
    FREELIST[k]->meta.magic = 123456;
    FREELIST[k]->meta.base = (Block *) ptr;

    BUDDY_ARENA_COUNT++;
    BUDDY_BLOCK_COUNT++;

}
*/

void buddy_new_space()
{
    unsigned k = HIGH_EXPO;
    //void *ptr = sbrk(1 << k);
    void *ptr = sbrk((int)(PAGE_NUMBER * sysconf(_SC_PAGESIZE)));
    if (ptr < 0 || errno == ENOMEM) {
        errno = ENOMEM;
        return;
    }

    Block *curr = (Block *)ptr;

    if (BUDDY_HEAD == NULL) {
        BUDDY_HEAD = curr;
    }

    curr->meta.prev = NULL;
    if (FREELIST[k] != NULL) {
        FREELIST[k]->meta.prev = curr;
    }
    curr->meta.next = FREELIST[k];
    FREELIST[k] = curr;

    curr->meta.free = 1;
    curr->meta.kval = k;
    curr->meta.size = (size_t) (1 << k);
    curr->meta.tid = pthread_self();
    curr->meta.magic = 123456;
    curr->meta.base = curr;

    BUDDY_ARENA_COUNT++;
    BUDDY_BLOCK_COUNT++;
}

/*
 * Returns the k for that size
 */
unsigned get_k(size_t size)
{
    unsigned k = LOW_EXPO;

    size_t availSize = (1 << k) - sizeof(Block);

    while (size > availSize) {
        availSize = ((size_t) 1 << ++k) - sizeof(Block);
    }

    return k;
}

/*
 * Align large malloc size to 8bytes boundary
 */
/*
unsigned align_size(size_t size)
{
    unsigned currSize = INIT_ALIGN_NUMBER * ALIGN_UNIT;

    while (size > currSize) {
        currSize = currSize + ALIGN_UNIT;
    }

    return currSize;
}
*/

/*
 *
 * Split a node.
 *
 */
void split(unsigned k)
{

    Block *curr1 = FREELIST[k];
    while (curr1 != NULL && curr1->meta.tid != pthread_self()) {
        curr1 =  curr1->meta.next;
    }

    if (curr1 != NULL && !curr1->meta.free) {
        printf("WRONG FREE LIST 1\n");
    }

    if (curr1 == NULL && k + 1 <= HIGH_EXPO) {
        split(k + 1);
    } else if (curr1 == NULL && k + 1 > HIGH_EXPO) {
        buddy_new_space();
    }

    Block *curr2 = FREELIST[k];
    while (curr2 != NULL && curr2->meta.tid != pthread_self()) {
        curr2 =  curr2->meta.next;
    }

    if (curr2 != NULL && !curr2->meta.free) {
        printf("WRONG FREE LIST 2\n");
    }

    if (curr2 != NULL) {

        Block *temp = curr2;
        Block *prev = curr2->meta.prev;
        Block *next = curr2->meta.next;
        if (prev != NULL) {
            prev->meta.next = next;
        } else {
            FREELIST[k] = next;
        }
        if (next != NULL) {
            next->meta.prev = prev;
        }
        temp->meta.prev = NULL;
        temp->meta.next = NULL;

        Block *list = FREELIST[k - 1];
        if (list != NULL) {
            while (list->meta.next != NULL) {
                list = list->meta.next;
            }
            list->meta.next = temp;
            temp->meta.prev = list;
        } else {
            FREELIST[k - 1] = temp;
            temp->meta.prev = NULL;
        }

        temp->meta.free = 1;
        temp->meta.kval = k - 1;
        temp->meta.size = (size_t) (1 << (k - 1));

        Block *bud = (Block *) (((size_t)temp) + (size_t)(1 << (k - 1)));
        temp->meta.next = bud;
        bud->meta.free = 1;
        bud->meta.kval = k - 1;
        bud->meta.size = (size_t) (1 << (k - 1));
        bud->meta.next = NULL;
        bud->meta.prev = temp;
        bud->meta.tid = temp->meta.tid;
        bud->meta.magic = 123456;
        bud->meta.base = temp->meta.base;
        BUDDY_BLOCK_COUNT++;
        return;
    } else {
        return;
    }
}


void *large_malloc(size_t size)
{
    size_t needSize = size + sizeof(Block);
    size_t allocSize = needSize; //align_size(needSize);
    void *ptr = mmap(0, allocSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (ptr < 0 || ptr == MAP_FAILED || errno == ENOMEM) {
        errno = ENOMEM;
        return NULL;
    }

    Block *curr = (Block *)ptr;

    curr->meta.size = (unsigned) allocSize;
    curr->meta.kval = (unsigned) 1;
    curr->meta.free = 0;
    curr->meta.tid = pthread_self();
    curr->meta.magic = 654321;

    curr->meta.prev = NULL;
    if (LARGE_HEAD != NULL) {
        LARGE_HEAD->meta.prev = curr;
    }
    curr->meta.next = LARGE_HEAD;
    LARGE_HEAD = curr;

    LARGE_ARENA_INUSE++;
    LARGE_SPACE_INUSE = LARGE_SPACE_INUSE + allocSize;

    curr++;

    void * returnPtr = (void *) curr;
    return returnPtr;
}

void *buddy_malloc(size_t size)
{
    // First time BASE is NULL
    if (BUDDY_HEAD == NULL) {
        buddy_new_space();
    }

    // loop to find size of k
    unsigned k = get_k(size);

    void *returnPtr = NULL;

    Block *curr1 = FREELIST[k];
    while (curr1 != NULL && curr1->meta.tid != pthread_self()) {
        curr1 =  curr1->meta.next;
    }

    // if kth list is null, split on k+1 if its not the max k
    if (curr1 == NULL) {
        split(k + 1);
    }

    Block *curr2 = FREELIST[k];
    while (curr2 != NULL && curr2->meta.tid != pthread_self()) {
        curr2 =  curr2->meta.next;
    }

    if (curr2 != NULL) {

        Block *temp = curr2;
        Block *prev = curr2->meta.prev;
        Block *next = curr2->meta.next;
        if (prev != NULL) {
            prev->meta.next = next;
        } else {
            FREELIST[k] = next;
        }
        if (next != NULL) {
            next->meta.prev = prev;
        }

        BUDDY_BLOCK_INUSE++;
        BUDDY_SPACE_INUSE = BUDDY_SPACE_INUSE + (unsigned long long) (1 << k);
        temp->meta.prev = NULL;
        temp->meta.next = NULL;
        temp->meta.magic = 123456;
        temp->meta.free = 0;
        temp++;
        returnPtr = (void *) temp;
    } else {
        errno = ENOMEM;
    }

    return returnPtr;
}

void large_free(void * ptr) {

    //ptr = ptr - sizeof(Block);
    Block *temp = (Block *) ptr;
    temp--;

    Block *prev_block = temp->meta.prev;
    Block *next_block = temp->meta.next;

    if (prev_block == NULL) {
        LARGE_HEAD = next_block;
    } else {
        prev_block->meta.next = next_block;
    }

    if (next_block != NULL) {
        next_block->meta.prev = prev_block;
    }

    temp->meta.free = 1;
    temp->meta.prev = NULL;
    temp->meta.next = NULL;

    unsigned long long free_size = (unsigned long long) temp->meta.size;


    int r = munmap((void *)temp, temp->meta.size);


    if (r == -1 || errno == ENOMEM) {
        errno = ENOMEM;
        return;
    }

    LARGE_SPACE_FREED = LARGE_SPACE_FREED + free_size;
    LARGE_SPACE_INUSE = LARGE_SPACE_INUSE - free_size;
    LARGE_ARENA_FREED++;
    LARGE_ARENA_INUSE--;

    return;
}

void buddy_free(void * ptr)
{
    if (ptr == NULL) {
        printf("Attempt to free NULL\n");
        return;
    }
    Block *temp = (Block *) ptr;
    temp--;

    if (temp->meta.free) {
        printf("Double FREE\n");
        return;
    }

    if (temp->meta.magic != 123456) {
        printf("Not buddy block\n");
        return;
    }

    unsigned bud_is_free = 0;
    Block *buddy = NULL;
    unsigned k = temp->meta.kval;

    if (k < HIGH_EXPO) {


        buddy = (Block *) ((((size_t) temp - (size_t) temp->meta.base) ^ ((size_t) 1 << k)) + (size_t) temp->meta.base);

        // MIRACLE WORKER
        if (k == buddy->meta.kval)
            bud_is_free = buddy->meta.free;
    }

    if (bud_is_free) {

        Block *prev_block = buddy->meta.prev;
        Block *next_block = buddy->meta.next;

        if(prev_block == NULL) {
            FREELIST[k] = buddy->meta.next;
        } else {
            prev_block->meta.next = buddy->meta.next;
        }

        if(next_block != NULL) {
            next_block->meta.prev = buddy->meta.prev;
        }


        Block *head;
        Block *tail;
        // Find the address that is smallest
        if ((long)buddy < (long)temp) {
            head = buddy;
            tail = temp;
        } else {
            head = temp;
            tail = buddy;
        }


        k++;
        head->meta.kval = k;
        head->meta.free = 0;
        head->meta.size = (size_t) (1 << k);
        head++;

        tail->meta.free = 1;

        BUDDY_BLOCK_COUNT--;
        buddy_free((void *) head);

    } else {
        if (FREELIST[k] == NULL) {
            temp->meta.prev = NULL;
            FREELIST[k] = temp;
        } else {
            Block *curr = FREELIST[k];
            while (curr->meta.next != NULL) {
                curr = curr->meta.next;
            }
            temp->meta.prev = curr;
            curr->meta.next = temp;
        }

        BUDDY_BLOCK_INUSE--;

        temp->meta.kval = k;
        temp->meta.size = (size_t) (1 << k);
        temp->meta.next = NULL;
        temp->meta.free = 1;

    }
    return;
}

void *mymalloc(size_t size)
{
    void *returnPtr = NULL;

    if ((size + sizeof(Block)) > (size_t) LARGE_MALLOC_SIZE) {
        returnPtr = large_malloc(size);
        LARGE_MALLOC_REQUEST++;
        /*
        if ((unsigned long long)returnPtr % 8 != 0) {
            printf("LARGE FAIL ALIGNED TO 8 bytes\n");
        }
        */
        return returnPtr;
    } else {
        returnPtr = buddy_malloc(size);
        BUDDY_MALLOC_REQUEST++;
        /*
        if ((unsigned long long)returnPtr % 8 != 0) {
            printf("BUDDY FAIL ALIGNED TO 8 bytes\n");
        }
        */
        return returnPtr;
    }
}

void myfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    Block *p = (Block *) ptr;
    p--;

    if (p->meta.magic == 654321 && !p->meta.free) {
        //printf("entering large free\n");
        large_free(ptr);
        LARGE_FREE_REQUEST++;
    } else if (p->meta.magic == 123456 && !p->meta.free){
     //printf("entering buddy free\n");
        unsigned long long free_size = (unsigned long long) p->meta.size;
        buddy_free(ptr);
        BUDDY_FREE_REQUEST++;
        BUDDY_SPACE_INUSE = BUDDY_SPACE_INUSE - free_size;
    }
    return;
}

void *malloc(size_t size)
{
    MALLOC_LOCK;
    void *returnPtr = NULL;
    returnPtr = mymalloc(size);
    MALLOC_UNLOCK;
    return returnPtr;
}



void free(void *ptr)
{
    MALLOC_LOCK;

    if(ptr != NULL) {
        myfree(ptr);
    }
    MALLOC_UNLOCK;
    return;
}


void *calloc(size_t nmemb, size_t size)
{
    MALLOC_LOCK;

    void *returnPtr = mymalloc(nmemb * size);
    // MUST INITIALIZE NEW DATA TO ZERO
    if (returnPtr != NULL) {
        memset(returnPtr, 0, nmemb * size);
    }

    MALLOC_UNLOCK;
    return returnPtr;
}

void *realloc(void *ptr, size_t size)
{
    MALLOC_LOCK;

    void * returnPtr;

    if (ptr == NULL) {
        returnPtr = mymalloc(size);
        MALLOC_UNLOCK;
        return returnPtr;
    }


    if (size == 0) {

        myfree(ptr);

        MALLOC_UNLOCK;

        return ptr;
    }

    Block * curr = (Block *) ptr;
    curr--;

    size_t oldSize = curr->meta.size - sizeof(Block);


    if (curr->meta.magic != 123456 && curr->meta.magic != 654321) {
        returnPtr = mymalloc(size);
        MALLOC_UNLOCK;

        return returnPtr;
    }


    if (oldSize != size) {
        returnPtr = mymalloc(size);

        size_t cpySize;
        if (oldSize < size) {
            cpySize = oldSize;
        } else {
            cpySize = size;
        }
        memcpy(returnPtr, ptr, cpySize);
        myfree(ptr);
    } else {
        returnPtr = ptr;
    }

    MALLOC_UNLOCK;
    return returnPtr;

}

void malloc_stats() {
    Block *curr;

    printf("Statistics of Current Thread: %u\n\n", (unsigned)pthread_self());

    printf("BUDDY ALLOCATION (<= 512 bytes with meta data) BY SBRK():\n");
    printf("Total Buddy Malloc Request Number: %llu\n", BUDDY_MALLOC_REQUEST);
    printf("Total Buddy Free Request Number: %llu\n", BUDDY_FREE_REQUEST);
    printf("Total Buddy Arena (4 Pages each) Allocated: %llu\n", BUDDY_ARENA_COUNT);
    printf("Total Buddy Space Allocated: %llu\n", BUDDY_ARENA_COUNT * PAGE_NUMBER * sysconf(_SC_PAGESIZE));
    printf("Total Buddy Space In Use: %llu\n", BUDDY_SPACE_INUSE);
    printf("Total Buddy Block Number: %llu\n", BUDDY_BLOCK_COUNT);
    printf("Total Buddy Block In Use: %llu\n", BUDDY_BLOCK_INUSE);


    for (int k = 0; k <= HIGH_EXPO; k++) {
        printf("Printing FREELIST[%d]\n", k);
        if (FREELIST[k] != NULL) {
            int i=0;
            curr = FREELIST[k];
            while(curr != NULL) {
                printf("the free node %d, free is %d, the size is %zd, tid: %u, base: %p, to: %p\n",i,curr->meta.free,curr->meta.size, (unsigned)curr->meta.tid, curr->meta.base, (void *)curr->meta.base + 4*4096);
                curr = curr->meta.next;
                i++;
            }
        } else {
            printf("the size %d is empty\n",k);
        }
    }

    curr = LARGE_HEAD;
    printf("\nLARGE ALLOCATION (> 512 bytes with meta data) BY MMAP():\n");
    printf("Total Large Malloc Request Number: %llu\n", LARGE_MALLOC_REQUEST);
    printf("Total Large Free Request Number: %llu\n", LARGE_FREE_REQUEST);
    printf("Total Large Arena In use: %llu\n", LARGE_ARENA_INUSE);
    printf("Total Large Space In use: %llu\n", LARGE_SPACE_INUSE);
    printf("Total Large Arena freed: %llu\n", LARGE_ARENA_FREED);
    printf("Total Large Space freed: %llu\n\n", LARGE_SPACE_FREED);

    while(curr!=NULL){
        printf("large block size is %zd,free=%d, tid: %u, addr: %p, LARGE_HEAD: %p\n", curr->meta.size,curr->meta.free, (unsigned)curr->meta.tid, curr, LARGE_HEAD);
        curr = curr->meta.next;
    }

}
