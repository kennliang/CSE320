#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "huff.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the nodes of the Huffman tree and other data have
 * been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */


int string_to_int(char *s)
{
    int num = 0;
    while(*s != '\0')
    {
        if(*s >= '0' && *s <= '9')
        {
            //shifts digits to the left by one decimal place and add the current digit
            num = num *10 + *s - '0';
        }
        else
            return 0;
        ++s;
    }
    return num;
}
int compare_string(char *s1,char *s2)
{
    while(*s1 != '\0' || *s1 != '\0')
    {
        if(*s1 == *s2)
        {
            s1++;
            s2++;
        }
        else
            return 0;
    }
    return 1;
}

/**
 * @brief Emits a description of the Huffman tree used to compress the current block.
 * @details This function emits, to the standard output, a description of the
 * Huffman tree used to compress the current block.  Refer to the assignment handout
 * for a detailed specification of the format of this description.
 */
void emit_huffman_tree() {
    // To be implemented.
}

/**
 * @brief Reads a description of a Huffman tree and reconstructs the tree from
 * the description.
 * @details  This function reads, from the standard input, the description of a
 * Huffman tree in the format produced by emit_huffman_tree(), and it reconstructs
 * the tree from the description.  Refer to the assignment handout for a specification
 * of the format for this description, and a discussion of how the tree can be
 * reconstructed from it.
 *
 * @return 0 if the tree is read and reconstructed without error, otherwise 1
 * if an error occurs.
 */
int read_huffman_tree() {
    // To be implemented.
    return 1;
}

/**
 * @brief Reads one block of data from standard input and emits corresponding
 * compressed data to standard output.
 * @details This function reads raw binary data bytes from the standard input
 * until the specified block size has been read or until EOF is reached.
 * It then applies a data compression algorithm to the block and outputs the
 * compressed block to the standard output.  The block size parameter is
 * obtained from the global_options variable.
 *
 * @return 0 if compression completes without error, 1 if an error occurs.
 */
int compress_block() {
    // To be implemented.
    return 1;
}

/**
 * @brief Reads one block of compressed data from standard input and writes
 * the corresponding uncompressed data to standard output.
 * @details This function reads one block of compressed data from the standard
 * inputk it decompresses the block, and it outputs the uncompressed data to
 * the standard output.  The input data blocks are assumed to be in the format
 * produced by compress().  If EOF is encountered before a complete block has
 * been read, it is an error.
 *
 * @return 0 if decompression completes without error, 1 if an error occurs.
 */
int decompress_block() {
    // To be implemented.
    return 1;
}

/**
 * @brief Reads raw data from standard input, writes compressed data to
 * standard output.
 * @details This function reads raw binary data bytes from the standard input in
 * blocks of up to a specified maximum number of bytes or until EOF is reached,
 * it applies a data compression algorithm to each block, and it outputs the
 * compressed blocks to standard output.  The block size parameter is obtained
 * from the global_options variable.
 *
 * @return 0 if compression completes without error, 1 if an error occurs.
 */
int compress()
{
    int i = 0;
    int c;
    //int blocksize = 2048;
/*
    //no input
    if( (c = getchar()) == EOF)
        return 1;
    do
    {
        current_block[i] = c
        if((i+1) == blocksize)
        {
            build_huff();
            i = 0; //reset the counter
        }
        i++;
    }while( (c = getchar()) != EOF);

    // partial block read in due to end of input
    // set the end of the block NULL to indicate where to stop reading input
    if(i != blocksize)
    {
        current_block[i] = NULL;
        build_huff();
    }
    */
    return 1;

}

/**
 * @brief Reads compressed data from standard input, writes uncompressed
 * data to standard output.
 * @details This function reads blocks of compressed data from the standard
 * input until EOF is reached, it decompresses each block, and it outputs
 * the uncompressed data to the standard output.  The input data blocks
 * are assumed to be in the format produced by compress().
 *
 * @return 0 if decompression completes without error, 1 if an error occurs.
 */
int decompress() {
    // To be implemented.
    return 1;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and 1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and 1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    if(argc == 1)
    {
          // no flags are provided display usage and return with an EXIT_FALIURE
         return 1;
    }
    else
    {
        char *first_arg = *++argv;

        if(compare_string(first_arg,"-h"))
        {
            //least significant bit is 1
            global_options =0x1;
            return 0;
        }
        else if(compare_string(first_arg,"-c"))
        {
            //perform data compression
            if(argc == 2)
            {
                //second-least significant bit is 1
                global_options =0xffff0002;
                //printf("%lu %s\n",(long unsigned)global_options,"executed2");
                return 0;
            }
            else if(argc == 4)
            {
                char *second_arg = *++argv;

                if(compare_string(second_arg,"-b"))
                {
                    char *third_arg = *++argv;
                    int block_num = string_to_int(third_arg)-1;

                    if(block_num+1 <= MAX_BLOCK_SIZE && block_num+1 >= MIN_BLOCK_SIZE)
                    {
                        //second least significant bit is blocksize -1 in the 16
                        //most significant bit of global_options
                        //global_options = 0xffff0004;
                        //printf("%d\n",block_num);
                        block_num = block_num << 16;
                        global_options = 0x2;
                        global_options = block_num | global_options;
                        //printf("%d\n",block_num);
                        //printf("%lu %s\n",(long unsigned)global_options,"executed3");
                        return 0;
                    }
                    else
                    {
                        return 1;
                    }
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                return 1;
            }

        }
        else if(compare_string(first_arg,"-d"))
        {
            if(argc != 2)
            {
                return 1; //invalid additional arguments
            }
            else
            {
                //third-most significant bit is 1 for decompression
                global_options =  0xffff0004;
                return 0; //valid
            }
        }
        else
        {
            return 1;
        }
    }
}


int build_huff()
{
    return 1;
}
