#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "smalloc.h"
#include <stdint.h>
#include <inttypes.h>

#define PAGESIZE 4096
#define MIN_BLOCK_SIZE 32

//Global variables
uint8_t* heap = NULL; 
uint8_t* free_head = NULL;
int heap_size = 0; 

//free_list points to the head of the

/*
 * my_init() is called one time by the application program to to perform any 
 * necessary initializations, such as allocating the initial heap area.
 * size_of_region is the number of bytes that you should request from the OS using
 * mmap().
 * Note that you need to round up this amount so that you request memory in 
 * units of the page size, which is defined as 4096 Bytes in this project.
 */

struct header {
    //b_size: returns the size of the entire memory block (header + payload + optinoal padding)
    //allocated: 0 if free, 1 if not free 
    //next_free: pointer to the next free block (in order of memory addresses)
    //prev_free: pointer to previous free block (also in order of memory addresses)

    int b_size; 
    int allocated; 
    uint8_t* next_free; 
    uint8_t* prev_free; 
}; 

int my_init(int size_of_region) {
  /*
   * Implement your initialization here.
   */
   
   //use mmap to allocate new pages 
   //open /dev/zero device first --> int fd = open("/dev/zero", O_RDWR); 
   //use fd to perform mmap --> https://man7.org/linux/man-pages/man2/mmap.2.html
   //close the device

   //1. Calculate the rounded up number of bytes to request 

   heap_size = (size_of_region <= PAGESIZE) ? PAGESIZE: PAGESIZE * ((size_of_region + PAGESIZE - 1) / PAGESIZE); 
   //heap_size is PAGE_SIZE if requested size of region is <= PAGE_SIZE, else it becomes a multiple of PAGE_SIZE. 

   //open fd /dev/zero and then map to virtual mem using mmap

   int fd = open("/dev/zero", O_RDWR);

   if (fd == -1){
    return -1; 
   }

   heap = mmap(NULL, heap_size, PROT_READ | PROT_WRITE,
                                        MAP_SHARED, fd, 0);

    if (heap == MAP_FAILED){
        close(fd);
        return -1; 
    }

    //Initializing the free list here:
    struct header* first_block = (struct header*) heap;
    first_block->b_size = heap_size; 
    first_block->allocated = 0;
    first_block->next_free = NULL; 
    first_block->prev_free = NULL; 
    //we dereference the pointer returned by mmap (beginning of memory region)
    //we set the memory at the beginning of the file to hold this struct
    free_head = heap; 
   //RETURNS:
        //0 upon success, otherwise -1

    //open /dev/zero using an open operation
    //map the dev zero file to 
    close(fd); 
    return 0;
    //Probably need to initialize the start of the explicit linked list (where the big block is the start)
}


/*
 * smalloc() takes as input the size in bytes of the payload to be allocated and 
 * returns a pointer to the start of the payload. The function returns NULL if 
 * there is not enough contiguous free space within the memory allocated 
 * by my_init() to satisfy this request.
 */
void *smalloc(int size_of_payload, Malloc_Status *status) {
  /*
   * Implement your malloc here.
   */

   //FOLLOW FIRST-FIIT REPLACEMENT POLICY 

   //Sample flow logic: 
        //0. User requests payload of size = size_of_payload
        //1. Search free list for the first section that fits size
        //2. Following first-fit, allocate the first block that can accomodate the payload 
        //3. Initialize the block 
            //a. Create the header: 
                //i. block size = 4 bytes 
                //ii. allocated = 4 bytes 
                //iii. next free block = pointer, 8 bytes 
                //iv. prev free block = pointer, 8 bytes 
            //b. payload = allocated block only 
            //c. padding = optional (for 8 byte alignment)
        //4. External Padding Check: 
            //if remaining free bytes after smalloc is large enough (at least 32 bytes):
                //fragment 
            //else:
                //check if the leftover bytes can be added as padding:
                    //if they can be added to the block and still be 8-aligned, then add 
                //else: 
                    //leave as is
        //5. Update free list
            //update next pointer 
            //update prev pointer
    
    //PRE-PROCESSING OUR PAYLOAD TO THE NEAREST MULTIPLE OF 8, this ensures padding and that 
    //subsequent arithmetic always results in multiples of 8 

    status->success = 0; 
    status->payload_offset = -1; 
    status->hops = -1; 

    //check if size is valid: 
    if (size_of_payload <= 0){
        return NULL; 
    }

    int aligned_payload_s = (size_of_payload + 7) & ~7;

    int required_block_s = sizeof(struct header) + aligned_payload_s; 

    uint8_t* current = free_head; 
    int hops = 0; 

    while (current != NULL){
        struct header* current_header = (struct header*)current;

    //we check if the current block is large enough and then allocate
    if (current_header ->b_size >= required_block_s){
        //now we calculate the remaining size after we allocate 
        int remaining_s = current_header->b_size - required_block_s; 

        //padding logic goes here now: 
        if (remaining_s >= MIN_BLOCK_SIZE){
            //now we split the block
            current_header->b_size = required_block_s;
            current_header->allocated = 1; 

            //we create a new free block since we've segemented 
            struct header* new_free = (struct header*)(current + required_block_s);
            new_free->b_size = remaining_s; 
            new_free->allocated = 0; 
            new_free->next_free = current_header->next_free; 
            new_free->prev_free = current_header->prev_free; 

            //now we have to update the free_head
            if (current_header->prev_free){
                ((struct header*)(current_header->prev_free))->next_free=(uint8_t*)new_free; 
            } else {
                //prev_free didn't exist meaning that that block was the head
                free_head = (uint8_t*)new_free; 
            }

            if (current_header->next_free){
                ((struct header*)(current_header->next_free))->prev_free=(uint8_t*)new_free;
            }
        } else {
            //we absorb the whole thing for padding 
            current_header->allocated = 1; 

            //update free list now
            if (current_header->prev_free){
                ((struct header*)(current_header->prev_free))->next_free = current_header->next_free;
            } else{
                //this was the head
                free_head = current_header->next_free;
            }

            if (current_header->next_free){
                ((struct header*)(current_header->next_free))->prev_free = current_header->prev_free;
                }
            }
            
            status->success = 1; 
            status->payload_offset = (unsigned long)(current + sizeof(struct header)) - (unsigned long)heap;
            status->hops = hops;

            
            return (void*)(current + sizeof(struct header));
        }

    //current block doesn't fit so we move on 
    current = current_header->next_free; 
    hops++;
    }

    return NULL; 
}



/*
 * sfree() frees the target block. "ptr" points to the start of the payload.
 * NOTE: "ptr" points to the start of the payload, rather than the block (header).
 */
void sfree(void *ptr)
{
  /*
   * Implement your free here.
   */

   //flow logic: 

   //if ptr is NULL, no operation is performed
   //if not NULL, check the address list and insert at the proper point
   //ensure proper coalescing measures 
        //left and right proper checks
    //update next free and prev_free pointers 

//case 1: left and right neighbors are allocated --> NO COALESCING INVOLVED

  //case 2: left = allocated, right = free --> COALESCE RIGHT 


  //case 3: right = allocated, left = free --> COALESCE LEFT 


  //case 4: left and right are free --> COALESCE BOTH LEFT AND RIGHT 
  
  if (ptr == NULL){
    return; 
  }

  

  //getting the header of the block being freed 
  struct header* block_header = (struct header*)((uint8_t*)ptr - sizeof(struct header));
  uint8_t* current = free_head; 
  uint8_t* prev = NULL; 
  block_header->allocated = 0;

  while (current != NULL && current < (uint8_t*)block_header){
    prev = current; 
    current = ((struct header*)current)->next_free; 
  }
  if (prev == NULL){
    block_header->prev_free = NULL; 
    block_header->next_free = free_head;

    if (free_head != NULL){
        ((struct header*)free_head)->prev_free = (uint8_t*)block_header; 
     }
     free_head = (uint8_t*)block_header; 
  } else {
    //inserting in the middle or end of the list
    block_header->prev_free = prev; 
    block_header->next_free = ((struct header*)prev)->next_free; 

    ((struct header*)prev)->next_free = (uint8_t*)block_header; 

    if (block_header->next_free != NULL){
        ((struct header*)(block_header->next_free))->prev_free = (uint8_t*)block_header;
    }
  }

  ///coalescing logic now
  //we check if previous block is adjacent, if adjacent then we coalesce with previous block

  if (block_header->prev_free != NULL){
    struct header* prev_header = (struct header*)(block_header->prev_free);

    if ((uint8_t*)prev_header + prev_header->b_size == (uint8_t*)block_header){
        prev_header->b_size += block_header->b_size; 
        prev_header->next_free = block_header->next_free;

        if (block_header->next_free != NULL){
            ((struct header*)(block_header->next_free))->prev_free = (uint8_t*)prev_header; 
        }

        block_header = prev_header; 
    }
  }

  if (block_header->next_free != NULL){
    struct header* next_header = (struct header*)(block_header->next_free);

    //check if next block is adjacent 
    if ((uint8_t*)block_header + block_header->b_size == (uint8_t*)next_header){
        block_header->b_size += next_header->b_size; 
        block_header->next_free = next_header->next_free; 

        if (next_header->next_free != NULL){
            ((struct header*)(next_header->next_free))->prev_free = (uint8_t*)block_header; 
        }
    }
  }
}
