#define  _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "cachelab.h"
#include "csim.h"

static int IS_VERBOSE = 0;

cache_t *init_cache(uint32_t set_bits, uint32_t block_bits, uint32_t associativity) {
    cache_t *cache = (cache_t *)calloc(sizeof(cache_t), 1);
    cache->associativity = associativity;
    cache->set_bits = set_bits;
    cache->block_bits = block_bits;

    uint32_t number_of_sets = 1 << set_bits;
    cache->sets = (set_t *)calloc(sizeof(set_t), number_of_sets);
    for (uint32_t i = 0; i < number_of_sets; i++) {
        set_t *current_set = &cache->sets[i];
        current_set->valid = calloc(sizeof(uint8_t), associativity);
        current_set->blocks = calloc(sizeof(uint64_t), associativity);
        current_set->lru = calloc(sizeof(uint64_t), associativity);
        for (uint32_t a = 0; a < associativity; a++) {
            current_set->lru[a] = a;
        }
    }

    return cache;
}

uint32_t get_set_index(cache_t *cache, uint64_t address) {
    address >>= cache->block_bits;
    uint32_t mask = (1 << cache->set_bits) - 1;
    return address & mask;
}

uint64_t get_tag(cache_t *cache, uint64_t address) {
    return address >> (cache->block_bits + cache->set_bits);
}

void update_lru(cache_t *cache, set_t *set, uint32_t index) {
    for (uint32_t i = 0; i < cache->associativity; i++) {
        if (set->lru[i] == index) {
            // we now need to move i to the end
            // i.e. we first move all elements from [i + 1, associativity) to
            // [i, associativity - 1), then place index at lru[associativity]
            for (uint32_t j = i; j < cache->associativity - 1; j++) {
                set->lru[j] = set->lru[j + 1];
            }
            
            set->lru[cache->associativity] = index;
            return;
        }
    }

    printf("failure, expected %i to be in LRU array\n", index);
    exit(EXIT_FAILURE);
}

void access_cache(cache_t *cache, access_t *access, uint64_t address) {
    uint32_t index = get_set_index(cache, address);
    uint64_t tag = get_tag(cache, address);
    set_t *set = &cache->sets[index];
    for (uint32_t i = 0; i < cache->associativity; i++) {
        if (set->blocks[i] == tag && set->valid[i]) {
            // block is in cache, we hit
            access->hit_count++;
            update_lru(cache, set, i);
            return;
        }
    }

    // block is NOT in cache
    
    // first look for a free index, we miss in any case now
    access->miss_count++;
    for (uint32_t i = 0; i < cache->associativity; i++) {
        if (!set->valid[i]) {
            set->valid[i] = true;
            set->blocks[i] = tag;
            update_lru(cache, set, i);
            return;
        }
    }

    // there are no free blocks, evict one, now we also evict in any case
    access->eviction_count++;
    // we should replace the item at index lru[0]
    uint32_t eviction_index = set->lru[0];
    set->blocks[eviction_index] = tag;
    update_lru(cache, set, eviction_index);
}

int main(int argc, char *argv[]) {
    int opt;

    char *tracefile = NULL;
    uint32_t set_bits = 0;
    uint32_t associativity = 0;
    uint32_t block_bits = 0;

    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
            case 's':
                set_bits = atoi(optarg);
                break;
            case 'E':
                associativity = atoi(optarg);
                break;
            case 'b':
                block_bits = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            case 'v':
                IS_VERBOSE = true;
                break;
            case 'h':
                printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
                printf("Options:\n");
                printf("  -h         Print this help message.\n");
                printf("  -v         Optional verbose flag.\n");
                printf("  -s <num>   Number of set index bits.\n");
                printf("  -E <num>   Number of lines per set.\n");
                printf("  -b <num>   Number of block offset bits.\n");
                printf("  -t <file>  Trace file.\n\n");
                printf("Examples:\n");
                printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
                printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (IS_VERBOSE) {
        printf("Running in verbose mode...\n");
        printf("s = %i, E = %i, b = %i\n", set_bits, associativity, block_bits);
        printf("Reading from %s\n", tracefile);
    }

    cache_t *cache = init_cache(set_bits, block_bits, associativity);
    access_t accesses = {0, 0, 0};

    FILE *fp = fopen(tracefile, "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1) {
        char command;
        uint32_t addr;
        uint32_t size;
        if (sscanf(line, " %c %x,%d", &command, &addr, &size) != 3) {
            printf("Invalid line: %s\n", line);
            exit(EXIT_FAILURE);
        }

        printf("%s\n", line);
        switch (command) {
            case 'I':
                break;
            case 'M':
                // M beheaves like L followed by S
                access_cache(cache, &accesses, addr);
                access_cache(cache, &accesses, addr);
                break;
            case 'L':
                // L just acceses the cache once
                access_cache(cache, &accesses, addr);
                break;
            case 'S':
                // S just acceses the cache once
                access_cache(cache, &accesses, addr);
                break;
            default:
                printf("Invalid command %c\n", command);
                exit(EXIT_FAILURE);
        }
    }

    fclose(fp);
    printSummary(accesses.hit_count, accesses.miss_count, accesses.eviction_count);
    return EXIT_SUCCESS;
}
