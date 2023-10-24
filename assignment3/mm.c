/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include "mm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define BLOCK_META(size, alloc) ((int)((int)size | (int)alloc))
#define BLOCK_META_SIZE sizeof(int)
#define NON_SIZE_BIT_MASK (0x7)

// get header or footer from block pointer
#define HDRP(bp) ((char *)(bp) - BLOCK_META_SIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - BLOCK_META_SIZE * 2)

// other useful macros
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// read values from header pointer
#define GET_SIZE(p) (GET(p) & ~NON_SIZE_BIT_MASK)
#define GET_ALLOC(p) (GET(p) & 0x1)

// a block contains header, footer and if not allocated also at least two pointers
#define BLOCK_SIZE(size) (ALIGN(MAX(size, 2 * sizeof(void *)) + 2 * BLOCK_META_SIZE))

#ifdef DEBUG
#define PRINT_DEBUG() mm_check()
#else
#define PRINT_DEBUG()
#endif

// get next/prev block from block pointer
// get size directly from current header
#define NEXT_BLOCK(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
// skip current header and read prev footer
#define PREV_BLOCK(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - BLOCK_META_SIZE * 2))

// get pointers from free block
#define SET_PRED(bp, pred) (*((char **)(bp)) = pred)
#define SET_SUCC(bp, succ) (*(((char **)(bp)) + 1) = succ)
#define PRED_BLOCK(bp) (*((char **)(bp)))
#define SUCC_BLOCK(bp) (*(((char **)(bp)) + 1))

// how many free list there are
#define FREE_LIST_COUNT 128
#define EXACT_BELOW 64
#define LINEAR_BELOW 128
#define LINEAR_STEP_SIZE 0x100

// globals
void *heap_listp; // points to epilogue block
void **free_listp; // points to free lists

static int map_to_free_list_index(size_t size) {
    // sizes are all aligned by ALIGNMENT, make sure smallest block size maps to 0
    size_t base = (size - BLOCK_SIZE(1)) / ALIGNMENT;
    if (base < EXACT_BELOW) {
        return base;
    }

    const size_t min_linear = EXACT_BELOW * ALIGNMENT + BLOCK_SIZE(1);
    const size_t max_linear_excl = LINEAR_STEP_SIZE * (LINEAR_BELOW - EXACT_BELOW) + min_linear;
    const size_t linear_size = LINEAR_BELOW - EXACT_BELOW;
    double frac = (double)(size - min_linear) / (double)(max_linear_excl - min_linear);
    size_t off = (size_t)(frac * linear_size);
    if (off < linear_size) {
        return off + EXACT_BELOW;
    }

    return FREE_LIST_COUNT - 1;
}

static void *find_fit(size_t size) {
    size_t free_list_index = map_to_free_list_index(size);
    for (int i = free_list_index; i < FREE_LIST_COUNT; i++) {
        void *list = free_listp[i];
        if (list == NULL) {
            continue;
        }

        // stop at epilogue block or if free list has no more items
        for (void *current = list; current != NULL && GET_SIZE(HDRP(current)) > 0; current = SUCC_BLOCK(current)) {
            assert(!GET_ALLOC(HDRP(current)));
            if (GET_SIZE(HDRP(current)) >= size) {
                return current;
            }
        }
    }

    return NULL;
}

static void remove_free_block_from_list(char *bp) {
    assert(!GET_ALLOC(HDRP(bp)));

    // we expect blocks to be removed and readded if size changes!
    size_t size = GET_SIZE(HDRP(bp));
    size_t free_list_index = map_to_free_list_index(size);

    char *pred = PRED_BLOCK(bp);
    char *succ = SUCC_BLOCK(bp);
    if (pred == NULL && succ == NULL) {
        // this is the only block
        free_listp[free_list_index] = NULL;
        return;
    }

    if (pred == NULL) {
        // this is the first block
        SET_PRED(succ, NULL);
        free_listp[free_list_index] = succ;
        return;
    }

    if (succ == NULL) {
        // this is the last block
        SET_SUCC(pred, NULL);
        return;
    }

    // any block in the list
    SET_SUCC(pred, succ);
    SET_PRED(succ, pred);
}

static void add_free_block_to_list(char *bp) {
    assert(!GET_ALLOC(HDRP(bp)));

    // we expect blocks to be removed and readded if size changes!
    size_t size = GET_SIZE(HDRP(bp));
    size_t free_list_index = map_to_free_list_index(size);

    SET_PRED(bp, NULL);
    SET_SUCC(bp, free_listp[free_list_index]);
    if (free_listp[free_list_index] != NULL) {
        SET_PRED(free_listp[free_list_index], bp);
    }

    free_listp[free_list_index] = bp;
}

static void *coalesce(char *bp) {
    size_t is_prev_alloc = GET_ALLOC(FTRP(PREV_BLOCK(bp)));
    size_t is_next_alloc = GET_ALLOC(HDRP(NEXT_BLOCK(bp)));
    if (is_prev_alloc && is_next_alloc) {
        // nothing to coalesce
        return bp;
    }

    size_t current_block_size = GET_SIZE(HDRP(bp));
    if (is_prev_alloc && !is_next_alloc) {
        // remove next from free list
        remove_free_block_from_list(NEXT_BLOCK(bp));
        remove_free_block_from_list(bp);

        // merge current with next one
        size_t new_block_size = current_block_size + GET_SIZE(HDRP(NEXT_BLOCK(bp)));
        PUT(HDRP(bp), BLOCK_META(new_block_size, 0));
        // works because header was already updated
        // update footer only after header in this case
        PUT(FTRP(bp), BLOCK_META(new_block_size, 0));

        add_free_block_to_list(bp);
        return bp;
    }

    if (!is_prev_alloc && is_next_alloc) {
        // remove current from free list
        remove_free_block_from_list(bp);
        remove_free_block_from_list(PREV_BLOCK(bp));

        // merge current with prev one
        size_t new_block_size = current_block_size + GET_SIZE(HDRP(PREV_BLOCK(bp)));
        PUT(HDRP(PREV_BLOCK(bp)), BLOCK_META(new_block_size, 0));
        PUT(FTRP(bp), BLOCK_META(new_block_size, 0));

        add_free_block_to_list(PREV_BLOCK(bp));
        return PREV_BLOCK(bp);
    }

    // remove current and next from free list
    remove_free_block_from_list(bp);
    remove_free_block_from_list(NEXT_BLOCK(bp));
    remove_free_block_from_list(PREV_BLOCK(bp));

    size_t new_block_size = current_block_size
        + GET_SIZE(HDRP(NEXT_BLOCK(bp)))
        + GET_SIZE(HDRP(PREV_BLOCK(bp)));

    PUT(HDRP(PREV_BLOCK(bp)), BLOCK_META(new_block_size, 0));
    PUT(FTRP(NEXT_BLOCK(bp)), BLOCK_META(new_block_size, 0));

    add_free_block_to_list(PREV_BLOCK(bp));
    return PREV_BLOCK(bp);
}

// number of bytes we want to extend the heap, not number of bytes
static void *extend_heap(size_t bytes) {
    size_t effective_size = ALIGN(bytes);
    char *bp;
    if ((long)(bp = mem_sbrk(effective_size)) == -1) {
        return NULL;
    }

    // this overwrites the current epilogue header
    // the size of the new block stretches until just
    // before the new epilogue block
    PUT(HDRP(bp), BLOCK_META(effective_size, 0));

    PUT(FTRP(bp), BLOCK_META(effective_size, 0));

    add_free_block_to_list(bp);

    // set new epilogue header
    PUT(HDRP(NEXT_BLOCK(bp)), BLOCK_META(0, 1));

    // previous block might have been free, coalesce them
    return coalesce(bp);
}

static void place(void *block, size_t size) {
    size_t current_size = GET_SIZE(HDRP(block));
    size_t min_block_size = BLOCK_SIZE(1);

    if (!GET_ALLOC(HDRP(block))) {
        remove_free_block_from_list(block);
    }

    if (current_size - size >= min_block_size) {
        // there is enough space to fit a new block of size min_block_size
        // we assume size is aligned to 8 bytes
        PUT(HDRP(block), BLOCK_META(size, 1));
        PUT(FTRP(block), BLOCK_META(size, 1));
        char *next_block = NEXT_BLOCK(block);
        PUT(HDRP(next_block), BLOCK_META((current_size - size), 0));
        PUT(FTRP(next_block), BLOCK_META((current_size - size), 0));

        add_free_block_to_list(next_block);

        return;
    }

    PUT(HDRP(block), BLOCK_META(current_size, 1));
    PUT(FTRP(block), BLOCK_META(current_size, 1));
}

void mm_check() {
    int i = 0;
    char *current;
    printf("\n\n--- CURRENT BLOCKS ---\n");
    for (current = heap_listp; GET_SIZE(HDRP(current)) > 0; current = NEXT_BLOCK(current)) {
        char *header_p = HDRP(current);
        printf("block %d(size=0x%x, addr=%p, alloc=%d)\n", i, GET_SIZE(header_p), current, GET_ALLOC(header_p));

        i++;
    }

    printf("epilogue block(size=0x%x, addr=%p, alloc=%d)\n", GET_SIZE(HDRP(current)), current, GET_ALLOC(HDRP(current)));
    printf("Found a total of %d blocks\n", i + 1);

    printf("\n--- FREE BLOCKS ---\n");
    for (int i = 0; i < FREE_LIST_COUNT; i++) {
        if (free_listp[i] != NULL) {
            printf("--- starting list %d ---\n", i);
        }

        int j = 0;
        for (current = free_listp[i]; current != NULL; current = SUCC_BLOCK(current)) {
            char *header_p = HDRP(current);

            unsigned int current_size = GET_SIZE(header_p);
            size_t expected_index = map_to_free_list_index(current_size);
            assert(i == expected_index);

            printf("block %d(size=0x%x, addr=%p, alloc=%d)\n", j++, current_size, current, GET_ALLOC(header_p));
        }
    }
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    heap_listp = NULL;

    // init free lists
    if ((free_listp = mem_sbrk(sizeof(void *) * FREE_LIST_COUNT)) == (void *)-1) {
        return -1;
    }

    for (int i = 0; i < FREE_LIST_COUNT; i++) {
        free_listp[i] = NULL;
    }

    // prologue and epilogue block do not contain pointers
    int prologue_block_size = 2 * BLOCK_META_SIZE;
    if ((heap_listp = mem_sbrk(2 * prologue_block_size)) == (void *)-1) {
        return -1;
    }

    PUT(heap_listp + 0 * BLOCK_META_SIZE, 0); // padding
    PUT(heap_listp + 1 * BLOCK_META_SIZE, BLOCK_META(prologue_block_size, 1)); // prologue header
    PUT(heap_listp + 2 * BLOCK_META_SIZE, BLOCK_META(prologue_block_size, 1)); // prologue footer
    PUT(heap_listp + 3 * BLOCK_META_SIZE, BLOCK_META(0, 1)); // epilogue header

    heap_listp += 2 * BLOCK_META_SIZE; // heap_listp is going to point to the first block content (size 0)
    if (extend_heap(mem_pagesize()) == NULL) {
        return -1;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t block_size = BLOCK_SIZE(size);
    char *bp;
    if ((bp = find_fit(block_size)) != NULL) {
        place(bp, block_size);
        return bp;
    }

    if ((bp = extend_heap(MAX(block_size, mem_pagesize()))) == NULL) {
        return NULL;
    }

    place(bp, block_size);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), BLOCK_META(size, 0));
    PUT(FTRP(ptr), BLOCK_META(size, 0));
    add_free_block_to_list(ptr);

    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    size_t block_size = BLOCK_SIZE(size);
    size_t current_size = GET_SIZE(HDRP(ptr));
    if (current_size >= block_size) {
        place(ptr, block_size);
        return ptr;
    }

    size_t c = current_size;
    char *current_block = ptr;
    char *stop = NULL;
    while (1) {
        char *next = NEXT_BLOCK(current_block);
        if (GET_ALLOC(HDRP(next)) || GET_SIZE(HDRP(next)) == 0) {
            // if we can't find continuous sequence of unallocated blocks, stop
            // if epilogue block, stop
            break;
        }

        c += GET_SIZE(HDRP(next));
        if (c >= block_size) {
            // merge all blocks from ptr until next
            stop = next;
            break;
        }

        current_block = next;
    }

    if (stop != NULL) {
        // merge all blocks from ptr to stop
        current_block = NEXT_BLOCK(ptr);
        while (current_block != stop) {
            remove_free_block_from_list(current_block);
        }

        remove_free_block_from_list(stop);

        PUT(FTRP(stop), BLOCK_META(c, 1));
        PUT(HDRP(ptr), BLOCK_META(c, 1));

        // split next block if possible, TODO: why does this decrease utilization???
        // place(ptr, block_size);
        return ptr;
    }

    void *newptr = mm_malloc(size);
    if (newptr == NULL) {
        return NULL;
    }

    size_t copySize = MIN(current_size - 2 * BLOCK_META_SIZE, size);
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}
