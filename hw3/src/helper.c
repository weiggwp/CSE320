// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "debug.h"
// #include "sfmm.h"
// //#include "helper.h"

// void initPrologue(sf_prologue* prologue){
//     prologue -> padding =0x0;

//     //init header
//     sf_header* header = & prologue->header;
//     sf_block_info* info= &header->info;
//     info->allocated = 1;
//     info->prev_allocated = 0;
//     info->two_zeroes = 0;
//     info->block_size = 0;
//     info->requested_size = 0;

//     //init footer
//     sf_footer* footer = &prologue->footer;
//     info= &footer->info;
//     info->allocated = 1;
//     info->prev_allocated = 0;
//     info->two_zeroes = 0;
//     info->block_size = 0;
//     info->requested_size = 0;

// }
// void initEpilogue(sf_epilogue* epilogue){
//     //init footer
//     sf_footer* footer = &epilogue->footer;
//     sf_block_info* info= &footer->info;
//     info->allocated = 1;
//     info->prev_allocated = 0;
//     info->two_zeroes = 0;
//     info->block_size = 0;
//     info->requested_size = 0;

// }

// sf_free_list_node* getCorrespondListNnode(size_t blocksize){
//     //search through the olist, if ilist for blocksize exist: insert at that list,
//     // else create the ilist then insert
//     sf_free_list_node* correspondListNode = NULL;
//     sf_free_list_node *listNodePtr = sf_free_list_head.next;
//     while(listNodePtr != &sf_free_list_head && listNodePtr->size < blocksize)
//         listNodePtr = listNodePtr->next;
//     //if node found
//     if(listNodePtr->size == blocksize)
//             correspondListNode = listNodePtr;

//     // if node not found
//     else if(correspondListNode==NULL){
//         //create the node
//         correspondListNode = sf_add_free_list(blocksize,listNodePtr);
//         //initialize head

//         sf_header* head = &(correspondListNode->head);
//         head->links.next = head;
//         head->links.prev = head;
//     }
//     return correspondListNode;
// }

// void insertBlockAtHead(sf_header* blockPtr,sf_header* head){
//     sf_header* nextNode = head->links.next;

//     head->links.next = blockPtr;
//     blockPtr->links.prev = head;

//     blockPtr->links.next = nextNode;
//     nextNode->links.prev = blockPtr;


// }
// /*
// precondition: blocksize|16

// */
// void* buildFreeBlock(sf_header* blockHeaderPtr,size_t blocksize){
//     //block must be multiple of 16
//     if(blocksize%16!=0){
//         return NULL;
//     }
//     //initialize header
//     blockHeaderPtr -> info = (sf_block_info){0,0,0,blocksize>>4,0};
//     //initialize footer
//     sf_footer* blockFooterPtr =(sf_footer*) ((void*)blockHeaderPtr +blocksize - sizeof(sf_footer));
//     blockFooterPtr ->info = (sf_block_info){0,0,0,blocksize>>4,0};

//     //get listNode corresponds to the size of the block
//     sf_free_list_node* correspondListNode = getCorrespondListNnode(blocksize);
//     //insert block to the head of the listNode
//     insertBlockAtHead(blockHeaderPtr,&(correspondListNode->head));
//     void* spacePtr = (void*)(blockHeaderPtr+1);


//     return spacePtr;


// }
// void initHeap(){
//     //initialize prologue
//     sf_prologue* prologue = (sf_prologue*)sf_mem_grow();//return pointer to start of additional page
//     initPrologue(prologue);
//     //initialize epilogue
//     //allocate enough space for epilogue
//     initEpilogue( (sf_epilogue*)(sf_mem_end()-sizeof(sf_epilogue)));

//     //adding first ilist(innerList)
//     size_t blocksize = PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue);
//     // sf_free_list_node* listNodePtr =
//     sf_add_free_list(blocksize, &sf_free_list_head);

//     // is head in listNodePtr set already for me ? debug line 77 sfmm.c

//     //increment prologue by its size to get to the free space

//     // treat the reminding as one block
//     sf_header* blockHeaderPtr = (sf_header*) (prologue+1);

//     // void* spacePtr =
//      buildFreeBlock(blockHeaderPtr,blocksize);
// }
// /*
// return the list node thich contain enough space for payload
// return NULL if not found
// */
// sf_free_list_node* getListNodeAtLeastSize(size_t datasize){
//     sf_free_list_node* correspondListNode = NULL;
//     sf_free_list_node *listNodePtr = sf_free_list_head.next;
//     while(listNodePtr != &sf_free_list_head){
//         if(listNodePtr->size-8 >= datasize){
//             correspondListNode = listNodePtr;
//             break;
//         }
//         listNodePtr = listNodePtr->next;
//     }

//     return correspondListNode;
// }
// void removeBlockFromList(sf_header* header){
//     sf_header* prev = header ->links.prev;
//     sf_header* next = header ->links.next;
//     prev->links.next = next;
//     next->links.prev = prev;
// }
// void addNewPageBlock(){
//     //sf_mem_grow changes mem begin to mem end, and increment mem end by 0x1000
//     void* pageBegin =  sf_mem_grow();
//     //old epilogue is now header
//     sf_header* headerPtr = (pageBegin-sizeof(sf_footer));
//     //move epilogue to the end of page
//     sf_epilogue* newEpilogue = sf_mem_end() - sizeof(sf_epilogue);
//     newEpilogue ->footer = ((sf_epilogue*) headerPtr) ->footer;

//     //coalese with previous block if free(check old footer, if prev allocated is 0 then free)
//     sf_footer* oldEpi = ((sf_footer*)headerPtr);

//     //if prev block is free,coaless: old header is new header, remove itself from ilist, change block size, insert into olist
//     if(! oldEpi->info.prev_allocated){
//         //previous footer is right before epilogue
//         sf_footer* previousFooter = oldEpi-1;
//         //old header is new header
//         headerPtr = (void*)headerPtr - ((previousFooter->info.block_size)<<4);
//         //removeitslef from ilist
//         removeBlockFromList(headerPtr);
//         //change block size
//         headerPtr->info.block_size += PAGE_SZ >>4;
//         //insert into ilist
//         // void* spacePtr =
//         buildFreeBlock(headerPtr,headerPtr->info.block_size <<4);
//     }

// }
// void* getFittingBlock(size_t datasize){
//     sf_free_list_node* listNodePtr = getListNodeAtLeastSize(datasize);
//     //datasize too big
//     if(listNodePtr == NULL){
//         //addBlock
//         addNewPageBlock();
//         return getFittingBlock(datasize);
//     }
//     //current ilist does not have blocks
//     while(&(listNodePtr->head) == listNodePtr->head.links.next){
//         listNodePtr = listNodePtr->next;
//         if(listNodePtr == listNodePtr->next){
//             addNewPageBlock();
//             return getFittingBlock(datasize);
//         }
//     }
//     return listNodePtr->head.links.next;
// }

// void split(sf_header* headerPtr,size_t firstBlocksize){
//     size_t blocksize = headerPtr->info.block_size <<4;
//     //remove self from old list
//     removeBlockFromList(headerPtr);
//     //first block
//     buildFreeBlock(headerPtr,firstBlocksize);
//     //second block
//     size_t secondBlockSize = blocksize - firstBlocksize;
//     sf_header* secondBlockPtr =((void*)headerPtr)+firstBlocksize;
//     buildFreeBlock(secondBlockPtr,secondBlockSize);

//     //can return first block ptr, not needed rn
// }
// sf_footer* getFooterPtr(sf_header* hp){
//     size_t blocksize = hp->info.block_size;
//     sf_footer* footp = (void*)hp + blocksize - sizeof(sf_footer);
//     return footp;
// }
// void updateInfo(sf_header* headerPtr, sf_block_info newInfo){
//     sf_block_info* infop = & (headerPtr->info);


//     if(newInfo.allocated!=-1) infop->allocated = newInfo.allocated;
//     if(newInfo.prev_allocated!=-1) infop->prev_allocated = newInfo.prev_allocated;
//     if(newInfo.two_zeroes!=-1) infop->two_zeroes = newInfo.two_zeroes;
//     if(newInfo.block_size!=-1) infop->block_size = newInfo.block_size;
//     if(newInfo.requested_size!=-1) infop->requested_size = newInfo.requested_size;

// }

// void updateFreeBlockInfo(sf_header* headerPtr, sf_block_info newInfo){
//     updateInfo(headerPtr,newInfo);

//     sf_footer* footp = getFooterPtr(headerPtr);
//     updateInfo(footp);
// }
// void allocateBlock(sf_header* headerPtr,size_t size){
//     // todo: remove from the ilist, also set the requestsize and allocated bit
//     removeBlockFromList(headerPtr);
//     //update header
//     updateInfo((sf_block_info){0,-1,-1,-1,size});


//     size_t blocksize = headerPtr->info.block_size <<4;
//     //set prev alloc of next block to 1
//     sf_header*nextBlock = ((void*)headerPtr) + blocksize;
//     nextBlock->info.prev_allocated = 1;