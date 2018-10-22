#include "sfmm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ALLOCATED 1
#define FREE 0

#define ALIGNMENT_SZ 16
#define MIN_BLOCK_SZ 32

#define VALID 1
#define INVALID 0

/*
set the allocated bit of given block's info to given value
param: headerPtr: ptr of block to be updated
    allocated: info bit to be set, if 0 set 0, othsewize set 1
*/
void setBlockAllocBit(sf_header* headerPtr,int allocated);
void setBlockPrevAllocBit(sf_header* headerPtr,int allocated);
void setBlockSize(sf_header* headerPtr,size_t size);
void setBlockRequestedSize(sf_header* headerPtr,size_t size);
/*
initialize given prologue: allocated =1, others = 0
both header and footer
*/
void initPrologue(sf_prologue* prologue);
/*
initialize given epilogue: allocated =1, others = 0
epilogue only has header
*/
void initEpilogue(sf_epilogue* epilogue);

/*
initialize heap
initialize prologue and epilogue,
get storage from mem grow, make it the first block and add it to corresponding list
*/
void initHeap();

/*
get List node with exact blocksize
if such node doesnt exist, add it to the olist
*/
sf_free_list_node* getCorrespondListNode(size_t blocksize);

/*
insert block to innerlist by appending it after the head
*/
void insertBlockAtHead(sf_header* blockPtr,sf_header* head);

/*
precondition: blocksize|16
build block at given ptr,
initialize header and footer,
insert into correspondind list
might need addFreeBlock too
*/
void* buildFreeBlock(sf_header* blockHeaderPtr,size_t blocksize);

/*
get ilist with at least size that being asked
return the list node thich contain enough space for payload
return NULL if not found
*/
sf_free_list_node* getListNodeAtLeastSize(size_t datasize);

/*
remove block at given ptr from its list
by redirect blocks pointing to it
*/
void removeBlockFromList(sf_header* header);

/*
allocate a new page of mem and make it into a block
coalcse to get bigger block than a page
*/
void* addNewPageBlock();

/*
return a block that fits the given payload
adding new page if neccesary
*/
void* getFittingBlock(size_t datasize);

/*
split the given block into two smaller block
one fits the given size. one has the remaining space
*/
sf_header* split(sf_header* headerPtr,size_t firstBlocksize);

/*
update info struct of given header or footer
param: headerPtr: ptr to update info
       newInfo: info struct, if skip elements with value -1
*/
void updateInfo(sf_header* headerPtr, sf_block_info newInfo);

/*
Update the bit in next block's prev_allocated to given allocated value
*/
void informNextBlock(sf_header* headerPtr,int allocated);

/*
allocate given block for usage,
update block info to allocated status
updage prev allocated bit in next block
*/
void updateAllocatedBlock(sf_header* headerPtr,size_t size);

/*
return ptr to the footer given header ptr
*/
sf_footer* getFooterPtr(sf_header* hp);

/*
update info struct of given block, in both header and footer
param: headerPtr: ptr to update info
*/
void updateFreeBlockInfo(sf_header* headerPtr, size_t blocksize);

/*
precondition: blocksize|16
param: blockHeaderPtr: ptr to the header of the block
    blocksize: size of block
*/
void* updateFreeBlock(sf_header* blockHeaderPtr,size_t blocksize);


/*************** sf_free functions *******************/
/*
check is given blockPtr is valid
return: 1 if Valid 0 otherwise
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
int validateBlockPtr(sf_header* headerPtr);

/*
If the block_size field is not a multiple of 16 or is less than the
minimum block size of 32 bytes, then INVALID is returned.
*/
int validateBlocksize(size_t blocksize);

/*
check prev and next block
if at least one of them  is free coalesce and return pointer to new block
else return orginal pointer
*/
sf_header* coalesce(sf_header* headerPtr);