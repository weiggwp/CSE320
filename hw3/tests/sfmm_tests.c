#include <criterion/criterion.h>
#include <errno.h>
#include <signal.h>
#include "debug.h"
#include "sfmm.h"

#define MIN_BLOCK_SIZE (32)

sf_free_list_node *find_free_list_for_size(size_t size) {
    sf_free_list_node *fnp = sf_free_list_head.next;
    while(fnp != &sf_free_list_head && fnp->size < size)
	fnp = fnp->next;
    if(fnp == &sf_free_list_head || fnp->size != size)
	return NULL;
    return fnp;
}

int free_list_count(sf_header *ahp) {
	// fprintf(stderr, "%s\n","entered free_list_count" );
    int count = 0;
    sf_header *hp = ahp->links.next;

    	// fprintf(stderr, "%p\n", ahp);
    while(hp != ahp) {
    	// fprintf(stderr, "%p\n", hp);
	count++;
	hp = hp->links.next;
    }
    return count;
}

void assert_free_list_count(size_t size, int count) {
    sf_free_list_node *fnp = sf_free_list_head.next;
    while(fnp != &sf_free_list_head) {
	if(fnp->size == size)
	    break;
	fnp = fnp->next;
    }
    cr_assert(fnp != &sf_free_list_head && fnp->size == size,
	      "No free list of size %lu was found", size);
    int flc = free_list_count(&fnp->head);
    cr_assert_eq(flc, count,
		 "Wrong number of blocks in free list for size %lu (exp=%d, found=%d)",
		 size, flc);
}

void assert_free_block_count(int count) {
	// fprintf(stderr, "%s\n", "entered assert_free_block_count");
    int n = 0;
    sf_free_list_node *fnp = sf_free_list_head.next;
    while(fnp != &sf_free_list_head) {
	n += free_list_count(&fnp->head);
	// fprintf(stderr, "%s\n", "inside loop 1");
	fnp = fnp->next;
    }


    cr_assert_eq(n, count, "Wrong number of free blocks (exp=%d, found=%d)", count, n);
}

Test(sf_memsuite_student, malloc_an_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	int *x = sf_malloc(sizeof(int));

	cr_assert_not_null(x, "x is NULL!");

	*x = 4;

	cr_assert(*x == 4, "sf_malloc failed to give proper space for an int!");

	assert_free_block_count(1);
	assert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE, 1);

	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, malloc_three_pages, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	void *x = sf_malloc(3 * PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE);

	cr_assert_not_null(x, "x is NULL!");
	assert_free_block_count(0);
	cr_assert(sf_errno == 0, "sf_errno is not 0!");
}

Test(sf_memsuite_student, malloc_over_four_pages, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	void *x = sf_malloc(PAGE_SZ << 2);

	cr_assert_null(x, "x is not NULL!");
	assert_free_block_count(1);
	cr_assert(sf_errno == ENOMEM, "sf_errno is not ENOMEM!");
}

Test(sf_memsuite_student, free_double_free, .init = sf_mem_init, .fini = sf_mem_fini, .signal = SIGABRT) {
	sf_errno = 0;
	void *x = sf_malloc(sizeof(int));
	sf_free(x);
	sf_free(x);
}

Test(sf_memsuite_student, free_no_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *x = */ sf_malloc(sizeof(long));
	void *y = sf_malloc(sizeof(double) * 10);
	/* void *z = */ sf_malloc(sizeof(char));

	sf_free(y);
	// fprintf(stderr, "\n%s\n","freed" );
	assert_free_block_count(2);
	// fprintf(stderr, "%s\n", "done");

	assert_free_list_count(96, 1);
	assert_free_list_count(3888, 1);

	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, free_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *w = */ sf_malloc(sizeof(long));
	void *x = sf_malloc(sizeof(double) * 11);
	void *y = sf_malloc(sizeof(char));
	/* void *z = */ sf_malloc(sizeof(int));

	sf_free(y);
	sf_free(x);

	assert_free_block_count(2);
	assert_free_list_count(128, 1);
	assert_free_list_count(3856, 1);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *u = sf_malloc(1); //32
	/* void *v = */ sf_malloc(40); //48
	void *w = sf_malloc(152); //160
	/* void *x = */ sf_malloc(536); //544
	void *y = sf_malloc(1); // 32
	/* void *z = */ sf_malloc(2072); //2080

	sf_free(u);
	sf_free(w);
	sf_free(y);

	assert_free_block_count(4);
	assert_free_list_count(32, 2);
	assert_free_list_count(160, 1);
	assert_free_list_count(1152, 1);

	// First block in list should be the most recently freed block.
	sf_free_list_node *fnp = find_free_list_for_size(32);
	cr_assert_eq(fnp->head.links.next, (sf_header *)((char *)y - sizeof(sf_footer)),
		     "Wrong first block in free list (32): (found=%p, exp=%p)",
                     fnp->head.links.next, (sf_header *)((char *)y - sizeof(sf_footer)));
}

Test(sf_memsuite_student, realloc_larger_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int));
	/* void *y = */ sf_malloc(10);
	x = sf_realloc(x, sizeof(int) * 10);

	cr_assert_not_null(x, "x is NULL!");
	sf_header *hp = (sf_header *)((char *)x - sizeof(sf_footer));
	cr_assert(hp->info.allocated == 1, "Allocated bit is not set!");
	cr_assert(hp->info.block_size << 4 == 48, "Realloc'ed block size not what was expected!");

	assert_free_block_count(2);
	assert_free_list_count(32, 1);
	assert_free_list_count(3936, 1);
}

Test(sf_memsuite_student, realloc_smaller_block_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int) * 8);
	void *y = sf_realloc(x, sizeof(char));

	cr_assert_not_null(y, "y is NULL!");
	cr_assert(x == y, "Payload addresses are different!");

	sf_header *hp = (sf_header *)((char*)y - sizeof(sf_footer));
	cr_assert(hp->info.allocated == 1, "Allocated bit is not set!");
	cr_assert(hp->info.block_size << 4 == 48, "Block size not what was expected!");
	cr_assert(hp->info.requested_size == 1, "Requested size not what was expected!");

	// There should be only one free block of size 4000.
	assert_free_block_count(1);
	assert_free_list_count(4000, 1);
}

Test(sf_memsuite_student, realloc_smaller_block_free_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(double) * 8);
	void *y = sf_realloc(x, sizeof(int));

	cr_assert_not_null(y, "y is NULL!");

	sf_header *hp = (sf_header *)((char*)y - sizeof(sf_footer));
	cr_assert(hp->info.allocated == 1, "Allocated bit is not set!");
	cr_assert(hp->info.block_size << 4 == 32, "Realloc'ed block size not what was expected!");
	cr_assert(hp->info.requested_size == 4, "Requested size not what was expected!");

	// After realloc'ing x, we can return a block of size 48 to the freelist.
	// This block will coalesce with the block of size 3968.
	assert_free_block_count(1);
	assert_free_list_count(4016, 1);
}

//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################

Test(sf_memsuite_student, malloc_an_Double_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    double *x = sf_malloc(sizeof(double));

    cr_assert_not_null(x, "x is NULL!");

    *x = 8;

    cr_assert(*x == 8, "sf_malloc failed to give proper space for an int!");

    assert_free_block_count(1);
    assert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, malloc_an_Char_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    char *x = sf_malloc(sizeof(char));

    cr_assert_not_null(x, "x is NULL!");

    *x = 'a';

    cr_assert(*x == 'a', "sf_malloc failed to give proper space for an int!");

    assert_free_block_count(1);
    assert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}
//rs = 32 bs = 48
Test(sf_memsuite_student, malloc_32_Char_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    char *x = sf_malloc(32*sizeof(char));
    cr_assert_not_null(x, "x is NULL!");

    const char s[32] = "1234567890123456789012345678901";
    memcpy(x, s, strlen(s)+1); //+1 for \0

    cr_assert(strcmp(x,s)==0, "sf_malloc failed to give proper space!");

    assert_free_block_count(1);
    assert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE-16, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}
//rs = -1
Test(sf_memsuite_student, malloc_negative_1_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    sf_malloc(-1);
    cr_assert(sf_errno == ENOMEM, "sf_errno is zero!");
}
Test(sf_memsuite_student, malloc_15_ptr_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    int **p = sf_malloc(15*sizeof(*p));// bs = 128 rs 120
    cr_assert_not_null(p, "x is NULL!");

    assert_free_block_count(1);
    assert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 128, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, malloc_15_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    for (int i = 0; i < 15; ++i)
    {
        /* code */
        sf_malloc(sizeof(int));
    }

    assert_free_block_count(1);
    size_t bs = PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - MIN_BLOCK_SIZE;
    for (int i = 0; i < 14; ++i)
    {
        /* code */
        assert_free_list_count(bs, 0);
        bs-=MIN_BLOCK_SIZE;
    }
    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}
Test(sf_memsuite_student, malloc_15_free_5th_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    int **p = sf_malloc(15*sizeof(int *));

    for (int i = 0; i < 15; ++i)
    {
        /* code */
        int *x = sf_malloc(sizeof(int));
        *(p+i) = x;
    }
    for (int i = 0; i < 15; ++i)
    {
        /* code */
        if(i%5 ==0)
        sf_free(*(p+i) );
    }

    assert_free_block_count(4);
    // assert_free_block_count(1);
    size_t bs = PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) ;
    assert_free_list_count(bs, 0);
    bs-=128;
    for (int i = 0; i < 14; ++i)
    {
        /* code */
        assert_free_list_count(bs, 0);
        bs-=MIN_BLOCK_SIZE;
    }

    // **(p+14) = 4;
    // cr_assert(**(p+14) == 4, "sf_malloc failed to give proper space for an int!");
    // sf_free(*(p+14));
    // sert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 128, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, malloc_15_free_non5th_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_errno = 0;
    int **p = sf_malloc(15*sizeof(int *));

    //allocate 15 blocks
    for (int i = 0; i < 15; ++i)
    {
        /* code */
        int *x = sf_malloc(sizeof(int));
        *(p+i) = x;
    }
    for (int i = 0; i < 15; ++i)
    {
        /* code */
        if(i%5 !=0)
        sf_free(*(p+i) );
    }

    assert_free_block_count(3);
    size_t bs = PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) ;
    assert_free_list_count(bs, 0);
    assert_free_list_count(128,2);

    // **(p+14) = 4;
    // cr_assert(**(p+14) == 4, "sf_malloc failed to give proper space for an int!");
    // sf_free(*(p+14));
    // sert_free_list_count(PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 128, 1);

    cr_assert(sf_errno == 0, "sf_errno is not zero!");
    cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}
Test(sf_memsuite_student, realloc_larger_block_check_content, .init = sf_mem_init, .fini = sf_mem_fini) {
    char *x = sf_malloc(32*sizeof(char)); //bs

    cr_assert_not_null(x, "x is NULL!");
    const char s[32] = "1234567890123456789012345678901";
    memcpy(x, s, strlen(s)+1); //+1 for \0

    cr_assert(strcmp(x,s)==0, "sf_malloc failed to give proper space!");
    /* void *y = */ sf_malloc(10);

    x = sf_realloc(x, sizeof(char) * 100); //bs 112
    cr_assert_not_null(x, "x is NULL!");
    cr_assert(strcmp(x,s)==0, "sf_realloc failed to give proper space!");

    sf_header *hp = (sf_header *)((char *)x - sizeof(sf_footer));
    cr_assert(hp->info.allocated == 1, "Allocated bit is not set!");
    cr_assert(hp->info.block_size << 4 == 112, "Realloc'ed block size not what was expected!");

    assert_free_block_count(2);
    // assert_free_list_count(112, 0);
    // assert_free_list_count(32, 0);// rs 10 block
    assert_free_list_count(48, 1);
}
//realoc to rs 0
Test(sf_memsuite_student, realloc_to_0, .init = sf_mem_init, .fini = sf_mem_fini) {
    char *x = sf_malloc(32*sizeof(char)); //bs

    cr_assert_not_null(x, "x is NULL!");
    const char s[32] = "1234567890123456789012345678901";
    memcpy(x, s, strlen(s)+1); //+1 for \0

    cr_assert(strcmp(x,s)==0, "sf_malloc failed to give proper space!");
    /* void *y = */ sf_malloc(10);

    x = sf_realloc(x, 0); //bs 112
    cr_assert_null(x, "x is not NULL!");


    assert_free_block_count(2);
    // assert_free_list_count(32, 0);// rs 10 block
    assert_free_list_count(48, 1);
}
