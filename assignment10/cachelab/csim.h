#ifndef CACHELAB_SIM_H
#define CACHELAB_SIM_H

#include <stdint.h>

typedef struct {
    uint32_t hit_count;
    uint32_t miss_count;
    uint32_t eviction_count;
} access_t;

typedef struct {
    uint8_t *valid; // valid[i] is valid is block i has a value
    uint64_t *blocks; // blocks[i] contains the tag of block i in this cache
    uint64_t *lru; // lru[i] is the i-th least recently accessed block index, i.e. lru[0] is going to be replaced next
} set_t;

typedef struct {
    uint32_t set_bits;
    uint32_t block_bits;
    uint32_t associativity;
    set_t *sets;
} cache_t;

cache_t *init_cache(uint32_t set_bits, uint32_t block_bits, uint32_t associativity);
void access_cache(cache_t *cache, access_t *access, uint64_t address);
uint32_t get_set_index(cache_t *cache, uint64_t address);
uint64_t get_tag(cache_t *cache, uint64_t address);
void update_lru(cache_t *cache, set_t *set, uint32_t index);

#endif
