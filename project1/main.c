#include "main.h"

// Flag to mark where the getcontext()
int jump_from_restart = 1;

// Get maps of current process

char maps[MAPS_MAX_SIZE];

void getCurrMaps()
{
    printf("Reading maps of current process ... \n");

    FILE * file = fopen("/proc/self/maps", "r");
    char header[HEADERS_MAX_LEN];

    if (file) {
        while (fgets(header, HEADERS_MAX_LEN, file) != NULL) {
            // each step will read a line of header
            strcpy(maps + strlen(maps), header);
        }
        printf("Maps as follows: \n%s\n", maps);
        fclose(file);
    }

    printf("~~~~~ End Reading Maps~~~~~\n\n\n");
}

// Get headers of the maps

char * headers[HEADERS_MAX_NUM]; // array of headers in the map

void getHeaders()
{
    printf("Reading headers of the maps ... \n");

    char * maps_cpy = (char*) malloc(strlen(maps) + 1);
    maps_cpy = strncpy(maps_cpy, maps, strlen(maps));
    maps_cpy[strlen(maps)] = '\0';
    char deli[2] = "\n";
    headers[0] = strtok(maps_cpy, deli);

    int i = 1;
    char * header;
    while ((header = strtok(NULL, deli)) != NULL)
    {
        headers[i++] = header;
    }
    headers[i] = NULL;

    printf("Headers as follows:\n");
    for (int j = 0; headers[j] != NULL; j++)
    {
        printf("%s\n", headers[j]);
    }

//    free(maps_cpy);

    printf("~~~~~ End Getting Headers~~~~~\n\n\n");
}

// Parse header into header struct

struct header_struct * header_structs[HEADERS_MAX_NUM];

void getHeaderStructs()
{
    printf("Parsing headers ...\n");
    int prots[3] = {PROT_READ, PROT_WRITE, PROT_EXEC};
    char curr_header[HEADERS_MAX_LEN];
    int i;
    for (i = 0; headers[i] != NULL; i++)
    {
        // Assign space for structs in advance
        header_structs[i] = (struct header_struct *) malloc(STRUCT_SIZE);

        // Get the first token - range of address
        char * address = strtok(headers[i], SPACE);
        char * new_addr = (char *)malloc(sizeof(char) * strlen(address) + 1);
        strncpy(new_addr, address, strlen(address));
        new_addr[strlen(address)] = '\0';

        // Get the second token - protection
        char * curr_prot = strtok(NULL, SPACE);
        int permit = 0;
        for (int j = 0; j < 3; j++) {
            if(curr_prot[j] != CHAR_HYPHEN) {
                permit |= prots[j];
            }
        }
        header_structs[i] -> protection = permit;

        // Skip the third to fifth token -
        // offset, dev, and inode
        for (int k = 0; k < 3; k++){
            strtok(NULL, SPACE);
        }

        // Get the sixth token - path
        char * path = strtok(NULL, SPACE);
        if(path && !strcmp(path, STACK)){
            header_structs[i] -> h_type = stack;
        }
        else if(path && !strcmp(path, VSYSCALL)){
            header_structs[i] -> h_type = vsyscall;
        }
        else{
            header_structs[i] -> h_type = others;
        }

        // Separate address range
        char *token;
        token = strtok(new_addr, PNTR_HYPHEN);
        header_structs[i] -> memo_start = readhex(token);
        token = strtok(NULL, PNTR_HYPHEN);
        header_structs[i] -> memo_end = readhex(token);
    }
    header_structs[i - 1] = NULL;

    for (int l = 0; header_structs[l] != NULL; l++ ){
        printf("%d : %llu %llu %d %d\n", l, header_structs[l] -> memo_start, header_structs[l] -> memo_end,
               header_structs[l] -> protection, header_structs[l] -> h_type);
    }
    printf("~~~~~ End Parsing Headers ~~~~~\n");
}

// Create a register struct

ucontext_t reg;

void saveAll(char * file_to_save)
{
    printf("Saving All... \n");

    // Create a new file using the given file name
    FILE * file = fopen(file_to_save, "w");

    // Save to file
    for (int i = 0; header_structs[i] != NULL; i++)
    {
        if (header_structs[i] -> protection != 0 & PROT_READ)
        {
            fwrite(header_structs[i], STRUCT_SIZE, 1, file);
            fwrite((void *) header_structs[i] -> memo_start,
                   header_structs[i] -> memo_end - header_structs[i] -> memo_start,
                   1, file);
        }

    }
    printf("all have been saved to %s\n", file_to_save);
    fclose(file);

    // Save register of file
    jump_from_restart = 0;
    getcontext(&reg);
    if (!jump_from_restart) {
        FILE *freg = fopen("ckpt_reg", "w");
        fwrite(&reg, sizeof(reg), 1, freg);
        fclose(freg);
    }

    printf("~~~~~ End Saving All ~~~~~\n\n\n");
}

// main function
void chk_ptr()
{
    getCurrMaps();
    getHeaders();
    getHeaderStructs();
    saveAll("myckpt");
}

__attribute__ ((constructor)) void init_signal() {
    printf("Start to print\n");
    signal(SIGUSR2, chk_ptr);
}

