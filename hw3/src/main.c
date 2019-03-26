#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    //testing no coalese
/*
    double* ptr = sf_malloc(505 * sizeof(double));
    *ptr = 320320320e-320;
    printf("main%p\n", ptr);
    double* ptr2 = sf_malloc( sizeof(double));
    *ptr = 320320320e-320;
    printf("main%p\n", ptr2);
    */

    //testing coalese
    //double* ptr = sf_malloc(7000 * sizeof(double));


    //realoc test from unit test
    /*
    void *x = sf_malloc(sizeof(double) * 8);

    sf_show_heap();
    void *y = sf_realloc(x, sizeof(int));
    sf_show_heap();
    printf("%p\n",y);
    */
   void *a = sf_malloc(4040);
   void *c = sf_malloc(50);
    void *b = sf_malloc(3434);
    printf("%p\n%p\n",a,c);
   printf("%p\n",a);
   sf_free(a);
   sf_free(b);
    sf_show_heap();


    //sf_free(ptr);
   // sf_show_heap();

    sf_mem_fini();


    return EXIT_SUCCESS;
}
