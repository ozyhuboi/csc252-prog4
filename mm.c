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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Y-team",
    /* First member's full name */
    "Yibo Zhou",
    /* First member's email address */
    "yzhou22@u.rochester.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* 16 byte alignment */
#define ALIGNMENT 16
#define CHUNK (1<<12)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT) - 1) & ~(ALIGNMENT- 1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// Inspired by 252 Perspective pg. 830
#define MAX(x, y) ((x) > (y)? (x) : (y) )

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) 		(*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p*/
#define GET_SIZE(p) (GET(p) & ~0x7 )
#define GET_ALLOC(p) (GET(p) & 0x1 )

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - 4)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 8)

/* Given block ptr bp, compute address of next and previous blocks, these connect the list */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - 4)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - 8)))

static char *heap_listp = 0; // Pointer to first block

/* 
 * mm_init - initialize the malloc package.
 * Inspired by 252 Perspective pg. 831
 */
int mm_init(void) {

	/* Create the intial heap*/

	if ((heap_listp = mem_sbrk(ALIGNMENT)) == (void*) - 1) {
		return -1;
	}

	PUT(heap_listp, 0); // Alignment padding
	PUT(heap_listp + 4, PACK(8, 1));//Prologue header
	PUT(heap_listp + 8, PACK(8, 1)); 	//Prologue footer
	PUT(heap_listp + 12, PACK(0, 1));	//Epilogue header
	heap_listp += 8;


	if (extend_heap(CHUNK/4) == NULL) {
		return -1; // Extend empty heap with CHUNK bytes
	}

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 *
 *     Inspired by 252 Perspective pg. 834
 */
void *mm_malloc(size_t size) {

    int newsize = ALIGN(size + SIZE_T_SIZE);
    //void *p = mem_sbrk(newsize);
    int extend;
    void *p;

    //Ignore sizes of 0
    if (size == 0) {
    	return NULL;
    }

    /* Search free list for a fit*/
    if ((p = find_fit(newsize)) != NULL) {

    	place(p, newsize );
    	return p;
    }

    /* If no fit found, get more memory and place block */
    extend = MAX(newsize, CHUNK);
    if ((p = extend_heap(extend/4)) == NULL) {
    	return NULL; // If can't extend, then don't
    }

    place(p, newsize);
    return p;
}

/*
 * mm_free - Freeing a block does nothing.
 * Inspired by 252 Perspective pg. 833
 */
void mm_free(void *ptr) {

	size_t size = GET_SIZE( HDRP(ptr) );

	if (heap_listp == 0) {
		mm_init();
	}
	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	coalesce(ptr);

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL) {
      return NULL;
    }

    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize) {
      copySize = size;
    }

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    return newptr;
}

/*
 * mm_check - heap consistency checker
 * http://www.coe.utah.edu/~trobinso/mm.c
 * http://www.smigit.com/wp-content/uploads/2006/06/mm.c
 * http://csapp.cs.cmu.edu/2e/ics2/code/vm/malloc/mm.c
 */
int mm_check (void) {


	return 1; // All checks pass
}

/*
 * Other helper functions used above.
 */

/*
 * Increase the size of the heap when called
 */
static void *extend_heap(size_t word) {
	char *bp;
	size_t size;

	size = (word % 2) ? (word+1) * 4 : word * 4;
	if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
}

/*
 * Search for blocks of right size to place into, then return pointer to location
 */

static void *find_fit(size_t asize) {
	void *p

	for (p = heap_listp; GET_SIZE(HDRP(p)) > 0; p = NEXT_BLKP(p)) {
		if (!GET_ALLOC(HDRP(p)) && (asize <= GET_SIZE(HDRP(p)))) {
			return p;
		}
	}
}

/*
 * Place newly allocated block
 */

static void place(9void *bp, size_t asize) {
	size_t csize = GET_SIZE(HDRP(bp));

	if ((csize - asize) >= (16)) {

		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(csize - asize, 0));
		PUT(FTRP(bp), PACK(csize - asize, 0));

	} else {
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
	}
}

/*
 * Coalesce the free blocks to be large enough for newly allocated memory
 */

static void *coalesce(void *bp) {

	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(FTRP(NEXT_BLKP(bp)));
	size_t seize = GET_ALLOC(HDRP(bp));

	// Case 1
	if (prev_alloc && next_alloc ) {
		return bp;
	} else {
		// Case 2
		if (prev_alloc && !next_alloc) {
			size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
			PUT(HDRP(bp), PACK(size, 0));
			PUT(FTRP(bp), PACK(size, 0));
		} else {
			// Case 3
			if (!prev_alloc && next_alloc) {
				size += GET_SIZE(HDRP(PREV_BLKP(bp)));
				PUT(FTRP(bp), PACK(size, 0));
				PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
				bp = PREV_BLKP(bp);
			} else {
				// Case 4
				size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXST_BLKP(bp)));
				PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
				PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
				bp = PREV_BLKP(bp);
			}

		}
		return bp;
	}
}
