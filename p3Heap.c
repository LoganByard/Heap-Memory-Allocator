#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include "p3Heap.h"
#include <limits.h>

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block.
 */
typedef struct blockHeader {           

    /*
     * 1) The size of each heap block must be a multiple of 8
     * 2) heap blocks have blockHeaders that contain size and status bits
     * 3) free heap block contain a footer, but we can use the blockHeader 
     *.
     * All heap blocks have a blockHeader with size and status
     * Free heap blocks have a blockHeader as its footer with size only
     *
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     */
    int size_status;

} blockHeader;         

/* Global variable
 * It must point to the first block in the heap and is set by init_heap()
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 * Uses best-fit placement policy to choose free block
 */
void* balloc(int size) {     
	printf("%i, %s", size, "\n\n");
    if (size < 1) {
		return NULL;
	}
	
	size = size + 4; // Add the header to the size
	int padding = (8 - (size % 8)) % 8;	
	size = size + padding; 

	blockHeader *best = NULL;
	int best_size = INT_MAX;
	int curr_size; 
	blockHeader *current = heap_start;
	
	while(current->size_status != 1) {
		curr_size = current->size_status;
		if (curr_size & 1) { //if allocated
			if (curr_size & 2) {
				curr_size = curr_size - 3;
			} else {
				curr_size = curr_size - 1;
			}
			current = (blockHeader*) ((char*) current + curr_size);
		} else { // if free
			if (curr_size & 2) {
				curr_size = curr_size - 2;				
			}
			if (curr_size == size) {
				best = current;
				best_size = curr_size;
				break;
				// break entire loop
			} else if (curr_size > size && curr_size < best_size) {
				best = current;
				best_size = curr_size;
			}
			current = (blockHeader*) ((char*) current + curr_size);
		}
		
	}
	
	if (best == NULL) {
		return NULL;
	}

	best->size_status = best->size_status + 1;
	void *payload = (void *)((char *)best + 4);

	int freed_size;
	if (best_size - size >= 8) {
		freed_size = best_size - size;
		blockHeader *new_block = (blockHeader*) ((char *) best + size);
		new_block->size_status = freed_size + 2;
		best->size_status = best->size_status - freed_size;
		blockHeader *new_footer = (blockHeader*) ((char *) new_block + freed_size - sizeof(blockHeader));
		new_footer->size_status = freed_size;
	} else {
		blockHeader *next_block = (blockHeader*) ((char *) best + size);
		if (next_block->size_status != 1) {
			next_block->size_status = next_block->size_status + 2;
		}
	}	
	
	disp_heap();
	
    return payload;
} 

/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 *
 * If free results in two or more adjacent free blocks,
 * they will be immediately coalesced into one larger free block.
 */                    
int bfree(void *ptr) {  
	printf("%08x, %s", (unsigned int)(ptr), "\n\n");
	if (ptr == NULL) {
		return -1;
	} 

	if ((unsigned long)ptr % 8 != 0) {
		return -1;
	}

	if ((void *)ptr > (void *)(heap_start + alloc_size)) {
		return -1;
	} 

	blockHeader *curr_block = (blockHeader*) ((char *)ptr - sizeof(blockHeader));
	if (!(curr_block->size_status & 1)) {
		return -1;
	}

	int curr_size; 

	if (curr_block->size_status & 2) {
		curr_size = curr_block->size_status - 2;
		curr_size = curr_size - 1;
	} else {
		curr_size = curr_block->size_status - 1;
	}

	blockHeader *new_footer = (blockHeader*) ((char*)curr_block + curr_size - sizeof(blockHeader));
	curr_block->size_status = curr_block->size_status - 1;
	new_footer->size_status = curr_size;	
	blockHeader *next_block = (blockHeader*) ((char*)new_footer + sizeof(blockHeader));
	
	if (next_block->size_status != 1) {
		next_block->size_status = next_block->size_status - 2;
	}

	if (!(next_block->size_status & 1)) {
	    int new_size2 = next_block->size_status + curr_size;
		int j = 0;
        if (curr_block->size_status & 2) {
			j = 2;
		}
		curr_block->size_status = new_size2 + j;
		//blockHeader *next_footer = (blockHeader *) ((
		blockHeader *next_footer = (blockHeader *) ((char *) next_block + next_block->size_status - sizeof(blockHeader));
		next_footer->size_status = new_size2;
		curr_size = curr_block->size_status - j;
	}
	
	
	if (!(curr_block->size_status & 2)) {
		blockHeader *prev_footer = (blockHeader *) ((char *) curr_block - sizeof(blockHeader));
		int prev_size = prev_footer->size_status;
		blockHeader *prev_block = (blockHeader *) ((char *) curr_block - prev_size);
		int new_size = curr_size + prev_size;
		int i = 0;
		if (prev_block->size_status & 2) {
			i = 2;
		}
		prev_block->size_status = new_size + i; 
		new_footer->size_status = new_size;
	}

	ptr = NULL;
	disp_heap();
	return 0;
} 


/* 
 * Initializes the memory allocator.
 * Called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    

    static int allocated_once = 0; //prevent multiple myInit calls

    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;

    if (0 != allocated_once) {
        fprintf(stderr, 
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

	// Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }

    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;

    return 0;	
} 

/* 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     

    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
            "********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
            "--------------------------------------------------------------------------------\n");

    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;

        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;

        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
                p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);

        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
            "--------------------------------------------------------------------------------\n");
    fprintf(stdout, 
            "********************************************************************************\n");
}
