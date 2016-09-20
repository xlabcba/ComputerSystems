#include "myrestart.h"

char ckpt_image[MAPS_MAX_SIZE];

ucontext_t reg;

const int REG_SIZE = sizeof(reg);

void changeStack()
{
    printf("GETTING CURRENT HEADERS\n");
    getHeaders();

    printf("RELEASING STACK\n");
    int prots[3] = {PROT_READ, PROT_WRITE, PROT_EXEC};
    int i;
    //struct header_struct * header_structs[HEADERS_MAX_NUM];
    char curr_header[HEADERS_MAX_LEN];
    for (i = 0; headers[i] != NULL; i++)
    {
        printf("current header: %s\n", headers[i]);
        // Assign space for structs in advance
        //header_structs[i] = malloc(STRUCT_SIZE);

        // Copy the current header into temp variable
        //printf("taking temp var...\n");
        memset(curr_header, CHAR_NULL, HEADERS_MAX_LEN);
        strncpy(curr_header, headers[i], HEADERS_MAX_LEN);
        //printf("finish temp var...\n");

        // Get the first token - range of address
        char * address = strtok(headers[i], SPACE);

        // Skip the second to fifth token -
        // offset, dev, and inode
        printf("skipping...\n");
        for (int k = 0; k < 4; k++){
            char* skip=strtok(NULL, SPACE);
            printf("skip:%s %d\n", skip,k);
        }
        printf("finish skipping ...\n");

        // Get the sixth token - path
        char * path = strtok(NULL, SPACE);
        printf("path: %s\n", path);
        if(path != NULL && strncmp(path, "[stack:...]", 6) == 0)
        {
            printf("entered if condition\n");
            unsigned long long memo_start = readhex(strtok(address, PNTR_HYPHEN));
            unsigned long long memo_end = readhex(strtok(NULL, PNTR_HYPHEN));
            unsigned long long memo_len = memo_end - memo_start;
            printf("ok up to now\n");
            if (munmap((void *)memo_start, (size_t)memo_len) == 0)
            {
                printf("Successful!!!!\n");
            }
            break;
        }
    }
    printf("FINISH RELEASING\n\n\n");
}

void restoreMemory(char * file_to_read)
{
    printf("reload file: %s\n", file_to_read);

    int i = 0;

    FILE * freg = fopen("ckpt_reg", "r");

    FILE * file = fopen(file_to_read, "r");

    if (fread(&reg, REG_SIZE, 1, freg) < 1){
        printf("unexpexted exit #1 \n");
        exit(0);
    }

    struct header_struct * header_structs[HEADERS_MAX_NUM];

    while(1) {

        // Assign space for structs in advance
        printf("cycle: %d\n", i);
        header_structs[i] = malloc(STRUCT_SIZE);
        size_t read_size = fread(header_structs[i], STRUCT_SIZE, 1, file);

        if (ferror(file) || feof(file)) {
            printf("successful final exit \n");
            break;
        }

        if (read_size == STRUCT_SIZE) {
            if (mprotect(header_structs[i], STRUCT_SIZE, PROT_READ) == -1) {
                printf("unexpexted exit #3 \n");
                perror("fail to assign protection to read\n");
            }
        }

        void *addr;
        addr = mmap((void *) header_structs[i]->memo_start,
                    header_structs[i]->memo_end - header_structs[i]->memo_start,
                    PROT_READ | PROT_EXEC | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

        if (addr == MAP_FAILED) {
            printf("unexpexted exit #4 \n");
            perror("mmap error. ");
            exit(0);
        }

        fread((void *) header_structs[i]->memo_start,
              header_structs[i]->memo_end - header_structs[i]->memo_start, 1, file);
        i++;
    }
    printf("~~~~~ End Restoring Memory ~~~~~\n");
    setcontext(&reg);
    fclose(freg);
    fclose(file);
}

int main(int argc, char* argv[])
{
    // Get Current Maps
    printf("PRINTING MYRESTART.C MAPS\n");
    getCurrMaps();

    // Assign new stack
    printf("ASSIGNING NEW STACK\n");
    mmap((void *) 0x5300000, 0x100000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);

    // Copy path
    printf("COPY CKPY_IMAGE\n");
    strcpy(ckpt_image, argv[1]);

    // Switch stack pointer
    printf("SWITCHING POINTER\n");
    void * stack_pntr = (void *)(0x5300000 + 0x90000);
    asm volatile ("mov %0,%%rsp;" : : "g" (stack_pntr) : "memory");

    // Change Stack to new Stack
    changeStack();

    // Reload Maps
    printf("Reloading from loadMaps %s\n", ckpt_image);
    restoreMemory(ckpt_image);
    printf("Finish Reloading\n");
}

