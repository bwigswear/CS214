#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mymalloc.h"

//block structure
//FREE
//addr      ->  header (size)
//addr+8    ->  prev
//addr+16   ->  next
//addr+24   ->  footer (size)

//ALLOCCED
//addr                  ->  header (size + 1)
//addr+8                ->  payload
//addr+payloadSize+8    ->  footer (size + 1)

//u said this needs to be changed, just update the number to whatever it was
unsigned long HEAP_SIZE = 1000000;
//head of explicit free list
void *head;
//next free block after last malloc (used in allocAlg 1)
void *next_free;
//what allocation argument to use: 0 = first fit, 1 = next fit, 2 = best fit
int globalAllocAlg = 0;

void* mem_start = 0;
void* mem_end = 0;

void mycleanup(){
    free(mem_start);
}

void myinit(int allocAlg){
    head = (void*) malloc(HEAP_SIZE);
    unsigned long* header = head;
    *header = HEAP_SIZE;
    void** prev = head + 8;
    *prev = NULL;
    void** next = head + 16;
    *next = NULL;
    unsigned long* footer = head + (HEAP_SIZE) - 8; 
    //unsigned long* footer = head + 3;
    *footer = HEAP_SIZE;
    next_free = head;
    mem_start = head;
    mem_end = head + (HEAP_SIZE / 8);


    //setting the global allocation algorithm to be used
    if (allocAlg == 0 || allocAlg == 1 || allocAlg == 2){
        globalAllocAlg = allocAlg;
    }
}

//helper function for mymalloc() since each allocAlg uses the same process for allocating once ideal block is found
void* getBlockFromFreeList(unsigned long* block, size_t totalSize){
    if (*block - totalSize < 32){
        //case 0: block is exact size needed, entire block is consumed and node can be removed from list
        next_free = (void*) *(block + 2);
        if (head == block){
            head = (void*) *(block + 2);
        }

        if((void*) *(block+1) != NULL && (void*) *(block+2) != NULL){
            void** prev_to_next = (void*) *(block+1)+16;
            *prev_to_next = (void*) *(block+2);
            void** next_to_prev = (void*) *(block+2)+8;
            *next_to_prev = (void*) *(block+1);
        }else if((void*) *(block+1) != NULL && (void*) *(block+2) == NULL){
            void** prev_to_next = (void*) *(block+1)+16;
            prev_to_next = NULL;
        }else{
            void** next_to_prev = (void*) *(block+2)+8;
            next_to_prev = NULL;
        }

        unsigned long* currHeader = block;
        *currHeader = totalSize + 1;
        void* payload = block + 1;
        unsigned long* currFooter = block + ((totalSize - 8) / 8);
        *currFooter = totalSize + 1;
   
        return payload;

    }else{
        //case 1: block has extra space, node can remain in list but size and address must be changed
        
        //create new block -- free
        unsigned long* nbHeader = block + (totalSize / 8);
        *nbHeader = *block - totalSize;
        //void** prev = (void**) nbHeader + 1;
        //*prev = (void*) *(block + 1);
        *(nbHeader + 1) = *(block + 1);
        *(nbHeader + 2) = *(block + 2);

        //void** next = (void**) nbHeader + 2;
        //*next = (void*) *(block + 2);
        unsigned long* nbFooter = nbHeader + ((totalSize - 8) / 8);
        *nbFooter = *block - totalSize;
        if ((void**) *(nbHeader+1)!= NULL) {
            void** prev_to_next = (void*) *(nbHeader+1)+16;
            *prev_to_next = nbHeader;
        }
        if ((void**) *(nbHeader+2) != NULL) {
            void** next_to_prev = (void*) *(nbHeader+2)+8;
            *next_to_prev = nbHeader;
        }

        //change curr block -- allocated
        unsigned long* currHeader = block;
        *currHeader = totalSize + 1;
        void* payload = block + 1;
        unsigned long* currFooter = block + ((totalSize - 8) / 8);
        *currFooter = totalSize + 1;
   
        if (head == currHeader){
            head = nbHeader;
        }
        next_free = nbHeader;

        return payload;
    }
}

void* mymalloc(size_t size)
{   
    if (size == 0){
        return NULL;
    }
    //making the header + payload a multiple of 8 bytes
    size_t totalSize = 32;
    if (size > 16){
        if (size % 8 == 0) totalSize += (size - 16);
        else totalSize += ((size + (8 - (size % 8))) - 16);
    }
    if (globalAllocAlg == 0){
        //first fit algorithm (first that fits)
        unsigned long* curr = head;
        while((void*) curr != NULL){
            if (*curr >= totalSize) return getBlockFromFreeList(curr, totalSize);
            curr = (unsigned long*) *(curr + 2);
        }
    }else if (globalAllocAlg == 1){
        //next fit algorithm (first that fits after last allocated)
        unsigned long* curr = next_free;
        while((void*) curr != NULL){
            if (*curr >= totalSize) return getBlockFromFreeList(curr, totalSize);
            curr = (unsigned long*) *(curr + 2);
        }

        //if hasn't found one yet go back to head
        //if this infinite loops, change to (curr != NULL)
        curr = head;
        while((void*) curr != NULL){
            if (*curr >= totalSize) return getBlockFromFreeList(curr, totalSize);
            curr = (unsigned long*) *(curr + 2);
        }
    }else{
        //best fit algorithm (smallest that fits)

        //store pointer to best block here
        unsigned long* best_block = NULL;
        //find best block
        unsigned long* curr = head;
        while((void*) curr != NULL){
            if (*curr >= totalSize){
                if (best_block == NULL || *curr < *best_block) best_block = curr;
            } 
            curr = (unsigned long*) *(curr+2);        }
        if (best_block != NULL) return getBlockFromFreeList(best_block, totalSize);
    }
}

//READ THIS BEN: curr + 2, footer

void myfree(void* ptr){
    //Four possible outcomes: block is isolated from free blocks, free block on left, free block on right, or free block on both sides
    //To find right block data: backtrack header size bytes to get block size and using size check to see if subsequent block is allocated
    //To find left block data: find next free block and go to prev pointer, if that block + its size leads to current then that block is before current
    //Set current block to be no longer allocated
    //If necessary coalesce, then set new pointers
    
    //Mark header and footer as free
    unsigned long* headerAddress = ptr - 8;
    unsigned long size = *headerAddress;
    size--;
    *headerAddress = size;
    unsigned long* footerAddress = headerAddress + (size/8) - 1;
    *footerAddress = size;
    
    //Check to see if left and right are allocated
    //ADD CHECK FOR END OF HEAP
    int left = 0;
    int right = 0;
    unsigned long* nextBlock = footerAddress + 1;
    unsigned long nextSize;
    unsigned long prevSize;
    if ((void*) nextBlock < mem_end){
        nextSize = *nextBlock;
        if(nextSize % 8 == 0){right = 1;}
    }
    unsigned long* prevBlock = headerAddress - 1;
    if ((void*) prevBlock >= mem_start){
        prevSize = *prevBlock;
        if(prevSize % 8 == 0){left = 1;}
    }



    if(left && right){//If left and right are free, coalesce all 3
        unsigned long* newHeader = headerAddress - (prevSize/8);
        //Set new header
        unsigned long newSize = size + nextSize + prevSize;
        *newHeader = newSize;
        //Set new footer
        *(newHeader + (newSize/8) - 1) = newSize;
        //Set new next
        *(newHeader + 2) = *(nextBlock+2);
        //New prev will be same as old prev
    }else if(left){//If only left is free, coalesce
        unsigned long* newHeader = headerAddress - (prevSize/8);
        //Set new header
        unsigned long newSize = size + prevSize;
        *newHeader = newSize;
        //Set new footer
        *(newHeader + (newSize/8) - 1) = newSize;
        //New next and prev should be the same
    }else if(right){//If only right is free, coalesce
        //Set new header
        unsigned long newSize = size + nextSize;
        *headerAddress = newSize;
        //Set new footer
        *(headerAddress + (newSize/8 - 1)) = newSize;
        //Set new prev
        *(headerAddress + 1) = *(nextBlock + 1);
        //Set new next
        *(headerAddress + 2) = *(nextBlock + 2);

        if (nextBlock == head) head = headerAddress;
        if (nextBlock == next_free) next_free = headerAddress;
     
    }else{//If there are no adjacent free blocks
        printf("isolated middle block\n");
        unsigned long* curr = head;
        if((void*) headerAddress < head){
            unsigned long* head_ptr = head;
            *(head_ptr+1) =  (unsigned long) headerAddress;
            *(headerAddress+2) = (unsigned long) head;
            *(headerAddress+1) = (unsigned long) NULL;
            head = headerAddress;
        }else{
            while((void*) curr != NULL){
                if ( curr < headerAddress && *(curr+2) == (unsigned long) NULL){
                    *(headerAddress+1) = (unsigned long) curr;
                    *(headerAddress+2) = (unsigned long) NULL;
                    *(curr+2) = (unsigned long) headerAddress;
                }else if (curr < headerAddress && headerAddress <  (unsigned long*) *(curr+2)){
                    *(headerAddress+1) = (unsigned long) curr; 
                    *(headerAddress+2) = *(curr+2);
                    void** next_to_prev = (void*) *(curr+2)+8;
                    *next_to_prev = headerAddress;
                    *(curr+2) = (unsigned long) headerAddress;
                }else{
                    curr = (unsigned long*) *(curr+2);
                    continue;
                }
                break;
            }
        }
    //printf("%p\n", curr);
        
    }
        
    
    //printf("Actual block start addr: %d, Block Size: %d bytes", blockStart, *blockStart);
}

void* myrealloc(void* ptr, size_t size){
  //Few cases with intial inputs: size = 0 in which case we free the ptr (I think), size < initial size
  //in which case we should check to see if the original data can fit in the new size. size > initial size
  //in which case we should do two things. First, we can try something similar to the free algorithm where 
  //we check to see if adjacent blocks in memory are free. If nothing there is free, then we need to look for
  //a suitable block, store the data there, and free the original block.
  
    if(size == 0 && ptr == NULL) return NULL;
    
    if(size == 0){
      myfree(ptr);
      return NULL;
    }
    if(ptr == NULL){
      return mymalloc(size);
    }

    size_t totalSize = 32;
    if (size > 16){
        if (size % 8 == 0) totalSize += (size - 16);
        else totalSize += ((size + (8 - (size % 8))) - 16);
    }

    unsigned long* headerAddress = ptr - 8;
    unsigned long oldSize = *headerAddress - 1;

    if(totalSize == oldSize) return ptr;
  
    if(totalSize < oldSize){
        if(oldSize - totalSize < 32){
            return ptr;
        }else{
            unsigned long splitSize = oldSize - totalSize;
            *headerAddress = totalSize + 1;
            *(headerAddress + (totalSize / 8) - 1) = totalSize + 1;
            unsigned long* splitAddress = headerAddress + (totalSize / 8);
            *splitAddress = splitSize + 1;
            *(splitAddress + (splitSize / 8) - 1) = splitSize + 1;
            myfree(splitAddress);
            return ptr;
        }
    }else if(totalSize > oldSize){
    //First, check if right and left adjacent blocks are free, same logic as myfree
        int right = 0;
        unsigned long* footerAddress = headerAddress + (oldSize / 8) - 1;
        unsigned long* nextBlock = footerAddress + 1;
        printf("head %d\n", headerAddress);
        printf("foot %d\n", footerAddress);

        unsigned long nextSize;
        unsigned long prevSize;
    
        if ((void*) nextBlock < mem_end){
            printf("sdfdsf\n");
            nextSize = *nextBlock;
            if(nextSize % 8 == 0){right = 1;}
        }
        printf("next size %d\n", *nextBlock);
    
        //If right is free, check if current size + right size > size
        if(right){
            printf("heresdfdsfdsfseeee\n");
            if(oldSize + nextSize > totalSize){
                printf("hereeeee\n");
                unsigned long* extraSpaceStart;
                if (totalSize - oldSize < 16) extraSpaceStart =   getBlockFromFreeList(nextBlock, 16);
                else extraSpaceStart = getBlockFromFreeList(nextBlock, totalSize - oldSize);
                printf("hereeeee\n");
                unsigned long newSize = oldSize + *(extraSpaceStart-1)-1;
                printf("olddize: %d\n",oldSize);
                printf("extra: %d\n",*(extraSpaceStart));
                printf("newsize: %d\n",newSize);
                *headerAddress = newSize + 1;
                *(headerAddress + (newSize/8) - 1) = newSize + 1;
                return headerAddress+1;
            }
        }


        //If neither left or right are sufficient, then find a new block

        void* newPointer = mymalloc(size);
        memcpy(newPointer, ptr, oldSize);
        myfree(ptr);
        printf("here3\n");
        return newPointer;
    }
}