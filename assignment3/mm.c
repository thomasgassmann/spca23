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

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define BLOCK_SIZE(size) (ALIGN(size + 2 * SIZE_T_SIZE))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define BLOCK_META(size, alloc) (size | alloc)
#define BLOCK_META_SIZE sizeof(int)

#define HDRP(bp) ((char *)(bp) - BLOCK_META_SIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - BLOCK_META_SIZE * 2)

#define GET_SIZE(p) (GET(p) & ~(ALIGNMENT - 1))
#define GET_ALLOC(p) (GET(p) & 0x1)

// globals
void *heap_listp;

static void *find_fit(size_t size) {

}

static void *extend_heap(size_t words) {

}

static void place(void *block, size_t size) {

}

void mm_check(void) {
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    int prologue_block_size = BLOCK_SIZE(0);
    if ((heap_listp = mem_sbrk(prologue_block_size)) == (void *)-1) {
        return -1;
    }

    PUT(heap_listp + 0 * BLOCK_META_SIZE, 0); // padding
    PUT(heap_listp + 1 * BLOCK_META_SIZE, BLOCK_META(prologue_block_size, 1)); // prologue header
    PUT(heap_listp + 2 * BLOCK_META_SIZE, BLOCK_META(prologue_block_size, 1)); // prologue footer
    PUT(heap_listp + 3 * BLOCK_META_SIZE, BLOCK_META(0, 1)); // epilogue header

    heap_listp += 2 * BLOCK_META_SIZE; // heap_listp is going to point to the first block content (size 0)

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

    if ((bp = extend_heap(block_size)) == NULL) {
        return NULL;
    }

    place(bp, block_size);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
