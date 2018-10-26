#include <stdio.h>
#include "sfmm.h"
#include <string.h>
int main(int argc, char const *argv[]) {
    sf_mem_init();

    char *x = sf_malloc(32*sizeof(char)); //bs

    const char s[32] = "1234567890123456789012345678901";
    memcpy(x, s, strlen(s)+1); //+1 for \0

    /* void *y = */ sf_malloc(10);

    x = sf_realloc(x, 0); //bs 112



    //  int **p = sf_malloc(15*sizeof(int *));
    // //allocate 15 blocks
    // for (int i = 0; i < 15; ++i)
    // {
    //     /* code */
    //     int *x = sf_malloc(24);
    //     *(p+i) = x;
    // }
    // for (int i = 0; i < 15; ++i)
    // {
    //     /* code */
    //     if(i%5 !=0)
    //     sf_free(*(p+i) );
    // }

    // fprintf(stderr, "%p,%p\n",p+1,*(p+1));
    // **(p?+14) = 4;
    // sf_fee(*(p+14));
    // char *x = sf_malloc(32*sizeof(char));
    // const char src[32] = "1234567890123456789012345678901";
    // memcpy(x, src, strlen(src)+1); //+1 for \0
    // double* ptr = sf_malloc(3 * PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 32);

    // void *x = sf_malloc(sizeof(double) * 32);
    // sf_malloc(sizeof(double) );
    // void * y =sf_malloc(sizeof(double)*32);
    // sf_malloc(sizeof(double) );
    // void * q = sf_malloc(sizeof(double)*32);
    // sf_malloc(sizeof(double) );
    // void *m = sf_malloc(sizeof(double)*32);
    // sf_malloc(sizeof(double) );
    // void *l = sf_malloc(sizeof(double)*32);

    // sf_free(x); sf_free(y);sf_free(q);sf_free(m);sf_free(l);

    // sf_header* bp = x-8;
    // //void *y = sf_realloc(x, sizeof(int));


    // sf_header *hp = (sf_header *)((char*)y - sizeof(sf_footer));
    // fprintf(stderr, "%p%p\n",hp,bp);
    // int n = 0;

    // sf_free_list_node *fnp = sf_free_list_head.next;
    // fprintf(stderr, "sf_free_list_head:%p\n",&sf_free_list_head );
    // while(fnp != &sf_free_list_head) {
    //     fprintf(stderr, "sf_free_list_node:%p\n",fnp );

    //     fprintf(stderr, "%s\n","entered free_list_count" );
    //     int count = 0;
    //     sf_header *hp = fnp->head.links.next;

    //     fprintf(stderr, "%p\n", &fnp->head);
    //     while(hp != &fnp->head) {
    //         fprintf(stderr, "%p\n", hp);
    //     count++;
    //     hp = hp->links.next;
    //     }

    // fprintf(stderr, "inloop:%d\n", count);
    // n += count;
    // fprintf(stderr, "outloop:%d\n", n);
    // fnp = fnp->next;
    // }
    // sf_show_block_info((sf_block_info *)x-1);

    // sf_show_block_info((sf_block_info *)y-1);

    // sf_show_block_info((sf_block_info *)z-1);
    // sf_show_blocks();
    // sf_show_free_lists();
    sf_show_heap();

    // sf_free(y);

    // printf("%p:%p:%pMallocReturned\n", x,y,z);

    // sf_free(ptr);

    // double* ptr =

    sf_mem_fini();

    return EXIT_SUCCESS;
}
