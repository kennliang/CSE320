#include <stdio.h>
#include <stdlib.h>

#include "const.h"
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

int main(int argc, char **argv)
{
    int ret;
    // if returns 0 then doesn't execute
    if(validargs(argc, argv))
    {
        USAGE(*argv, EXIT_FAILURE);
    }
    debug("Options: 0x%x", global_options);

    // h flag provided
    if((global_options & 1))
    {
        USAGE(*argv, EXIT_SUCCESS);
        //printf("%s\n","executed");
    }
    // c flag provided
    else if(global_options & 0x2)
    {
        ret = compress();
        if(ret)
            return EXIT_FAILURE;
        //printf("%s\n","executed");
        //USAGE(*argv, EXIT_SUCCESS);
    }
    // d flag provided
    else if(global_options & 0x4)
    {
/*
        int number = 7;
        unsigned int number_1 = number << 24;
        number_1 = number_1 >> 24;
        unsigned int number_2 = number >> 8;
        printf("%c%c",(unsigned char)number_2,(unsigned char)number_1);
        printf("%c%c%c%c",'a',(unsigned char)4,'n','g');
        */


        //read_huffman_tree();
        //printf("exeucted");
        //decompress_block();
        //printf("exeucted");

        //printf("exeucted");
        ret = decompress();
        //printf("\n%d\n",ret);
        if(ret)
            return EXIT_FAILURE;

        //printf("%d\n",result);
    }
    // printf("%s\n","executed");
     return EXIT_SUCCESS;
}


/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
