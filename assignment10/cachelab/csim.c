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

void access_cache(cache_t *cache, access_t *access, uint64_t address) {

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

        access_cache(cache, &accesses, 0);
    }

    fclose(fp);
    printSummary(accesses.hit_count, accesses.miss_count, accesses.eviction_count);
    return EXIT_SUCCESS;
}
