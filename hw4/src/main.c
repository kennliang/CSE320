#include <stdlib.h>
#include <stdio.h>
#include "cookbook.h"
#include "debug.h"

#include<unistd.h>
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


    extern char *optarg ;
    extern int opterr;
    opterr = 0;

    int max_cooks = 1;
    char *filename = "cookbook.ckb";
    char *main_recipe_name = NULL;


    int c;
    while ((c = getopt (argc, argv, ":c:f:")) != -1)
    {
        switch(c)
        {
            //name of file for recipe
            case 'f':
                //debug("optarg%s",optarg);
                if(strncmp(optarg,"-",1))
                    filename = optarg;
                break;
            //maximum number of cooks available
            case 'c':
                //debug("optarg%s",optarg);
                max_cooks = string_to_int(optarg);
                if(max_cooks == 0)
                    return EXIT_FAILURE;
                break;
            case ':':
                debug("Error no arguments provided");
                return EXIT_FAILURE;
                break;
            case '?':
                debug("Error with processing arguments.(invalid flags)");
                return EXIT_FAILURE;
                break;

        }
    }

    if(optind < argc)
    {
        //printf("Non-option args: ");
        main_recipe_name = argv[optind++];
       // debug("Main_recipe = %s",argv[optind++]);
        if(optind < argc)
            //additional non-option arguments to be handled how? exit?
            debug("extra_non_option_arguments_%s ", argv[optind++]);
    }
    /*
    debug("%d",opterr);
    debug("File_name = %s",filename);
    debug("Max_cooks = %d",max_cooks);
    */
    //REMOVE THIS LINE LATER@@@@@@@@@@@@@@@@@@@@@@@
    filename++;

    COOKBOOK *cbp;
    int err = 0;
    FILE *in = fopen("rsrc/eggs_benedict.ckb", "r");
    cbp = parse_cookbook(in, &err);
    if(err)
    {
	   fprintf(stderr, "Error parsing cookbook\n");
	   exit(1);
    }
    if(process_recipe(cbp,main_recipe_name,max_cooks))
        return 1;

    /*
    else
    {
	   unparse_cookbook(cbp, stdout);
    }
    exit(1);
    */

    return 0;
}
