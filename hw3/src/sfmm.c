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

    //printf("size%ld\n",size);

     // initialize memory - MUST BE CALLED BEFORE USING ANY OTHER FUNCTIONS IN sfutil.o
    //sf_mem_init();

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

   // printf("size of the block%d\nrequested size %ld\n",size_block,size);

    // @@@@@@@@@@@@@@@@@@@@@@ storing the information into a header and displaying to see if it is correct @@@@@@@@@@@@@@@
   // sf_header header = {size_block, size};
    //sf_show_header(&header);


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

        //printf("%p\n%p\n%p\n",free_block->body.payload,free_block + 8,free_block);
        //printf("%ld\n",sizeof(sf_header));

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

   // printf("\nCall to malloc after initialization\n");
   // sf_show_heap();

    //set while loop for all code below here. reason - program request size that needs multiple pages to fit

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

                    //check if previous block is allocated to set prev alloc bit
                    //sf_header *prev = (sf_header *)((void *)match - 8);
                    //if((prev->block_size & 1) == 1)
                        //match->header.block_size = match->header.block_size + 2;

                    //set the allocated bit to 1 and requested size
                    //match->header.block_size = match->header.block_size + 1;
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
                //printf("sf_free_list\n%p\n%p\n",&sf_free_list_head,sf_free_list_head.body.links.prev);

                //check if previous block allocated
                //int previous_alloc = 1;
                //sf_header *find_prev = (sf_header *)((void *)find - 8);
               // if((find_prev->block_size & 1) == 1)
                   // previous_alloc = 1;
                int prev_alloc = 0;
                if((find->header.block_size & 2) == 2)
                        prev_alloc = 2;

                int split = find->header.block_size - size_block - prev_alloc;
                //split the block -  does not create block sizes less than 32 bytes
                if(split >= 32)
                {
                   // printf("blocksize%d\nsize_block%d\nprev_alloc%d\n",find->header.block_size,size_block,prev_alloc);
                    //setting the block_size and allocated bit of the allocated block

                    find->header.block_size = size_block + 1 + prev_alloc;
                    //printf("previous_alloc%d\n",previous_alloc);
                    //if(previous_alloc == 1)
                    //{
                        //printf("exec@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@22uted");
                      //  find->header.block_size = find->header.block_size + 2;
                    //}

                    //set the header and footer for the split free block in main list
                    void *split_block = (void *)find + size_block;
                    //split_block = split_block + size_block;
                    //printf("%p\n%d\n%p\n",split_block,size_block,find);


                    sf_block *new_block = (sf_block *)split_block;
                    //ensure the requested size bits will be same as footer requested size
                    new_block->header.requested_size = 0;
                    //set the previous allocated block
                    new_block->header.block_size = split + 2;
                    //printf("%d\n",split);

                    //add the new block into the main free list
                    new_block->body.links.next = sf_free_list_head.body.links.next;
                    new_block->body.links.prev = &sf_free_list_head;
                    sf_free_list_head.body.links.next = new_block;

                    sf_header *new_footer = (sf_header *)(split_block + split - sizeof(sf_header));
                    //printf("%p\n",new_footer);

                    //same information as header above
                    new_footer->requested_size = 0;
                   // printf("split%d\n",split);
                    new_footer->block_size = split + 2;

                }
                else
                {
                    //if(previous_alloc == 1)
                        //find->header.block_size = find->header.block_size + 2;
                    find->header.block_size = find->header.block_size + 1;
                }
                find->header.requested_size = size;
               //sf_show_heap();
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
        //printf("%p\n",new_mem);

        //setting the free block of new page (replace old epilogue with header of free block)
        sf_block *free_block = (sf_block *) (new_mem - sizeof(sf_header));

        //setting the initial free block footer
        sf_header *free_footer = sf_mem_end() -16;


        //setting the epilogue (8 bytes)
        sf_epilogue * epil = (sf_epilogue *)(sf_mem_end() - sizeof(sf_header));
        //printf("epilogue%p\n",epil);
        epil->header.block_size = 1;

        sf_header *combine = (sf_header *)((void *)free_block - sizeof(sf_header));
        int add = 0;
        if((combine->block_size % 16) != 0)
            add = combine->block_size % 16;

        sf_block *combined_block = (sf_block *)((void *)free_block - combine->block_size + add);
       // printf("%p\n%p\n",combine,combined_block);
        if((combine->block_size & 1) != 1 && (combined_block->header.block_size & 1) != 1
            && combined_block->header.block_size == combine->block_size && combined_block->header.requested_size == combine->requested_size)
        {
            //valid free block to coalesce with
            //setting new blocksize for header and footer
            //printf("executed@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            combined_block->header.block_size = combine->block_size + size_of_page;
            free_footer->block_size = combine->block_size + size_of_page;
        }
        else
        {
            free_block->header.block_size = size_of_page + 2;
            free_footer->block_size = size_of_page + 2;
            free_block->body.links.prev = &sf_free_list_head;
            free_block->body.links.next = sf_free_list_head.body.links.next;
            sf_free_list_head.body.links.next = free_block;
        }
        // sf_show_heap();
       // printf("\nHeap is extended\n");
        //sf_show_heap();
    }
}

void coalesce(void *free_head,int actual_size)
{

    //sf_show_heap();
    sf_block *free_header = (sf_block *)free_head;
    sf_header *free_footer = (sf_header *)((void *)free_header + actual_size - 8);
    //printf("\nfreeheader%p\nfreefooter%p\n",free_header,free_footer);

    sf_block *next_header = (sf_block *)((void *)free_footer + 8);
    sf_header *prev_footer = (sf_header *) ((void *)free_header - 8);

   // printf("\nnextheader%p\nprevfooter%p\n",next_header,prev_footer);

    int prev_alloc = 0;
    int next_alloc = 0;

    int next_pa = 0;
    int prev_pa = 0;
    //printf("\nprev_block%d\n",prev_footer->block_size);

    //if((prev_footer->block_size & 1) == 1)
    if((free_header->header.block_size & 2)== 2)
    {
        //prev_size = prev-size - 1;
        prev_alloc = 1;
    }
    if((prev_footer->block_size & 2) == 2)
    {
        prev_pa = 2;
    }
    if((next_header->header.block_size & 1) == 1)
    {
       // next_size = next_size - 1;
        next_alloc = 1;

    }
     if((next_header->header.block_size & 2 ) == 2)
    {
        next_pa = 2;
        //printf(" ");
    }
    else
        printf("\ninvalid previous allocated bit in the next block PROBLEM!!!!\n");
    //sf_show_heap();

    int prev_size = prev_footer->block_size - prev_pa - prev_alloc;
    int next_size = next_header->header.block_size - next_pa - next_alloc;

    sf_header *next_footer = (sf_header *) ((void *)next_header + next_size - 8);
    sf_block *prev_header = (sf_block *) ((void *)prev_footer - prev_size + 8);

    //case 1 neither previous or next block is free
   // printf("\n%d\n%d\n",prev_alloc,next_alloc);
   // printf("%d\n%d\n",next_header->header.block_size,prev_footer->block_size);
    if(prev_alloc == 1 && next_alloc == 1)
    {

       // printf("@@@@@@@@@@");
        //printf("\n%d\n",free_header->header.block_siz);
        //insert into main free list
       // sf_show_heap();
        free_header->body.links.next = sf_free_list_head.body.links.next;
        free_header->body.links.prev = &sf_free_list_head;
        free_header->body.links.next->body.links.prev = free_header;
        sf_free_list_head.body.links.next = free_header;

      //  sf_show_heap();

        //set the allocated bit to 0
       // if((free_header->header.block_size & 1) == 0)
           // printf("ERROR free block has allocated bit set to 0 already");
        free_header->header.block_size = free_header->header.block_size -1;
        free_header->header.requested_size = 0;

        //sf_show_heap();
        //if((free_footer->block_size & 1) == 0)
           // printf("Error free footer has allcoated bit set to 0 already");
       free_footer->block_size = free_header->header.block_size;
        free_footer->requested_size = 0;

        //set the previous allocated bit of next block to 0
        if((next_header->header.block_size & 2) == 2)
        {
            //printf("Error next header has previous allocated bit set already FIX!!");
            next_header->header.block_size = next_header->header.block_size - 2;
        }
        //sf_show_heap();
    }
    //case 2 only previous is free
    if(prev_alloc == 0 && next_alloc == 1 )
    {

        //sf_show_heap();
        prev_header->header.block_size = prev_header->header.block_size + actual_size;
        free_footer->block_size = prev_header->header.block_size;
        free_footer->requested_size = 0;
        if((next_header->header.block_size & 2) == 2)
        {
            //printf("Error next header has previous allocated bit set already FIX!!");
            next_header->header.block_size = next_header->header.block_size - 2;
        }

        sf_block *find = sf_free_list_head.body.links.next;
        int found = 0;
        while(find != &sf_free_list_head && found == 0)
        {
            if(find == prev_header)
            {
                find->body.links.prev->body.links.next = find->body.links.next;
                find->body.links.next->body.links.prev = find->body.links.prev;

                prev_header->body.links.next = sf_free_list_head.body.links.next;
                prev_header->body.links.prev = &sf_free_list_head;

                sf_free_list_head.body.links.next->body.links.prev = prev_header;
                sf_free_list_head.body.links.next = prev_header;

                found = 1;

            }
            find = find->body.links.next;
        }

       // sf_show_heap();



    }
    //case 3 only next block is free
    if(prev_alloc == 1 && next_alloc == 0)
    {
       // sf_show_heap();
        //printf("next_size%d\n",next_size);
        //printf("next_footer%p\nprev_header%p",next_footer,prev_header);

        free_header->header.block_size = free_header->header.block_size -1 + next_size;
        next_footer->block_size = free_header->header.block_size;

        free_header->header.requested_size = 0;
        next_footer->requested_size = 0;

        sf_block *find = sf_free_list_head.body.links.next;
        int found = 0;
        while(find != &sf_free_list_head && found == 0)
        {
            if(find == next_header)
            {
                find->body.links.prev->body.links.next = find->body.links.next;
                find->body.links.next->body.links.prev = find->body.links.prev;

                free_header->body.links.next = sf_free_list_head.body.links.next;
                free_header->body.links.prev = &sf_free_list_head;

                sf_free_list_head.body.links.next->body.links.prev = free_header;
                sf_free_list_head.body.links.next = free_header;

                found = 1;

            }
            find = find->body.links.next;
        }

        //sf_show_heap();
    }
    //case 4 both previous and next are free
    if(prev_alloc == 0 && next_alloc == 0)
    {
       // sf_show_heap();

        prev_header->header.block_size = prev_header->header.block_size + actual_size + next_size;
        next_footer->block_size = prev_header->header.block_size;

        int found = 0;
        int found2 = 0;

         sf_block *find = sf_free_list_head.body.links.next;
         while(find != &sf_free_list_head && (found == 0 || found2 == 0))
        {
            if(find == prev_header)
            {
                find->body.links.prev->body.links.next = find->body.links.next;
                find->body.links.next->body.links.prev = find->body.links.prev;

                found = 1;

            }
            else if(find == next_header)
            {

                find->body.links.prev->body.links.next = find->body.links.next;
                find->body.links.next->body.links.prev = find->body.links.prev;
                found2 = 1;
            }
            find = find->body.links.next;
        }
        if(found == 1 && found2 == 1)
        {
            prev_header->body.links.next = sf_free_list_head.body.links.next;
            prev_header->body.links.prev = &sf_free_list_head;

            sf_free_list_head.body.links.next->body.links.prev = prev_header;
            sf_free_list_head.body.links.next = prev_header;
        }
       // sf_show_heap();
    }

   // sf_show_heap();
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
//do this in coalesce
/*
                flush_block->header.block_size = flush_block->header.block_size -1;


                sf_header *flush_footer = (sf_header *)((void *)flush_block + quick_size - 8);
                flush_footer->block_size = flush_block->header.block_size;
                */
                sf_quick_lists[quick_index].first = flush_block->body.links.next;


                //coalese in main free list??????????????????????????????????????????????????
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
       // printf("free_ptr%p\nsize_block%d\n",free_ptr,actual_block_size);
       coalesce(free_ptr,actual_block_size);
      // sf_show_heap();
     //  printf("\n11111111111111111111111111111111111111111111111111111111111111111111\n");
    }

}


void sf_free(void *pp) {

    if(pp == NULL)
    {
         printf("3executed");
        abort();
    }

    sf_block *free_ptr = (sf_block *)(pp-8);
    int actual_block_size = free_ptr->header.block_size;

     //determine if address is before end of prologue or after beginning of epilogue+32
    //int free_address = sf_mem_end()- sizeof(sf_epilogue)+32;
    //printf("free_addresss%d\n",free_address);
    if( (void *)free_ptr < (void *)(sf_mem_start()+ sizeof(sf_prologue)) || (void *)free_ptr >  (void *)(sf_mem_end() - sizeof(sf_epilogue)))
    {
        printf("#$@$Q@#RWDAE");
        abort();
    }

    //determine if the allocated bit is set
    if((free_ptr->header.block_size & 1) != 1)
    {
        printf("size_check%d\n",free_ptr->header.block_size);
        printf("2executed");
        abort();
    }
    else
        actual_block_size = actual_block_size -1;

    //determine if address is before end of prologue or after beginning of epilogue+32
    //int free_address = sf_mem_end()- sizeof(sf_epilogue)+32;
    //printf("free_addresss%d\n",free_address);
    if( (void *)free_ptr < (void *)(sf_mem_start()+ sizeof(sf_prologue)) || (void *)free_ptr >  (void *)(sf_mem_end() - sizeof(sf_epilogue)))
    {
        printf("#$@$Q@#RWDAE");
        abort();
    }

    //determine if the previous allocated bit is set
   // sf_show_heap();

   // printf("free_ptr%p\n",free_ptr);

    if((free_ptr->header.block_size & 2) == 0)
    {
        sf_header *prev_header = (sf_header *) ((void *)free_ptr - 8);
        int actual_previous_size = prev_header->block_size;
        if((actual_previous_size & 1) == 1)
            actual_previous_size = actual_previous_size -1;
        if((actual_previous_size & 2) == 2)
            actual_previous_size = actual_previous_size -2;

        //printf("actual_prev_size%d\n",actual_previous_size);

        sf_block *prev_block = (sf_block *) ((void *)free_ptr - actual_previous_size);

       // printf("prev_footer_blocksize%d\nprevious_header_blocksize%d\nprevious_footer_Requested%d\nprevious_header_requested%d\n",
            //prev_header->block_size,prev_block->header.block_size,prev_header->requested_size,prev_block->header.requested_size);


        if((prev_block->header.block_size & 1) == 1 || (prev_header->block_size & 1) == 1 ||
            prev_block->header.block_size != prev_header->block_size || prev_block->header.requested_size != prev_header->requested_size)
        {
             printf("1executed");
            abort();
        }
    }
    else
    {
        actual_block_size = actual_block_size -2;
    }

    //determine if blocksize is less than 32 or not a multiple of 16
    if(actual_block_size < 32 || (actual_block_size % 16) != 0)
    {
       printf("blockwise%d\n",actual_block_size);
        abort();
    }
    //determine if requested size+8 is greater than actual blocksize
    if((free_ptr->header.requested_size + 8) > actual_block_size)
    {
        printf("#######################");
        abort();
    }


    insert(free_ptr,actual_block_size);




    return;
}

void *sf_realloc(void *pp, size_t rsize) {

    printf("rsize%ld\n",rsize);

    if(pp == NULL)
    {
         printf("3executed");
        abort();
    }

    sf_block *realloc_ptr = (sf_block *)(pp-8);
    int actual_block_size =realloc_ptr->header.block_size;

    //determine if the allocated bit is set
    if((realloc_ptr->header.block_size & 1) != 1)
    {
        printf("2execut324rwedafged");
        abort();
    }
    else
        actual_block_size = actual_block_size -1;

    if((realloc_ptr->header.block_size & 2) == 0)
    {
        sf_header *prev_header = (sf_header *) ((void *)realloc_ptr - 8);
        int actual_previous_size = prev_header->block_size;
        if((actual_previous_size & 1) == 1)
            actual_previous_size = actual_previous_size -1;
        if((actual_previous_size & 2) == 2)
            actual_previous_size = actual_previous_size -2;

        //printf("actual_prev_size%d\n",actual_previous_size);

        sf_block *prev_block = (sf_block *) ((void *)realloc_ptr - actual_previous_size);

       // printf("prev_footer_blocksize%d\nprevious_header_blocksize%d\nprevious_footer_Requested%d\nprevious_header_requested%d\n",
            //prev_header->block_size,prev_block->header.block_size,prev_header->requested_size,prev_block->header.requested_size);

        if((prev_block->header.block_size & 1) == 1 || (prev_header->block_size & 1) == 1 ||
            prev_block->header.block_size != prev_header->block_size || prev_block->header.requested_size != prev_header->requested_size)
        {
             printf("1executed");
            abort();
        }
    }
    else
    {
        actual_block_size = actual_block_size -2;
    }

    //determine if blocksize is less than 32 or not a multiple of 16
    if(actual_block_size < 32 || (actual_block_size % 16) != 0)
    {
       printf("blockwise%d\n",actual_block_size);
        abort();
    }
    //determine if requested size+8 is greater than actual blocksize
    if((realloc_ptr->header.requested_size + 8) > actual_block_size)
    {
        printf("#######################");
        abort();
    }

    //determine if address is before end of prologue or after beginning of epilogue+32
    //int free_address = sf_mem_end()- sizeof(sf_epilogue)+32;
    //printf("free_addresss%d\n",free_address);
    if( (void *)realloc_ptr < (void *)(sf_mem_start()+ sizeof(sf_prologue)) || (void *)realloc_ptr >  (void *)(sf_mem_end() - sizeof(sf_epilogue) + 32))
    {
        printf("#$@$Q@#RWDAE");
        abort();
    }

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
    else if (realloc_ptr->header.requested_size > rsize)
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
            printf("size_block%d\nactual_block_Size%d\n",size_block,actual_block_size);
            realloc_ptr->header.block_size = size_block + (realloc_ptr->header.block_size - actual_block_size);
            realloc_ptr->header.requested_size = rsize;
            printf("realloc_size%d\n",realloc_ptr->header.block_size);

            sf_block *free_realloc = (sf_block *)((void *)realloc_ptr + size_block);
            free_realloc->header.block_size = actual_block_size - size_block + 3;
            printf("free_size%d\n",free_realloc->header.block_size);

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
