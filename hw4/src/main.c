#include <stdlib.h>
#include <stdio.h>
#include "cookbook.h"
#include "debug.h"
#include <getopt.h>
#include <string.h>

int process_recipe();

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

int main(int argc, char *argv[]) {

    char *last = NULL;
    if(argc != 1)
    {
     last = argv[argc-1];
    }

    extern char *optarg ;
   // extern int opterr;
   // opterr = 0;

    int dup_f = 0;
    int dup_c = 0;

    int max_cooks = 1;
    char *filename = "cookbook.ckb";
    char *main_recipe_name = NULL;

    int c;
    while ((c = getopt (argc, argv, ":c:f:")) != -1)
    {
        debug("optind = %d argc = %d",optind,argc);
        debug("optarg = %s",optarg);
        switch(c)
        {
            //name of file for recipe
            case 'f':
                dup_f++;
                if(dup_f == 2)
                {
                    fprintf(stderr, "%s\n","Duplicate f flag was entered" );
                    exit(1);
                }
                filename = optarg;
                break;
            //maximum number of cooks available
            case 'c':
                dup_c++;
                 if(dup_c == 2)
                {
                    fprintf(stderr, "%s\n","Duplicate c flag was entered" );
                    exit(1);
                }
                max_cooks = string_to_int(optarg);
                if(max_cooks == 0)
                    exit(1);
                break;
            case ':':
                fprintf(stderr, "%s\n", "Error no arguments provided to flags");
                exit(1);
                break;
            case '?':
                fprintf(stderr, "%s\n", "Error with processing arguments.(invalid flags)");
                exit(1);
                break;

        }
    }

    if(optind < argc)
    {
        main_recipe_name = argv[optind++];
        if(strcmp(main_recipe_name,last) != 0)
        {
            fprintf(stderr, "%s\n", "The main recipe argument was not the last argument" );
            exit(1);
        }

        debug("Main_recipe = %s",main_recipe_name);
        if(optind < argc)
        {
            fprintf(stderr, "%s\n", "An additional non-option argument was passed");
            exit(1);
        }
    }
    debug("main_recipe_name = %s",main_recipe_name);
    debug("File_name = %s",filename);
    debug("Max_cooks = %d",max_cooks);

    //REMOVE THIS LINE LATER@@@@@@@@@@@@@@@@@@@@@@@
    filename++;


    COOKBOOK *cbp;
    int err = 0;


    //  REMEMBER TO CHANGE THIS TO THE CORRECT REQUIREMENTS//
    FILE *in = fopen("rsrc/eggs_benedict.ckb", "r");



    cbp = parse_cookbook(in, &err);
    if(err)
    {
	   fprintf(stderr, "Error parsing cookbook\n");
	   exit(1);
    }
    if(process_recipe(cbp,main_recipe_name,max_cooks))
        exit(1);

    exit(0);
}
