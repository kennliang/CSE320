/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include <errno.h>
#include "sfmm.h"

#define size_of_page 4096


void *sf_malloc(size_t size) {
    static int initialization = 0;

    // if size is 0 return null without setting sf_errno
    if(size == 0)
        return NULL;

    // adding the header bytes to the requested size
    size_t size_block = size + 8;
    // computing the amount of adding needed to reach double-word alignment
    int remainder = 16 - (size_block % 16);
    if(remainder != 16)
        size_block = size_block + remainder;

    // ensuring that the blocksize is a minimum of 32 bytes
    if(size_block < 32)
        size_block = size_block + 16;

    // no heap exit - create the heap
    if(initialization == 0)
    {
        // returns a void pointer to the start of the page
        void *p = sf_mem_grow();
        // see if sf_mem_grow was unsuccessful
        if(p == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }

        //setting the prologue (40 bytes)
        sf_prologue *pro = (sf_prologue *)p;
        pro->unused1 = 0;
        pro->footer.block_size = 33;
        pro->footer.requested_size = 0;
        pro->block.header.block_size = 33;

        // initial size of the first free block - 4096-40-8 = 4048
        int first_block_size = size_of_page -sizeof(sf_prologue) - sizeof(sf_header);

        //setting the initial free block header
        sf_block *free_block = (sf_block *) (p+ sizeof(sf_prologue));
        free_block->header.block_size = first_block_size + 2;
        free_block->body.links.next = &sf_free_list_head;
        free_block->body.links.prev = &sf_free_list_head;

        //setting the initial free block footer
        sf_header * free_footer = sf_mem_end() -16;
        free_footer->block_size = first_block_size + 2;

        //setting the epilogue (8 bytes)
        sf_epilogue * epil = (sf_epilogue *)(sf_mem_end() - sizeof(sf_header));
        epil->header.block_size = 1;

        //adding the initial free block to the circular, doubly linked list (sentinal node)
        sf_free_list_head.body.links.next = free_block;
        sf_free_list_head.body.links.prev = free_block;
        initialization = 1;

    }
    while(1)
    {
        //traversing through quick list
        for(int i = 0;i < NUM_QUICK_LISTS; i++)
        {
            if(sf_quick_lists[i].length != 0)
            {
                // the block size exactly matches the quick list specified block size
                if(size_block == 32 + i * 16)
                {
                    // remove the first block in the quick_list and assign the next as beginning
                    sf_block *match = sf_quick_lists[i].first;
                    sf_quick_lists[i].first = match->body.links.next;

                    sf_quick_lists[i].length = sf_quick_lists[i].length - 1;

                    match->header.requested_size = size;

                    return match->body.payload;
                }
            }
        }

        // traverse through main free list to find free block
        sf_block *find = sf_free_list_head.body.links.next;
        while(find != &sf_free_list_head)
        {
            if(find->header.block_size >= size_block)
            {
                //remove the block from main free list
                find->body.links.prev->body.links.next = find->body.links.next;
                find->body.links.next->body.links.prev = find->body.links.prev;

                //check if previous block allocated
                int prev_alloc = 0;
                if((find->header.block_size & 2) == 2)
                        prev_alloc = 2;

                int split = find->header.block_size - size_block - prev_alloc;
                //split the block -  does not create block sizes less than 32 bytes
                if(split >= 32)
                {
                    //setting the block_size and allocated bit of the allocated block
                    find->header.block_size = size_block + 1 + prev_alloc;

                    //set the header and footer for the split free block in main list
                    void *split_block = (void *)find + size_block;

                    sf_block *new_block = (sf_block *)split_block;
                    //ensure the requested size bits will be same as footer requested size
                    new_block->header.requested_size = 0;

                    //set the previous allocated block
                    new_block->header.block_size = split + 2;

                    //add the new block into the main free list
                    new_block->body.links.next = sf_free_list_head.body.links.next;
                    new_block->body.links.prev = &sf_free_list_head;
                    sf_free_list_head.body.links.next->body.links.prev = new_block;
                    sf_free_list_head.body.links.next = new_block;

                    sf_header *new_footer = (sf_header *)(split_block + split - sizeof(sf_header));

                    //same information as header above
                    new_footer->requested_size = 0;
                    new_footer->block_size = split + 2;

                }
                else
                {
                    find->header.block_size = find->header.block_size + 1;
                    sf_header *p = (sf_header *)((void *)find + find->header.block_size - prev_alloc -1);
                    p->block_size = p->block_size + 2;
                }
                find->header.requested_size = size;
                return find->body.payload;

            }

            find = find->body.links.next;
        }

        //extend heap here

        // returns a void pointer to the start of the page
        void *new_mem = sf_mem_grow();
        // see if sf_mem_grow was unsuccessful
        if(new_mem == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }

        //setting the free block of new page (replace old epilogue with header of free block)
        sf_block *free_block = (sf_block *) (new_mem - sizeof(sf_header));

        //setting the initial free block footer
        sf_header *free_footer = sf_mem_end() -16;

        //setting the epilogue (8 bytes)
        sf_epilogue * epil = (sf_epilogue *)(sf_mem_end() - sizeof(sf_header));
        epil->header.block_size = 1;

        sf_header *combine = (sf_header *)((void *)free_block - sizeof(sf_header));
        int add = 0;
        if((combine->block_size % 16) != 0)
            add = combine->block_size % 16;

        sf_block *combined_block = (sf_block *)((void *)free_block - combine->block_size + add);

        if((combine->block_size & 1) != 1 && (combined_block->header.block_size & 1) != 1
            && combined_block->header.block_size == combine->block_size && combined_block->header.requested_size == combine->requested_size)
        {
            //valid free block to coalesce with
            //setting new blocksize for header and footer
            combined_block->header.block_size = combine->block_size + size_of_page;
            free_footer->block_size = combine->block_size + size_of_page;
            combined_block->body.links.prev->body.links.next = combined_block->body.links.next;
            combined_block->body.links.next->body.links.prev = combined_block->body.links.prev;


            combined_block->body.links.prev = &sf_free_list_head;
            combined_block->body.links.next = sf_free_list_head.body.links.next;
            sf_free_list_head.body.links.next->body.links.prev = combined_block;
            sf_free_list_head.body.links.next = free_block = combined_block;
        }
        else
        {
            free_block->header.block_size = size_of_page + 2;
            free_footer->block_size = size_of_page + 2;
            free_block->body.links.prev = &sf_free_list_head;
            free_block->body.links.next = sf_free_list_head.body.links.next;
            sf_free_list_head.body.links.next->body.links.prev = free_block;
            sf_free_list_head.body.links.next = free_block;
        }
    }
}

void coalesce(void *free_head,int actual_size)
{
    sf_block *free_header = (sf_block *)free_head;
    sf_header *free_footer = (sf_header *)((void *)free_header + actual_size - 8);

    sf_block *next_header = (sf_block *)((void *)free_footer + 8);
    sf_header *prev_footer = (sf_header *) ((void *)free_header - 8);

    int prev_alloc = 0;
    int next_alloc = 0;

    int next_pa = 0;
    int prev_pa = 0;

    if((free_header->header.block_size & 2)== 2)
    {
        prev_alloc = 1;
    }
    if((prev_footer->block_size & 2) == 2)
    {
        prev_pa = 2;
    }
    if((next_header->header.block_size & 1) == 1)
    {
        next_alloc = 1;
    }
    if((next_header->header.block_size & 2 ) == 2)
    {
        next_pa = 2;
    }

    int prev_size = prev_footer->block_size - prev_pa - prev_alloc;
    int next_size = next_header->header.block_size - next_pa - next_alloc;

    sf_header *next_footer = (sf_header *) ((void *)next_header + next_size - 8);
    sf_block *prev_header = (sf_block *) ((void *)prev_footer - prev_size + 8);

    //case 1 neither previous or next block is free
    if(prev_alloc == 1 && next_alloc == 1)
    {
        //insert into main free list
        free_header->body.links.next = sf_free_list_head.body.links.next;
        free_header->body.links.prev = &sf_free_list_head;
        free_header->body.links.next->body.links.prev = free_header;
        sf_free_list_head.body.links.next = free_header;

        //set the allocated bit to 0
        free_header->header.block_size = free_header->header.block_size -1;
        free_header->header.requested_size = 0;

        free_footer->block_size = free_header->header.block_size;
        free_footer->requested_size = 0;

        //set the previous allocated bit of next block to 0
        if((next_header->header.block_size & 2) == 2)
        {
            next_header->header.block_size = next_header->header.block_size - 2;
        }
    }
    //case 2 only previous is free
    if(prev_alloc == 0 && next_alloc == 1 )
    {
        prev_header->header.block_size = prev_header->header.block_size + actual_size;
        free_footer->block_size = prev_header->header.block_size;
        free_footer->requested_size = 0;
        if((next_header->header.block_size & 2) == 2)
        {
            next_header->header.block_size = next_header->header.block_size - 2;
        }

        prev_header->body.links.prev->body.links.next = prev_header->body.links.next;
        prev_header->body.links.next->body.links.prev = prev_header->body.links.prev;

        prev_header->body.links.next = sf_free_list_head.body.links.next;
        prev_header->body.links.prev = &sf_free_list_head;

        sf_free_list_head.body.links.next->body.links.prev = prev_header;
        sf_free_list_head.body.links.next = prev_header;
    }
    //case 3 only next block is free
    if(prev_alloc == 1 && next_alloc == 0)
    {
        free_header->header.block_size = free_header->header.block_size -1 + next_size;
        next_footer->block_size = free_header->header.block_size;

        free_header->header.requested_size = 0;
        next_footer->requested_size = 0;

        next_header->body.links.prev->body.links.next = next_header->body.links.next;
        next_header->body.links.next->body.links.prev = next_header->body.links.prev;

        free_header->body.links.next = sf_free_list_head.body.links.next;
        free_header->body.links.prev = &sf_free_list_head;

        sf_free_list_head.body.links.next->body.links.prev = free_header;
        sf_free_list_head.body.links.next = free_header;
    }
    //case 4 both previous and next are free
    if(prev_alloc == 0 && next_alloc == 0)
    {
        prev_header->header.block_size = prev_header->header.block_size + actual_size + next_size;
        next_footer->block_size = prev_header->header.block_size;

        prev_header->body.links.prev->body.links.next = prev_header->body.links.next;
        prev_header->body.links.next->body.links.prev = prev_header->body.links.prev;

        next_header->body.links.prev->body.links.next = next_header->body.links.next;
        next_header->body.links.next->body.links.prev = next_header->body.links.prev;

        prev_header->body.links.next = sf_free_list_head.body.links.next;
        prev_header->body.links.prev = &sf_free_list_head;

        sf_free_list_head.body.links.next->body.links.prev = prev_header;
        sf_free_list_head.body.links.next = prev_header;
    }
    return;
}

void insert(sf_block *free_ptr, int actual_block_size)
{

    int quick_index = actual_block_size / 16;
    if(quick_index >= 2 && quick_index <= 11)
    {
        // it can be freed into a quick list
        quick_index = quick_index -2;

        if(sf_quick_lists[quick_index].length == QUICK_LIST_MAX)
        {
            //flush the quick list
            while(sf_quick_lists[quick_index].length != 0)
            {
                int quick_size = 32 + quick_index * 16;
                sf_quick_lists[quick_index].length = sf_quick_lists[quick_index].length - 1;
                sf_block *flush_block = sf_quick_lists[quick_index].first;

                sf_quick_lists[quick_index].first = flush_block->body.links.next;

                //coalese into main free list
                coalesce(flush_block,quick_size);
            }
        }
            //insert the free block into quick list
            sf_quick_lists[quick_index].length = sf_quick_lists[quick_index].length + 1;
            free_ptr->body.links.next = sf_quick_lists[quick_index].first;
            sf_quick_lists[quick_index].first = free_ptr;

            free_ptr->header.requested_size = 1000;

            //set footer for free block
            sf_header *free_header = (sf_header *)((void *)free_ptr + actual_block_size - 8);
            free_header->block_size = free_ptr->header.block_size;
            free_header->requested_size = free_ptr->header.requested_size;
    }
    else
    {
        //free block into main list
       coalesce(free_ptr,actual_block_size);
    }
}

void check_valid(void *pp)
{
    sf_block *free_ptr = (sf_block *)(pp-8);
    int actual_block_size = free_ptr->header.block_size & BLOCK_SIZE_MASK;

     //determine if address is before end of prologue or after beginning of epilogue
    if( (void *)free_ptr < (void *)(sf_mem_start()+ sizeof(sf_prologue)) || (void *)free_ptr >  (void *)(sf_mem_end() - sizeof(sf_epilogue)))
    {
        abort();
    }

    //determine if the allocated bit is set
    if((free_ptr->header.block_size & 1) != 1)
    {
        abort();
    }

    //determine if the previous allocated bit is set
    if((free_ptr->header.block_size & 2) == 0)
    {
        sf_header *prev_header = (sf_header *) ((void *)free_ptr - 8);
        int actual_previous_size = prev_header->block_size & BLOCK_SIZE_MASK;

        sf_block *prev_block = (sf_block *) ((void *)free_ptr - actual_previous_size);

        if((prev_block->header.block_size & 1) == 1 || (prev_header->block_size & 1) == 1 ||
            prev_block->header.block_size != prev_header->block_size || prev_block->header.requested_size != prev_header->requested_size)
        {
            abort();
        }
    }
    //determine if blocksize is less than 32 or not a multiple of 16
    if(actual_block_size < 32 || (actual_block_size % 16) != 0)
    {
        abort();
    }
    //determine if requested size+8 is greater than actual blocksize
    if((free_ptr->header.requested_size + 8) > actual_block_size)
    {
        abort();
    }
}


void sf_free(void *pp) {

    if(pp == NULL)
    {
        abort();
    }

    sf_block *free_ptr = (sf_block *)(pp-8);
    int actual_block_size = free_ptr->header.block_size & BLOCK_SIZE_MASK;

    check_valid(pp);

    insert(free_ptr,actual_block_size);
    return;
}

void *sf_realloc(void *pp, size_t rsize) {

    if(pp == NULL)
    {
        abort();
    }

    sf_block *realloc_ptr = (sf_block *)(pp-8);
    int actual_block_size =realloc_ptr->header.block_size & BLOCK_SIZE_MASK;

    check_valid(pp);

    if(rsize == 0)
    {
        free(pp);
        return NULL;
    }

    if(realloc_ptr->header.requested_size < rsize)
    {
        void *new_malloc = sf_malloc(rsize);
        if(new_malloc == NULL)
            return NULL;
        memcpy(new_malloc,pp,rsize);
        sf_free(pp);
        return new_malloc;
    }
    else if (realloc_ptr->header.requested_size >= rsize)
    {
         int size_block = rsize + 8;
        // computing the amount of adding needed to reach double-word alignment
        int remainder = 16 - (size_block % 16);
        if(remainder != 16)
            size_block = size_block + remainder;

        // ensuring that the blocksize is a minimum of 32 bytes
        if(size_block < 32)
            size_block = size_block + 16;

        //split the block
        if((actual_block_size - size_block) >= 32)
        {
            realloc_ptr->header.block_size = size_block + (realloc_ptr->header.block_size - actual_block_size);
            realloc_ptr->header.requested_size = rsize;

            sf_block *free_realloc = (sf_block *)((void *)realloc_ptr + size_block);
            free_realloc->header.block_size = actual_block_size - size_block + 3;

            coalesce(free_realloc,actual_block_size - size_block);
            return pp;
        }
        //block is not split
        else
        {
            realloc_ptr->header.requested_size = rsize;
            return pp;
        }
    }
    return NULL;
}
