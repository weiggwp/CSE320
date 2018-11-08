// #ifndef MAXV
// #define MAXV 100
// #endif
#include "connectedComponents.h"

void backtrack(int a[], int k,  Graph* g);

int is_a_solution(int a[], int k,  Graph* g);

void process_solution(int a[], int k);

void construct_candidates(int a[], int k,  Graph* g, int c[], int *ncandidates);

int update_current_min(int a[],int k,Graph* g);

void find_min_bandth(Graph* g,Components* c);
