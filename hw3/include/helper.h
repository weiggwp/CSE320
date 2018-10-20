#include "sfmm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
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
void* buildFreeBlock(sf_header* blockHeaderPtr,size_t blocksize,int new);

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
void addNewPageBlock();

/*
return a block that fits the given payload
adding new page if neccesary
*/
void* getFittingBlock(size_t datasize);

/*
split the given block into two smaller block
one fits the given size. one has the remaining space
*/
void split(sf_header* headerPtr,size_t firstBlocksize);

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
void allocateBlock(sf_header* headerPtr,size_t size);

/*
return ptr to the footer given header ptr
*/
sf_footer* getFooterPtr(sf_header* hp);

/*
update info struct of given block, in both header and footer
param: headerPtr: ptr to update info
       newInfo: info struct, if skip elements with value -1
*/
void updateFreeBlockInfo(sf_header* headerPtr, sf_block_info newInfo);


