/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "helper.h"


/*
   It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If size is 0, then NULL is returned without setting sf_errno.
 * If size is nonzero, then if the allocation is successful a pointer to a valid region of
 * memory of the requested size is returned.  If the allocation is not successful, then
 * NULL is returned and sf_errno is set to ENOMEM.
 */

void *sf_malloc(size_t size) {
    //If size is 0, then NULL is returned without setting sf_errno.
    if(size == 0) return NULL;

    //initialize the free list first, when no heap is allocated for us OR when the list only contain head
    if(sf_mem_start() == sf_mem_end()){
        initHeap();
    }

    //get ptr of a block that fits the payload
    sf_header* freeBlockHeaderPtr =  getFittingBlock(size);

    //split the block into two
    //givesize = ((requestedsize-1)/16+1)*16 b/c allignment
    size_t blocksize = freeBlockHeaderPtr->info.block_size <<4;
    size_t givensize = ((size+sizeof(sf_block_info)-1)/16+1)*16;
    if(givensize<32) givensize = 32;
    //if blocksize - givensize >=32: split
    if(blocksize-givensize >=32){
        split(freeBlockHeaderPtr,givensize);
        blocksize = givensize;
    }
    //else: dont

    // ready to return allocated space, update allocated status
    allocateBlock(freeBlockHeaderPtr,size);


    // printf("%p\n", sf_mem_start() );
    // printf("%p\n", sf_mem_end() );

    void* freespacePtr = ((void*) freeBlockHeaderPtr)+sizeof(sf_block_info);
    return freespacePtr;
}

void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}
