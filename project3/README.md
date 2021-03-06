# malloc Library (UNIX/Linux)

## Introdunction

This allocation system is designed mainly based on Buddy Allocation Algorithm, which can effectively decrease inner fragmentation.

1. If the size requested to be allocated > 512 bytes (include meta size), mmap is called.
it contains a LARGE_HEAD, which maintains a list of large block.
2. If the size requested to be allocated <= 512 bytes (include meta size), sbrk is used to implement buddy system.

Segregated free list was used to manage the free blocks for higher memory utilization and throughput.It was implemented by a double linked free list called FREELIST[k], which maintains current free node according to its order of size. k ranges from 0 to 14, and the usable k ranges from 6 to 14, which means the minimum size of a node is 2^6 = 64 bytes = size of meta (56 bytes) + available size (8 bytes), and the maximum size of a node is 2^14 = 16384 bytes = 4 * page size.

Each node has a meta struct, which contains the following properties:

1. free: 0 if node is in use, and 1 if it is free.
2. kval: for buddy malloc, it represents the order of size of this node, and for large 3. malloc, it is 1.
4. size: size of this node, includes meta size.
5. prev: previous block in the list.
6. next: next block in the list.
7. base: for buddy allocation, it represents start address of the buddy arena (4 pages each), which is used to calculate the address of buddy of the this node. For large allocation, it means nothing.
8. tid: pthread id represents the thread which owns this node.
9. magic: 123456 stands for buddy block and 654321 stands for large block

For Allocation Request:

1. If the size + meta > 512 bytes, requests a space by mmap, make a node, and add to the large list.
2. If the size + meta <= 512 bytes, calculate the corresponding order of size k, and search for FREELIST[k], if there is a node in this buddy list, and it is owned by current thread, then take it from the buddy list and return it. If not, then search in FREELIST[k+1], and so on, until there is a node fulfill the request. If there is no node available inFREELIST[14], require new buddy space (4 pages) by sbrk(). After there is available node in > k FREELIST, start split it to lower level, and move them to lower level FREELIST recursively until there is a node in FREELIST[k], which k is the request order of size. Take it from the list and return the pointer.

For Free Request:

1. If it is large block, call munmap() to free it.
2. If it is buddy block, set it to free and find its buddy. If the buddy is also free, combine them and free them together in higher level, until the buddy is not free, to it reaches the highest level k = 14. Add the final free node into corresponding free list.

I use the same struct for both large block and buddy block, and use magic to differentiate them, because it is easier to tell the pointer is large to buddy during free process.

## Run Instruction

1. Open terminal
2. Go to project file folder
2. Type make clean
3. Type make

