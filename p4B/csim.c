////////////////////////////////////////////////////////////////////////////////
// Main File:        csim.c
// This File:        csim.c
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
// Online sources:   https://stackoverflow.com/questions/7559531/javascript-typescript-switch-statement-way-to-run-same-code-for-two-cases
//                   ^ used in replay_trace
// 
// AI chats:         N/Aß
//////////////////////////// 80 columns wide ///////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Copyright 2013,2019-2025
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission for Spring 2025
////////////////////////////////////////////////////////////////////////////////

/**
 * csim.c:  
 * Simulatate the contents of a cache with given configuration and 
 * count the number of hits, misses, and evictions for a given 
 * sequence of memory accesses for a program.
 *
 * If you want to create your own memory access traces, you can 
 * use valgrind to output traces that this simulator can analyze.
 *
 * What is the replacement policy implemented?  ___________________________
 *
 * Implementation and assumptions:
 *  1. (L) load or (S) store cause at most one cache miss and a possible eviction.
 *  2. (I) Instruction loads are ignored.
 *  3. (M) Data modify is treated as a load followed by a store to the same
 *     address. Hence, an (M) operation can result in two cache hits, 
 *     or a miss and a hit plus a possible eviction.
 *
 * Find the TODO tags to see where to make chanes for your simulator.
 */  

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/******************************************************************************/
/* DO NOT MODIFY THESE VARIABLE NAMES and TYPES                               */
/* DO UPDATE THEIR VALUES AS NEEDED BY YOUR CSIM                              */

//Globals set by command line args.
int b = 0; //number of (b) bits
int s = 0; //number of (s) bits
int E = 0; //number of lines per set

//Globals derived from command line args.
int B; //block size in bytes: B = 2^b
int S; //number of sets: S = 2^s

//Global counters to track cache statistics in access_data().
int hit_cnt = 0;
int miss_cnt = 0;
int evict_cnt = 0;

//Global to control trace output
int verbosity = 0; //print trace if set
/******************************************************************************/


// Type mem_addr_t: stores an addresses or address masks.
typedef unsigned long long int mem_addr_t;

// Type cache_line_t: stores "overhead" (v-bit and tag) for a cache line
// The cache block's data is not needed or stored for this simulator.
typedef struct cache_line {                    
    char valid;
    mem_addr_t tag;
    int lru_counter;
} cache_line_t;

// Type cache_set_t: Stores a pointer to the first cache line in a set.
// Note: Each set is a pointer to a heap array of one or more cache lines.
typedef cache_line_t* cache_set_t;

// Type cache_t: Stores a pointer to the first set in a cache
// Note: The cache is a pointer to a heap array of one or more sets.
typedef cache_set_t* cache_t;

// Create the cache we're simulating. 
// Note: The cache is a pointer to a heap array of one or more sets.
cache_t cache;  

/*
 * init_cache:
 * Allocates the data structure for a cache with S sets and E lines per set.
 * Initializes all valid bits and tags with 0s.
 */                    
void init_cache() {         
     S = 1 << s;
     B = 1 << b;

     cache = (cache_t)malloc(S * sizeof(cache_set_t));

     if (!cache) {
        printf("Failed to allocated cache sets\n");
        exit(1);
     }

     for (int i = 0; i < S; i++) {
        cache[i] = (cache_set_t)malloc(E * sizeof(cache_line_t));
        if (!cache[i]) {
            printf("Failed to allocated cache line\n");
            exit(1);
        }

        for (int j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru_counter = 0;
        }
     }
}


/*
 * free_cache:
 * Frees all heap allocated memory used by the cache.
 */                    
void free_cache() {        
    for (int i = 0; i < S; i++) {
        free(cache[i]);
    }     
    free(cache);
}



/*
 * access_data:
 * Simulates data access at given "addr" memory address in the cache.
 *
 * If block containing addr is already in cache, increment hit_cnt
 * If block containing addr is not in cache, 
 *    cache it (set tag and valid), increment miss_cnt
 * If a block in a cache line is evicted, increment evict_cnt
 * 
 * For my own reference, this is the structure of an address
 * | tag |  set index | block offset |
 *         ^          ^             ^
 *         s bits     b bits
 */                    
void access_data(mem_addr_t addr) {    
    // lowest s bits out of (addr without offset bits, s bits)
    mem_addr_t index = (addr >> b) & ((1 << s) - 1); 
    mem_addr_t tag = addr >> (s + b);
    cache_set_t set = cache[index];

    int line = -1, empty = -1, min = INT_MAX, lru_index = -1;

    // LRU cache sim
    // look for a hit or candidate for LRU
    for (int i = 0; i < E; i++) {
        if (set[i].valid && set[i].tag == tag) {
            line = i; // hit!
            break;
        }
        if (!set[i].valid && empty == -1) {
            empty = i;
        }
        if (set[i].valid && set[i].lru_counter < min) {
            min = set[i].lru_counter;
            lru_index = i;
        }
    }

    static int global_lru_tracker = 0;
    global_lru_tracker += 1;

    // hit!
    if (line != -1) {
        hit_cnt += 1;
        set[line].lru_counter = global_lru_tracker;
        if (verbosity) printf("hit ");
    }
    // miss!
    else {
        miss_cnt += 1;
        if (verbosity) printf("miss ");
        int eviction_index = (empty != -1) ? empty : lru_index;
        if (set[eviction_index].valid) {
            evict_cnt += 1;
            if (verbosity) printf("eviction ");
        }

        set[eviction_index].valid = 1;
        set[eviction_index].tag = tag;
        set[eviction_index].lru_counter = global_lru_tracker;
    }

}


/* TODO - FILL IN THE MISSING CODE
 * replay_trace:
 * Replays the given trace file against the cache.

 Example: subset of trace, shows type of access, address, size of access

 L 7ff0005b8,4
 S 7feff03ac,4
 M 7fefe059c,4
 
 * Reads the input trace file line by line.
 * Extracts the type of each memory access : L/S/M
 * TRANSLATE "L" as a Load i.e. one memory access
 * TRANSLATE "S" as a Store i.e. one memory access
 * TRANSLATE "M" as a Modify which is a load followed by a store, 2 mem accesses 
 */                    
void replay_trace(char* trace_fn) {           
    char buf[1000];  
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE* trace_fp = fopen(trace_fn, "r"); 

    if (!trace_fp) { 
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);   
    }

    while (fgets(buf, 1000, trace_fp) != NULL) { 
        if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);

            if (verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            switch (buf[1]) {
                // one access for store and load
                case 'S':
                case 'L':
                    access_data(addr);
                    break;
                case 'M': // load followed by store
                    access_data(addr);
                    access_data(addr);
                    break;

            }

            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}  


/*
 * print_usage:
 * Print information on how to use csim to standard output.
 */                    
void print_usage(char* argv[]) {                 
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Verbose flag.\n");
    printf("  -s <num>   Number of s bits for set index.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of b bits for word and byte offsets.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}  


/*
 * print_summary:
 * Prints a summary of the cache simulation statistics to a file.
 */                    
void print_summary(int hits, int misses, int evictions) {                
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}  


/*
 * main:
 * parses command line args, 
 * makes the cache, 
 * replays the memory accesses, 
 * frees the cache and 
 * prints the summary statistics.  
 */                    
int main(int argc, char* argv[]) {                      
    char* trace_file = NULL;
    char c;

    // Parse the command line arguments: -h, -v, -s, -E, -b, -t 
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (c) {
            case 'b':
                b = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'h':
                print_usage(argv);
                exit(0);
            case 's':
                s = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            default:
                print_usage(argv);
                exit(1);
        }
    }

    // Make sure that all required command line args were specified.
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        print_usage(argv);
        exit(1);
    }

    // Initialize cache.
    init_cache();

    // Replay the memory access trace.
    replay_trace(trace_file);

    // Free memory allocated for cache.
    free_cache();

    // Print the statistics to a file.
    // DO NOT REMOVE: This function must be called for test_csim to work.
    print_summary(hit_cnt, miss_cnt, evict_cnt);
    return 0;   
}  

/*202501*/                                     

