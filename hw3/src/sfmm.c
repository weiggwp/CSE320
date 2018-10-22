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

size_t splitable(size_t bsize,size_t rsize){
    size_t firstBsize = ((rsize+sizeof(sf_block_info)-1)/16+1)*16;
    if(firstBsize<32) firstBsize = 32;
    //if blocksize - givensize >=32: splitable
    if(bsize-firstBsize >=32){
        return firstBsize;
    }
    else
        return 0;
}
void *sf_malloc(size_t size) {
    //If size is 0, then NULL is returned without setting sf_errno.
    if(size == 0) return NULL;

    //initialize the free list first, when no heap is allocated for us OR when the list only contain head
    if(sf_mem_start() == sf_mem_end()){
        initHeap();
    }

    //get ptr of a block that fits the payload
    sf_header* freeBlockHeaderPtr =  getFittingBlock(size);
    if(freeBlockHeaderPtr ==NULL)
        return NULL;
    //split the block into two
    //givesize = ((requestedsize-1)/16+1)*16 b/c allignment
    size_t blocksize = freeBlockHeaderPtr->info.block_size <<4;
    size_t givensize;
    if((givensize=splitable(blocksize,size))){
        split(freeBlockHeaderPtr,givensize);
        blocksize = givensize;
    }
    //else: dont

    // ready to return allocated space, update allocated status
    updateAllocatedBlock(freeBlockHeaderPtr,size);


    // printf("%p\n", sf_mem_start() );
    // printf("%p\n", sf_mem_end() );

    void* freespacePtr = ((void*) freeBlockHeaderPtr)+sizeof(sf_block_info);
    return freespacePtr;
}
/*
free an allocated block given pointer to the block
Invalid pointer:
    The pointer is NULL.
    The header of the block is before the end of the prologue, or after the
    beginning of the epilogue.
    The allocated bit in the header or footer is 0
    The block_size field is not a multiple of 16 or is less than the
    minimum block size of 32 bytes.
    NOTE: It is always a multiple of 16

    The requested_size field, plus the size required for the block header,
    is greater than the block_size field.
    If the prev_alloc field is 0, indicating that the previous block is free,
    then the alloc fields of the previous block header and footer should also be 0.

*/
void sf_free(void *pp) {
    void * blockP = pp-sizeof(sf_block_info);
    // if blockPtr is not valid, exit program by calling abort
    if(!validateBlockPtr(blockP))
        abort();
    sf_header* headerPtr = blockP;
    //update block to free status
    updateFreeBlock(headerPtr, headerPtr->info.block_size <<4);
    //coalesce if possible
    headerPtr = coalesce(headerPtr);

    return;
}
/*
My implementation of realloc
1.verify that the pointer and size parameters passed to the function are valid.
*/
void *sf_realloc(void *pp, size_t rsize) {
    void * blockP = pp-sizeof(sf_block_info);
    // if blockPtr is not valid, exit program by calling abort
    if(!validateBlockPtr(blockP))
        abort();
    sf_header* headerPtr = blockP;
    size_t blocksize = headerPtr->info.block_size <<4;
    //realloc to size 0 is same as free
    if(rsize ==0){
        free(pp);
        return NULL;
    }

    void* newpp;
    /*When reallocating to a larger size, always follow these three steps:
        Call sf_malloc to obtain a larger block.
        Call memcpy to copy the data in the block given by the client to the block
        returned by sf_malloc.
        Call sf_free on the block given by the client (coalescing if necessary).
        Return the block given to you by sf_malloc to the client.
    */
    if(rsize >= blocksize){
        newpp = sf_malloc(rsize);
        //FIXME: do i need to check if new block is null, and stop doing the following?
        memcpy(newpp,pp,blocksize);
        sf_free(pp);
        return newpp;

    }
    //Reallocating to a Smaller Size
    else{
        size_t splitSize = splitable(blocksize,rsize);
        //split the original block block
        //two cases for splitting:
        //1. Splitting the returned block results in a splinter. In this case, do not
        // split the block. Leave the splinter in the block, update the header field
        // if necessary, and return the same block back to the caller.
        if(!splitSize)
            headerPtr ->info.requested_size = rsize ;
        // The block can be split without creating a splinter. In this case, split the
        // block and update the block size fields in both headers.  Free the remaining block
        // (i.e. coalesce if possible and insert the block into the head of the correct
        // free list).  Return a pointer to the payload of the smaller block to the caller.
        else{
            sf_header* secBlockPtr =  split(headerPtr,splitSize);
            coalesce(secBlockPtr);
            setBlockRequestedSize(headerPtr,rsize);

        }
        return pp;
    }


    return NULL;
}
