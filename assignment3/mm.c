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

// globals
void *heap_listp;

static void *find_fit(size_t size) {
    for (void *current = heap_listp; GET_SIZE(HDRP(current)) > 0; current = NEXT_BLOCK(current)) {
        if (!GET_ALLOC(HDRP(current)) && GET_SIZE(HDRP(current)) >= size) {
            return current;
        }
    }

    return NULL;
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
        // merge current with next one
        size_t new_block_size = current_block_size + GET_SIZE(HDRP(NEXT_BLOCK(bp)));
        PUT(HDRP(bp), BLOCK_META(new_block_size, 0));
        // works because header was already updated
        // update footer only after header in this case
        PUT(FTRP(bp), BLOCK_META(new_block_size, 0));
        return bp;
    }

    if (!is_prev_alloc && is_next_alloc) {
        // merge current with prev one
        size_t new_block_size = current_block_size + GET_SIZE(HDRP(PREV_BLOCK(bp)));
        PUT(HDRP(PREV_BLOCK(bp)), BLOCK_META(new_block_size, 0));
        PUT(FTRP(bp), BLOCK_META(new_block_size, 0));
        return PREV_BLOCK(bp);
    }

    size_t new_block_size = current_block_size
        + GET_SIZE(HDRP(NEXT_BLOCK(bp)))
        + GET_SIZE(HDRP(PREV_BLOCK(bp)));

    PUT(HDRP(PREV_BLOCK(bp)), BLOCK_META(new_block_size, 0));
    PUT(FTRP(NEXT_BLOCK(bp)), BLOCK_META(new_block_size, 0));

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
    PUT(HDRP(NEXT_BLOCK(bp)), BLOCK_META(0, 1));

    // previous block might have been free, coalesce them
    return coalesce(bp);
}

static void place(void *block, size_t size) {
    size_t current_size = GET_SIZE(HDRP(block));
    size_t min_block_size = BLOCK_SIZE(1);
    if (current_size - size >= min_block_size) {
        // there is enough space to fit a new block of size min_block_size
        // we assume size is aligned to 8 bytes
        PUT(HDRP(block), BLOCK_META(size, 1));
        PUT(FTRP(block), BLOCK_META(size, 1));
        char *next_block = NEXT_BLOCK(block);
        PUT(HDRP(next_block), BLOCK_META((current_size - size), 0));
        PUT(FTRP(next_block), BLOCK_META((current_size - size), 0));
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
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
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

    PRINT_DEBUG();
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    PRINT_DEBUG();
    if (size == 0) {
        return NULL;
    }

    size_t block_size = BLOCK_SIZE(size);
    char *bp;
    if ((bp = find_fit(block_size)) != NULL) {
        place(bp, block_size);
        PRINT_DEBUG();
        return bp;
    }

    if ((bp = extend_heap(MAX(block_size, mem_pagesize()))) == NULL) {
        PRINT_DEBUG();
        return NULL;
    }

    place(bp, block_size);
    PRINT_DEBUG();
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    PRINT_DEBUG();
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), BLOCK_META(size, 0));
    PUT(FTRP(ptr), BLOCK_META(size, 0));

    coalesce(ptr);
    PRINT_DEBUG();
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
            PUT(FTRP(next), BLOCK_META(c, 1));
            PUT(HDRP(ptr), BLOCK_META(c, 1));
            return ptr;
        }

        current_block = next;
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
