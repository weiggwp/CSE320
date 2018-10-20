#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    double* ptr = sf_malloc(3 * PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 32);

    // sf_show_block_info(sf_block_info *ip);
    // sf_show_blocks();
    // sf_show_free_lists();
    sf_show_heap();


    // printf("%fMallocReturned\n", *ptr);

    sf_free(ptr);

    // double* ptr =

    sf_mem_fini();

    return EXIT_SUCCESS;
}
