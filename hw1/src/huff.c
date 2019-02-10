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

/**
 *  string_to_int --- converts a string into an int and checks if all chars
 *  of the string are digits (0-9). Returns 0 if strings are not digits.
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

/**
 *  compare_string --- a simple string comparison function that checks if two strings
 *  exactly matches. Returns 0 if strings do not match. 1 if strings do match
 */
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
 *  proces_block --- a linear traversal through current_block,an array of unsigned chars
 *  containing the input received from standard input, creating leaf nodes for
 *  every unique symbol and storing the total number of times each symbol appears
 *  in the current_block. Once the current_block is processed, a new leaf node is created
 *  to represent the end of a block with weight 0.
 *
 */

void process_block()
{
    unsigned char *input_block = current_block;
    NODE *nodes_ptr = nodes;
    int i;
    int found = 0;
    num_nodes = 0;

    // processing the entire block
    while(*input_block != '\0')
    {
         NODE *nodes2_array = nodes;
        for(i = 0 ;i < num_nodes;i++)
        {
            // the symbol already exists, increase the count
            if((unsigned char)nodes2_array->symbol == *input_block)
            {
                nodes2_array->weight = nodes2_array->weight + 1;
                found = 1;
            }
            nodes2_array++;
        }
        // a new symbol, create a leaf node
        if(found != 1)
        {
            nodes_ptr->symbol = (short)*input_block;
            nodes_ptr->weight = 1;
            nodes_ptr->parent = NULL;
            nodes_ptr->right = NULL;
            nodes_ptr->left = NULL;

            num_nodes = num_nodes + 1;
            nodes_ptr++;
        }
        input_block++;
        found = 0;
    }
    // adding the end block symbol
    nodes_ptr->symbol = 400;
    nodes_ptr->weight = 0;
    nodes_ptr->parent = NULL;
    nodes_ptr->right = NULL;
    nodes_ptr->left = NULL;
    num_nodes = num_nodes + 1;
}

/**
 *  find_min --- finds the two minimum node weights within the low end of the nodes array
 *  The information of the minimum nodes are stored for later use using a pass by reference.
 *
 *  num_leaf - the number of low end nodes remaining in the nodes array
 */
void find_min(NODE *min,NODE *min2,int num_leaf)
{
    NODE *nodes_ptr = nodes;
    int i;
    NODE *min_node;
    NODE *min_node2;

    // a linear traversal through the low end of the nodes array ito find the two minimum nodes
    for(i = 0; i < num_leaf && num_leaf != 1; i++)
    {
        if(i == 0)
        {
            min_node = nodes;
        }
        else if(i == 1)
        {
            if(min_node->weight > nodes_ptr->weight)
            {
                min_node2 = min_node;
                min_node = nodes_ptr;
            }
            else
            {
                min_node2 = nodes_ptr;
            }
        }
        else
        {
            if(min_node->weight > nodes_ptr->weight && min_node2->weight > nodes_ptr->weight)
            {
                min_node2 = min_node;
                min_node = nodes_ptr;
            }
            else if(min_node2->weight > nodes_ptr->weight)
            {
                min_node2 = nodes_ptr;
            }
        }
        nodes_ptr++;
    }
    // storing the information of the two minimum nodes
    min->symbol = min_node->symbol;
    min->weight = min_node->weight;
    min->left = min_node->left;
    min->right = min_node->right;

    min2->symbol = min_node2->symbol;
    min2->weight = min_node2->weight;
    min2->left = min_node2->left;
    min2->right = min_node2->right;
}

/**
 *  shift method --- removes the two minimum nodes found in the low end of the nodes array
 *  Shifts the remaining nodes in the array to maintain a contiguous set of nodes
 *
 *  num_leaf is the number of nodes remaining in the low end of the array
 */

void shift(NODE min,NODE min2,int num_leaf)
{
    int found = 0;
    int i;

    //reference to the current node address and next node address
    NODE *nodes_ptr = nodes;
    NODE *next = ++nodes_ptr;
    nodes_ptr = nodes;

    // removing and shifting from the minimum node
    for(i = 0; i < num_leaf;i++)
    {
        if(nodes_ptr->symbol == min.symbol || found == 1)
        {
            nodes_ptr->weight = next->weight;
            nodes_ptr->symbol = next->symbol;
            nodes_ptr->left = next->left;
            nodes_ptr->right = next->right;
            nodes_ptr->parent = next->parent;
            found = 1;
        }
        nodes_ptr++;
        next++;
    }

    //reference to the current node address and next node address
    nodes_ptr = nodes;
    next = ++nodes_ptr;
    nodes_ptr = nodes;
    found = 0;

    // removing and shifting from the second minimum node
    for(i = 0; i < num_leaf-1;i++)
    {
        if(nodes_ptr->symbol == min2.symbol || found == 1)
        {
            nodes_ptr->weight = next->weight;
            nodes_ptr->symbol = next->symbol;
            nodes_ptr->left = next->left;
            nodes_ptr->right = next->right;
            nodes_ptr->parent = next->parent;
            found = 1;
        }
        nodes_ptr++;
        next++;
    }
}

/**
 *  construct_tree --- places the two minimum nodes previously found into node[2*num_leaf-2]
 *  and node[2*num_leaf-3]. The parent node, the sum of the two minimum nodes is placed into
 *  node[num_leaf-2].
 *
 *  nun_leaf - number of nodes remaining in the low end of the array.
 */
void construct_tree(NODE min,NODE min2,int num_leaf)
{
    //placing the two min nodes and parent node into the correct array position
    //plaing the parent node to the correct array index
    NODE *nodes_ptr = nodes+(num_leaf-2);
    NODE *parent_index;
    nodes_ptr->symbol = 500;
    nodes_ptr->weight = min2.weight + min.weight;
    nodes_ptr->parent = NULL;
    parent_index = nodes_ptr;

    //placing the second min node to the correct array index
    nodes_ptr = nodes+(2*num_leaf-3);
    nodes_ptr->weight = min2.weight;
    nodes_ptr->symbol = min2.symbol;
    nodes_ptr->left = min2.left;
    nodes_ptr->right = min2.right;
    //update parent node right pointer
    parent_index->right = nodes_ptr;

    //placing the minimum node to the correct array index
    nodes_ptr++;
    nodes_ptr->weight = min.weight;
    nodes_ptr->symbol = min.symbol;
    nodes_ptr->left = min.left;
    nodes_ptr->right = min.right;
    //update parent left pointer
    parent_index->left = nodes_ptr;
}

int build_huff()
{

    process_block();

     //setting the total number of nodes our tree can have
    num_nodes = (2*num_nodes)-1;

    //number of nodes in the low end of tree (initially number of leaf nodes)
    int num_leaf = (num_nodes+1)/2;

    //leaf nodes are now in the array
    while(num_leaf != 1)
    {
        NODE min;
        NODE min2;

        // finding the two min nodes in the array
        find_min(&min,&min2,num_leaf);

        // removes the two minimum nodes and shifts remaining nodes
        shift(min,min2,num_leaf);

        // place the two removed nodes and parent to the correct index of the array
        // setting the left and right child pointers of parent nodes
        construct_tree(min,min2,num_leaf);

        //repeat until 1 node in lower end is left (root node)
        num_leaf = num_leaf-1;
    }
    // traversing through the nodes array to correctly set the parent pointers of each node
    // and setting pointers to the leaf nodes in the array
    int i;
    NODE *nodes_ptr = nodes;
    NODE **node_symbol = node_for_symbol;
    for(i = 0 ;i < num_nodes;i++)
    {
        if(nodes_ptr->left == NULL && nodes_ptr->right == NULL)
        {
            //leaf node - assign pointer!
            *node_symbol++ = nodes_ptr;
        }
        if(nodes_ptr->left != NULL)
        {
            NODE *p = nodes_ptr->left;
            p->parent = nodes_ptr;
        }
        if(nodes_ptr->right != NULL)
        {
            NODE *p = nodes_ptr->right;
            p->parent = nodes_ptr;;
        }
        nodes_ptr++;
    }

    num_leaf = (num_nodes+1)/2;
    node_symbol = node_for_symbol;
/*
    for(i = 0; i <num_leaf;i++)
    {
        NODE *node_p = *node_symbol;
        printf("%c %d %s\n\n",node_p->symbol,node_p->weight," is the leaf node");
        node_symbol++;
    }
    printf("\n");

    node_symbol = node_for_symbol;
    */
    emit_huffman_tree();

    int big_num = 1000000000;
    //printf("%d\n",big_num);
    unsigned char *input_block = current_block;
    int found = 0;
    int bit_count = 0;
    int bit_num = 0;
    while(*input_block != '\0')
    {
        for(i = 0 ;i < num_leaf && found == 0;i++)
        {
            NODE *node_p = *node_symbol;
            //printf("%c %c\n",(unsigned char)node_p->symbol,*input_block);
            if((unsigned char)node_p->symbol == *input_block)
            {
                //printf("yay match\n");
                node_p->weight = big_num;
                while(node_p->parent != NULL)
                {
                    node_p = node_p->parent;
                    node_p->weight = big_num;
                }
                //node_p should be root now
                while(node_p->left != NULL && node_p->right != NULL)
                {
                    if(node_p->left != NULL && (node_p->left)->weight == big_num)
                    {
                        node_p = node_p->left;
                        bit_num = bit_num | 0;
                        bit_count++;
                        if(bit_count == 8)
                        {
                            printf("%c",(unsigned char)bit_num);
                            bit_count = 0;
                            bit_num = 0;
                        }
                        else
                        {
                            bit_num = bit_num << 1;
                        }
                    }
                    else if(node_p->right != NULL && (node_p->right)->weight == big_num)
                    {
                        node_p = node_p->right;
                        bit_num = bit_num | 1;
                        bit_count++;
                        if(bit_count == 8)
                        {
                            printf("%c",(unsigned char)bit_num);
                            bit_count = 0;
                            bit_num = 0;
                        }
                        else
                        {
                            bit_num = bit_num << 1;
                        }
                    }
                }
                big_num++;
                found = 1;
            }
            else
            {
                //printf("no match\n");
                node_symbol++;
            }
        }
        input_block++;
        node_symbol = node_for_symbol;
        found = 0;
        if(*input_block == '\0')
        {
            for(i = 0; i < num_leaf && found == 0;i++)
            {
                NODE *node_p = *node_symbol;
                if(node_p->symbol == 400)
                {
                    node_p->weight = big_num;
                    while(node_p->parent != NULL)
                    {
                        node_p = node_p->parent;
                        node_p->weight = big_num;
                    }
                    //node_p should be root now
                    while(node_p->left != NULL && node_p->right != NULL)
                    {
                        if(node_p->left != NULL && (node_p->left)->weight == big_num)
                        {
                            node_p = node_p->left;
                            bit_num = bit_num | 0;
                            bit_count++;
                            if(bit_count == 8)
                            {
                                printf("%c",(unsigned char)bit_num);
                                bit_count = 0;
                                bit_num = 0;
                            }
                            else
                            {
                                bit_num = bit_num << 1;
                            }
                        }
                        else if(node_p->right != NULL && (node_p->right)->weight == big_num)
                        {
                            node_p = node_p->right;
                            bit_num = bit_num | 1;
                            bit_count++;
                            if(bit_count == 8)
                            {
                                printf("%c",(unsigned char)bit_num);
                                bit_count = 0;
                                bit_num = 0;
                            }
                            else
                            {
                                bit_num = bit_num << 1;
                            }
                        }
                    }
                    found = 1;
                    //printf("\n%d\n",bit_count);

                    //if(bit_count != 0 && bit_count != 8)
                    //{
                        while(bit_count != 8 && bit_count != 0)
                        {
                            bit_num = bit_num | 0;
                            //bit_num = bit_num << 1;
                            bit_count++;
                            if(bit_count == 8)
                            {
                                printf("%c",(unsigned char)bit_num);
                            }
                            else
                            {
                                bit_num = bit_num << 1;
                            }
                        }
                    //}
                }
                else
                {
                    //printf("no match\n");
                    node_symbol++;
                }
            }
        }
    }



    return 1;
}

void postorder(NODE *node,int *byte_count,int *display)
{
    if(node == NULL)
        return;

    postorder(node->left,byte_count,display);

    postorder(node->right,byte_count,display);

    if(node->left == NULL && node->right == NULL)
    {
        //printf("0 ");
        //printf("%d\n",*byte_count);
        *byte_count = *byte_count + 1;
        //printf("%d\n",*byte_count);
        *display = *display | 0;
        if(*byte_count == 8)
        {
            printf("%c",(unsigned char)*display);
            *byte_count = 0;
            *display = 0;
        }
        else
        {
            *display = *display << 1;
        }
    }
    else
    {
        //printf("1 ");
        *byte_count = *byte_count + 1;
        *display = *display | 1;
        if(*byte_count == 8)
        {
            printf("%c",(unsigned char)*display);
            *byte_count = 0;
            *display = 0;
        }
        else
        {
            *display = *display << 1;
        }
    }
}

/**
 * @brief Emits a description of the Huffman tree used to compress the current block.
 * @details This function emits, to the standard output, a description of the
 * Huffman tree used to compress the current block.  Refer to the assignment handout
 * for a detailed specification of the format of this description.
 */
void emit_huffman_tree() {
    // To be implemented.
    int number = num_nodes;
    unsigned int number_1 = number << 24;
    number_1 = number_1 >> 24;
    unsigned int number_2 = number >> 8;
    printf("%c%c",(unsigned char)number_2,(unsigned char)number_1);
    NODE **p = node_for_symbol;
    NODE *root = *p;
    while(root->parent != NULL)
    {
        root = root->parent;
    }
    int byte_count = 0;
    int display = 0;
    postorder(root,&byte_count,&display);
    //printf("%d %d\n",display,byte_count);

    while(byte_count != 0 && byte_count != 8)
    {
        display = display | 0;
        byte_count++;
        if(byte_count != 8)
            display = display << 1;
    }
    printf("%c",(unsigned char)display);

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
    unsigned char *input_block = current_block;
    //printf("%d %s\n",global_options, "is the global_options");
    unsigned int blocksize = global_options - 0x2;
    //printf("%d %s\n",blocksize, "is the blocksize");
    blocksize = (blocksize >> 16) + 1;
    //printf("%d %s\n",blocksize, "is the blocksize");


    //no input
    if( (c = getchar()) == EOF)
        return 1;
    do
    {
        *input_block = (unsigned char)c;
        //printf("%c\n",(unsigned char)c);
        if((i+1) == blocksize)
        {
            //printf("%d %s\n",blocksize, "is the max");
            build_huff();
            i = 0; //reset the counter
            input_block = current_block;
        }
        else
        {
            i++;
            input_block++;
        }
    }while( (c = getchar()) != EOF);

    // partial block read in due to end of input
    // set the end of the block NULL to indicate where to stop reading input
    if(i != blocksize && i != 0)
    {
        //printf("%d %d\n",blocksize,i);
        *input_block = '\0';
        build_huff();
    }

    // testing if input was processed correctly from stdin
    //input_block = current_block;
    //printf("executed\n");
    /*
    while(*input_block != '\0')
    {
        printf("%c",*input_block);
        input_block++;
    }
    printf("\n");
    */

    return 0;

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



