////////////////////////////////////////////////////////////////////////////////
// Main File:        N/A
// This File:        p3Heap.c
// Other Files:      .gitignore, Makefile, p3Heap.h
// Semester:         CS 354 Lecture 10 Spring 2025
// Instructor/TA:    deppeler, asch
//
// Author:           Michael Hu
// UW NetID:         mhu97
// CS Login:         mhu
//
//////////////////// REQUIRED -- OTHER SOURCES OF HELP /////////////////////////
// Persons:          N/A
//
// Online sources:
// https://stackoverflow.com/questions/1217691/c-function-returning-via-void
// https://en.wikipedia.org/wiki/Bitwise_operations_in_C
//
// AI chats:         Prompt - "Explain all bitwise operations generically as if
//                             a 5 year old could understand, then explain them
//                             specific to behavior in C. Spend extra time on
//                              "|", "&", and "~" and provide discrete, simple
//                             examples"
//////////////////////////// 80 columns wide ///////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2025 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes.
// Used by permission SPRING 2025, CS354-deppeler
//
/////////////////////////////////////////////////////////////////////////////

#include "p3Heap.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
     * Status is stored using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit
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
     * Examples:
     *
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     *
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
    int size_status;

} blockHeader;

/* Global variable - DO NOT CHANGE NAME or TYPE.
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader* heap_start = NULL;

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */

int HEADER_SIZE = 4;
int FOOTER_SIZE = 4;
int MIN_BLOCK_SIZE = 8;
int ALIGNMENT = 8;

/*
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 1
 * - Determine block size rounding up to a multiple of 8
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements
 *       - Update all heap block header(s) and footer(s)
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* alloc(int size) {
    if (size < 1) return NULL;

    // allocated size should be nearest multiple of 8, including header
    int block_size =
        ((size + HEADER_SIZE + (ALIGNMENT - 1)) / ALIGNMENT) * ALIGNMENT;

    blockHeader* best_fit = NULL;
    blockHeader* ptr = heap_start;
    int best_size = 0;

    // find the best-fit block that's free
    while (ptr != NULL && (ptr->size_status & ~0x7) != 0) {
        int curr_size = ptr->size_status & ~0x7;
        int curr_alloc = ptr->size_status & 0x1;

        if (!curr_alloc && curr_size >= block_size) {
            if (best_fit == NULL || curr_size < best_size) {
                best_fit = ptr;
                best_size = curr_size;
            }
        }

        ptr = (blockHeader*)((char*)ptr + curr_size);
    }

    if (!best_fit) return NULL;

    // otherwise we have a result, determine if we can split it or not
    int remaining_size = best_size - block_size;
    int p_bit = best_fit->size_status & 0x2;

    // return "exact" matches
    if (remaining_size < MIN_BLOCK_SIZE) {
        best_fit->size_status = best_size | p_bit | 0x1;

        blockHeader* next = (blockHeader*)((char*)best_fit + best_size);

        // only update the p-bit we're not at the end mark
        if (next->size_status != 1) next->size_status |= 0x2;

        return (blockHeader*)((char*)best_fit + HEADER_SIZE);
    }

    // otherwise we can split it
    best_fit->size_status = block_size | p_bit | 0x1;
    blockHeader* new_free_block = (blockHeader*)((char*)best_fit + block_size);
    new_free_block->size_status = remaining_size | 0X2;

    blockHeader* footer =
        (blockHeader*)((char*)new_free_block + remaining_size - FOOTER_SIZE);
    footer->size_status = remaining_size;

    blockHeader* next = (blockHeader*)((char*)best_fit + best_size);
    // only update the p-bit we're not at the end mark
    if (next->size_status != 1) next->size_status &= ~0x2;

    return (void*)((char*)best_fit + HEADER_SIZE);
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
 * so free blocks require a footer (blockHeader works) to store the size
 *
 * TIP: work on getting immediate coalescing to work after your code
 *      can pass the tests in partA and partB of tests/ directory.
 *      Submit code that passes partA and partB to Canvas before continuing.
 */
int free_block(void* ptr) {
    if (ptr == NULL) return -1;

    if ((unsigned long)ptr % ALIGNMENT != 0) return -1;

    blockHeader* end_mark = (blockHeader*)((char*)heap_start + alloc_size);

    if ((char*)ptr < (char*)heap_start + HEADER_SIZE ||
        (char*)ptr >= (char*)end_mark) {
        return -1;
    }

    blockHeader* block = (blockHeader*)((char*)ptr - HEADER_SIZE);
    int size_status = block->size_status;

    // a-bit = (size_status & 0x1)
    if ((size_status & 0x1) == 0) return -1;

    // clear a-bit, save p-bit if it was set
    int p_bit = size_status & 0x2;
    int block_size = size_status & ~0x7;
    block->size_status = (block_size | p_bit);

    blockHeader* footer =
        (blockHeader*)((char*)block + block_size - FOOTER_SIZE);
    footer->size_status = block_size;

    blockHeader* next = (blockHeader*)((char*)block + block_size);
    if (next->size_status != 1) {
        // clear p-bit
        next->size_status &= ~0x2;
    }

    // coalesce with next if it's free + not end
    if (next->size_status != 1 && ((next->size_status & 0x1) == 0)) {
        int next_size = next->size_status & ~0x7;
        int total_size = block_size + next_size;

        block->size_status = (block->size_status & 0x2) | total_size;
        footer = (blockHeader*)((char*)block + total_size - FOOTER_SIZE);
        footer->size_status = total_size;

        blockHeader* next_next = (blockHeader*)((char*)next + next_size);

        if (next_next->size_status != 1) next_next->size_status &= ~0x2;

        block_size = total_size;
    }

    // coalesce with previous block if it's free
    if ((block->size_status & 0x2) == 0) {
        blockHeader* prev_footer = (blockHeader*)((char*)block - FOOTER_SIZE);
        int prev_size = prev_footer->size_status;

        blockHeader* prev_header = (blockHeader*)((char*)block - prev_size);
        int combined_size = prev_size + block_size;

        // clear combined a-bit, save p-bit from prev_header->size_status
        int prev_p_bit = (prev_header->size_status & 0x2);
        prev_header->size_status = (combined_size | prev_p_bit);

        footer =
            (blockHeader*)((char*)prev_header + combined_size - FOOTER_SIZE);
        footer->size_status = combined_size;

        blockHeader* next_block =
            (blockHeader*)((char*)prev_header + combined_size);

        if (next_block->size_status != 1) next_block->size_status &= ~0x2;
    }

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
    static int allocated_once = 0;  // prevent multiple myInit calls

    int pagesize;  // page size
    int padsize;   // size of padding when heap size not a multiple of page size
    void* mmap_ptr;  // pointer to memory mapped area
    int fd;

    blockHeader* end_mark;

    if (0 != allocated_once) {
        fprintf(stderr,
                "Error:mem.c: InitHeap has allocated space during a previous "
                "call\n");
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
    mmap_ptr =
        mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
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
    heap_start = (blockHeader*)mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader* footer = (blockHeader*)((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;

    return 0;
}

/* STUDENTS MAY EDIT THIS FUNCTION, but do not change function header.
 * TIP: review this implementation to see one way to traverse through
 *      the blocks in the heap.
 *
 * Can be used for DEBUGGING to help you visualize your heap structure.
 * It traverses heap blocks and prints info about each block found.
 *
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts)
 * t_End    : address of the last byte in the block
 * t_Size   : size of the block as stored in the block header
 */
void disp_heap() {
    int counter;
    char status[6];
    char p_status[6];
    char* t_begin = NULL;
    char* t_end = NULL;
    int t_size;

    blockHeader* current = heap_start;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used = -1;

    fprintf(stdout,
            "********************************** HEAP: Block List "
            "****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout,
            "------------------------------------------------------------------"
            "--------------\n");

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
                p_status, (unsigned long int)t_begin, (unsigned long int)t_end,
                t_size);

        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout,
            "------------------------------------------------------------------"
            "--------------\n");
    fprintf(stdout,
            "******************************************************************"
            "**************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout,
            "******************************************************************"
            "**************\n");
    fflush(stdout);

    return;
}

//		p3Heap.c (SP25)
