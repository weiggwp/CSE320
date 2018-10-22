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
void initPrologue(sf_prologue* prologue){
    prologue -> padding =0;

    //init header
    sf_header* header = & prologue->header;
    sf_block_info* info= &header->info;
    info->allocated = ALLOCATED;
    info->prev_allocated = 0;
    info->two_zeroes = 0;
    info->block_size = 0;
    info->requested_size = 0;

    //init footer
    sf_footer* footer = &prologue->footer;
    info= &footer->info;
    info->allocated = ALLOCATED;
    info->prev_allocated = 0;
    info->two_zeroes = 0;
    info->block_size = 0;
    info->requested_size = 0;

}
void initEpilogue(sf_epilogue* epilogue){
    //init footer
    sf_footer* footer = &epilogue->footer;
    sf_block_info* info= &footer->info;
    info->allocated = ALLOCATED;
    info->prev_allocated = FREE;
    info->two_zeroes = 0;
    info->block_size = 0;
    info->requested_size = 0;

}

sf_free_list_node* getCorrespondListNode(size_t blocksize){
    //block must be multiple of 16
    if(blocksize%ALIGNMENT_SZ != 0){
        return NULL;
    }
    //search through the olist, if ilist for blocksize exist: insert at that list,
    // else create the ilist then insert
    sf_free_list_node* correspondListNode = NULL;
    sf_free_list_node *listNodePtr = sf_free_list_head.next;
    while(listNodePtr != &sf_free_list_head && listNodePtr->size < blocksize)
        listNodePtr = listNodePtr->next;
    //if node found
    if(listNodePtr->size == blocksize)
            correspondListNode = listNodePtr;

    // if node not found
    else if(correspondListNode==NULL){
        //create the node
        correspondListNode = sf_add_free_list(blocksize,listNodePtr);
        //initialize head

        sf_header* head = &(correspondListNode->head);
        head->links.next = head;
        head->links.prev = head;
    }
    return correspondListNode;
}

void insertBlockAtHead(sf_header* blockPtr,sf_header* head){
    sf_header* nextNode = head->links.next;

    head->links.next = blockPtr;
    blockPtr->links.prev = head;

    blockPtr->links.next = nextNode;
    nextNode->links.prev = blockPtr;


}

void initHeaderAndFooter(sf_header* headerPtr,size_t blocksize){
    sf_block_info* info =  &headerPtr->info;
     *info= (sf_block_info){FREE,FREE,0,blocksize>>4,0};
    sf_footer* footPtr = getFooterPtr(headerPtr);
    footPtr->info = (sf_block_info){FREE,FREE,0,blocksize>>4,0};
}
/*
precondition: blocksize|16
param: blockHeaderPtr: ptr to the header of the block
    blocksize: size of block
    new : if this is a new block to be built
*/
void* buildFreeBlock(sf_header* blockHeaderPtr,size_t blocksize){
    //block must be multiple of 16
    if(blocksize % ALIGNMENT_SZ != 0){
        return NULL;
    }
    initHeaderAndFooter(blockHeaderPtr,blocksize);
    return updateFreeBlock(blockHeaderPtr, blocksize);
}
/*
update info struct of given block, in both header and footer
param: headerPtr: ptr to update info
   newInfo: info struct, if skip elements with value -1
*/

void updateFreeBlockInfo(sf_header* headerPtr,size_t blocksize){
    sf_block_info* infop = & headerPtr->info;
    infop->allocated = FREE;
    infop->block_size = blocksize>>4;
    infop->requested_size = 0;

    sf_footer* footPtr = getFooterPtr(headerPtr);
    sf_block_info* infop_f = & footPtr->info;
    infop_f->allocated = FREE;
    infop_f->block_size = blocksize>>4;
    infop_f->requested_size = 0;
    infop_f->prev_allocated = infop->prev_allocated;
}
/*
precondition: blocksize|16
param: blockHeaderPtr: ptr to the header of the block
    blocksize: size of block
*/
void* updateFreeBlock(sf_header* blockHeaderPtr,size_t blocksize){
     //block must be multiple of 16
    if(!validateBlocksize(blocksize))
        return NULL;
    updateFreeBlockInfo(blockHeaderPtr,blocksize);
    //tell next block this is free block
    informNextBlock(blockHeaderPtr,FREE);
    //get listNode corresponds to the size of the block
    sf_free_list_node* correspondListNode = getCorrespondListNode(blocksize);
    //insert block to the head of the listNode
    insertBlockAtHead(blockHeaderPtr,&(correspondListNode->head));
    void* spacePtr = (void*)(blockHeaderPtr+1);


    return spacePtr;
}

void initHeap(){
    //initialize prologue
    sf_prologue* prologue = (sf_prologue*)sf_mem_grow();//return pointer to start of additional page
    initPrologue(prologue);
    //initialize epilogue
    //allocate enough space for epilogue
    initEpilogue( (sf_epilogue*)(sf_mem_end()-sizeof(sf_epilogue)));

    //adding first ilist(innerList)
    size_t blocksize = PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue);
    // sf_free_list_node* listNodePtr =
    sf_add_free_list(blocksize, &sf_free_list_head);

    //increment prologue by its size to get to the free space
    // treat the reminding as one block
    sf_header* blockPtr = (sf_header*) (prologue+1);

    // void* spacePtr =
    buildFreeBlock(blockPtr,blocksize);
    blockPtr->info.prev_allocated = ALLOCATED;

}
/*
return the list node thich contain enough space for payload
return NULL if not found
*/
sf_free_list_node* getListNodeAtLeastSize(size_t datasize){
    sf_free_list_node* correspondListNode = NULL;
    sf_free_list_node *listNodePtr = sf_free_list_head.next;
    while(listNodePtr != &sf_free_list_head){
        if(listNodePtr->size-sizeof(sf_block_info)>= datasize){
            correspondListNode = listNodePtr;
            break;
        }
        listNodePtr = listNodePtr->next;
    }

    return correspondListNode;
}
void removeBlockFromList(sf_header* header){
    sf_header* prev = header ->links.prev;
    sf_header* next = header ->links.next;
    prev->links.next = next;
    next->links.prev = prev;
}
sf_header* getNextBlock(sf_header* headerPtr){
    return ((void*)headerPtr+(headerPtr->info.block_size <<4));
}
/*
check prev and next block
if at least one of them  is free coalesce and return pointer to new block
else return orginal pointer
*/
sf_header* coalesce(sf_header* headerPtr){
    int removed = 0;
    //if prev block is free,coaless: old header is new header, remove itself from ilist, change block size, insert into olist
    if(!headerPtr->info.prev_allocated){
        //getBlocksize before moving ptr, note block_size is un shifted
        size_t block_size = headerPtr->info.block_size;
        //remove current block from its lsit
        removeBlockFromList(headerPtr);
        removed =1;
        //previous footer is right before the header
        sf_footer* previousFooter = (sf_footer*)headerPtr-1;
        //old header is new header
        headerPtr = (void*)headerPtr - ((previousFooter->info.block_size)<<4);
        //removeitslef from ilist
        removeBlockFromList(headerPtr);
        //change block size
        headerPtr->info.block_size += block_size;
        //insert into ilist
        // void* spacePtr =
    }
    //if next block is free,
    sf_header* nextBlockp = getNextBlock(headerPtr);
    if(!nextBlockp->info.allocated){
        //remove current block from its list if havent doen so
        if(!removed)
            removeBlockFromList(headerPtr);
        removed=1;
        headerPtr->info.block_size+= nextBlockp->info.block_size;
        removeBlockFromList(nextBlockp);
    }
    if(removed)
        updateFreeBlock(headerPtr,headerPtr->info.block_size <<4);
    return headerPtr;
}
void* addNewPageBlock(){
    //sf_mem_grow changes mem begin to mem end, and increment mem end by 0x1000
    void* pageBegin =  sf_mem_grow();
    if(pageBegin ==NULL){
        // sf_errno = ENOMEM;
        return NULL;
    }
    //old epilogue is now header
    sf_header* headerPtr = (pageBegin-sizeof(sf_footer));
    //move epilogue to the end of page
    sf_epilogue* newEpilogue = sf_mem_end() - sizeof(sf_epilogue);
    newEpilogue ->footer = ((sf_epilogue*) headerPtr) ->footer;//epilogue set

    //build the block


    // newPageBlock has size PAGE_SZ
    updateFreeBlock(headerPtr,PAGE_SZ);
    //coalesce if possible
    headerPtr = coalesce(headerPtr);
    // if prev block is free,coaless: old header is new header, remove itself from ilist, change block size, insert into olist
    // if(! oldEpi->info.prev_allocated){
    //     //previous footer is right before epilogue
    //     sf_footer* previousFooter = oldEpi-1;
    //     //old header is new header
    //     headerPtr = (void*)headerPtr - ((previousFooter->info.block_size)<<4);
    //     //removeitslef from ilist
    //     removeBlockFromList(headerPtr);
    //     //change block size
    //     headerPtr->info.block_size += PAGE_SZ >>4;
    //     //insert into ilist
    //     // void* spacePtr =
    //     buildFreeBlock(headerPtr,headerPtr->info.block_size <<4);
    // }
    return headerPtr;

}
void* getFittingBlock(size_t datasize){
    sf_free_list_node* listNodePtr = getListNodeAtLeastSize(datasize);
    //datasize too big
    if(listNodePtr == NULL){
        //addBlock
        if(addNewPageBlock() == NULL)
                return NULL;
        return getFittingBlock(datasize);
    }
    //current ilist does not have blocks
    while(&(listNodePtr->head) == listNodePtr->head.links.next){
        listNodePtr = listNodePtr->next;
        if(listNodePtr == listNodePtr->next){
            if(addNewPageBlock() == NULL)
                return NULL;
            return getFittingBlock(datasize);
        }
    }
    return listNodePtr->head.links.next;
}

/*
split the given block into two smaller block
one fits the given size. one has the remaining space
*/
sf_header* split(sf_header* headerPtr,size_t firstBlocksize){
    size_t blocksize = headerPtr->info.block_size <<4;
    //second block
    size_t secondBlockSize = blocksize - firstBlocksize;
    sf_header* secondBlockPtr =((void*)headerPtr)+firstBlocksize;
    buildFreeBlock(secondBlockPtr,secondBlockSize);

    //update first block
    //remove self from old list
    if(!headerPtr->info.allocated){
        removeBlockFromList(headerPtr);
        updateFreeBlock(headerPtr,firstBlocksize);
    }
    else{
        setBlockSize(headerPtr,firstBlocksize);
        setBlockPrevAllocBit(secondBlockPtr,ALLOCATED);
    }
    return secondBlockPtr;
    //can return first block ptr, not needed rn
}
/*
set the allocated bit of given block's info to given value
param: headerPtr: ptr of block to be updated
    allocated: info bit to be set, if 0 set 0, othsewize set 1
*/
void setBlockAllocBit(sf_header* headerPtr,int allocated){
    if(allocated)
        headerPtr->info.allocated = ALLOCATED;
    else
        headerPtr->info.allocated = FREE;
}
void setBlockPrevAllocBit(sf_header* headerPtr,int allocated){


    sf_footer* footp =  getFooterPtr(headerPtr);
    if(allocated){

        headerPtr->info.prev_allocated = ALLOCATED;
        footp->info.prev_allocated = ALLOCATED;
    }
    else{
        headerPtr->info.prev_allocated = FREE;
        footp->info.prev_allocated = FREE;
    }

}

void setBlockSize(sf_header* headerPtr,size_t size){
        headerPtr->info.block_size = size>>4;
}
void setBlockRequestedSize(sf_header* headerPtr,size_t size){
        headerPtr->info.requested_size = size;
}
/*
update info struct of given header or footer
param: headerPtr: ptr to update info
       newInfo: info struct, if skip elements with value -1
*/
// void updateInfo(sf_header* headerPtr, sf_block_info newInfo){
//     headerPtr->info = newInfo;

//     if(newInfo.allocated!=-1) infop->allocated = newInfo.allocated;
//     if(newInfo.prev_allocated!=-1) infop->prev_allocated = newInfo.prev_allocated;
//     if(newInfo.two_zeroes!=-1) infop->two_zeroes = newInfo.two_zeroes;
//     if(newInfo.block_size!=-1) infop->block_size = newInfo.block_size;
//     if(newInfo.requested_size!=-1) infop->requested_size = newInfo.requested_size;

// }
/*
return ptr to the footer given header ptr
return NULL if blocksize is 0
*/
sf_footer* getFooterPtr(sf_header* hp){
    //allocated block has no footer
    if(hp->info.allocated)
        return (sf_footer*)hp;
    size_t blocksize = hp->info.block_size<<4;
    //blocksize 0 no footer (epilogue)
    if(blocksize==0){
        //TODO: investigate when size is 0
        // fprintf(stderr, "addr:%p\n",hp);
        // fprintf(stderr, "%p\n",sf_mem_end()-sizeof(sf_epilogue) );
        // fprintf(stderr,"getFooterPtr:blocksize=0");
        return (sf_footer*)hp;
    }
    sf_footer* footp = (void*)hp + blocksize - sizeof(sf_footer);
    return footp;
}
/*
Update the bit in next block's prev_allocated to given allocated value
*/
void informNextBlock(sf_header* headerPtr,int allocated){
     sf_header*nextBlock = ((void*)headerPtr) + (headerPtr->info.block_size <<4);
    if(nextBlock->info.allocated)
    nextBlock->info.prev_allocated = allocated;
    else
        setBlockPrevAllocBit(nextBlock,allocated);
}
void updateAllocBlockInfo(sf_header* headerPtr,size_t requestedSize){
    sf_block_info* infop = & headerPtr->info;
    infop->allocated = ALLOCATED;
    infop->requested_size = requestedSize;
}
/*
allocate given block for usage,
update block info to allocated status
updage prev allocated bit in next block
*/
void updateAllocatedBlock(sf_header* headerPtr,size_t size){
    // todo: remove from the ilist, also set the requestsize and allocated bit
    removeBlockFromList(headerPtr);
    //update header
    updateAllocBlockInfo(headerPtr,size);
    //set prev alloc of next block to 1
    informNextBlock(headerPtr,ALLOCATED);
}


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
int validateBlockPtr(sf_header* headerPtr){

    // The pointer is NULL.
    if(headerPtr==NULL)
        return INVALID;
    // The header of the block is before the end of the prologue, or after the
    // beginning of the epilogue.

    //prologue ends at mem start + sizeof(prologue)
    if((void*)headerPtr< sf_mem_start()+sizeof(sf_prologue))
        return INVALID;
    //the epologue start at mem end -size of epilogue
    void* endOfBlock = (void*) (getFooterPtr+sizeof(sf_footer));
    if(endOfBlock > (void*)(sf_mem_end-sizeof(sf_epilogue)))
        return INVALID;

    sf_block_info headerInfo = headerPtr->info;

    // The allocated bit in the header is 0
    if (headerInfo.allocated==0 )
        return INVALID;
    // The block_size field is not a multiple of 16 or is less than the
    // minimum block size of 32 bytes.
    if(!validateBlocksize(headerInfo.block_size<<4))
        return INVALID;
    // NOTE: It is always a multiple of 16
    // The requested_size field, plus the size required for the block header,
    // is greater than the block_size field.
    if(headerInfo.requested_size+sizeof(sf_block_info)>headerInfo.block_size<<4)
        return INVALID;
    // If the prev_alloc field is 0, indicating that the previous block is free,
    // then the alloc fields of the previous block header and footer should also be 0.
    if(!headerInfo.prev_allocated){
        sf_block_info* prevFootInfoPtr = &headerInfo - 1;
        if(prevFootInfoPtr->allocated !=FREE)
            return INVALID;
        sf_block_info* prevHeaderInfoPtr =
        (void*)prevFootInfoPtr -prevFootInfoPtr->block_size+sizeof(sf_block_info);
        if(prevHeaderInfoPtr->allocated!=FREE)
            return INVALID;
    }
    return VALID;

}
int validateBlocksize(size_t blocksize){
    if(blocksize % ALIGNMENT_SZ != 0){
        fprintf(stderr, "Alignment error: blocksize mod %d != 0\n",ALIGNMENT_SZ );
        return INVALID;
    }
    if(blocksize<32){
        fprintf(stderr, "MIN_BLOCK_SZ error: blocksize < %d\n",MIN_BLOCK_SZ );
        return INVALID;
    }
    return VALID;

}
