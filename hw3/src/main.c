#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    // double* ptr = sf_malloc(3 * PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 32);

    void *x = sf_malloc(sizeof(double) * 8);
    sf_header* bp = x-8;
    void *y = sf_realloc(x, sizeof(int));


    sf_header *hp = (sf_header *)((char*)y - sizeof(sf_footer));
    fprintf(stderr, "%p%p\n",hp,bp);
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
