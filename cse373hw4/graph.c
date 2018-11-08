// code from Steven Skiena lecture
// modified by Wei Chen


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#include "graph.h"

void initialize_graph(graph *g, int  directed){
    int i;
    g -> nvertices = 0;
    g -> nedges = 0;
    g -> directed = directed;
    for (i=1; i<=MAXV; i++) g -> degree[i] = 0;
    for (i=1; i<=MAXV; i++) {g -> edges[i] = NULL;}
}

void insert_edge(graph *g, int x, int y, int  directed)
{
    edgenode *p;
    p = malloc(sizeof(edgenode));
    p -> weight = 0;
    p -> y = y;
    p -> next = (void*) g -> edges[x];
    g -> edges[x] = p;
    g -> degree[x] ++;
    if (directed == 0)
    insert_edge(g,y,x,1);
    else
    g -> nedges ++;
}

void build_graph(graph *g, int directed,FILE*infile)
{
    g ->directed = 0;

    initialize_graph(g,g ->directed);

    fscanf(infile, "%d\n",&g -> nvertices);

    fscanf(infile, "%d\n", &g -> nedges);
    int x;
    int y;

    while(fscanf(infile, "%d %d\n", &x,&y)!=EOF){
        insert_edge(g,x,y,g ->directed);
    }
}
int sol_minBandth ;// minBandth solution
int sol_seq[MAXV+1];// solution sequence

int minBandthStack[MAXV+1]; // minBandth for current seq, default 0
int stackPos = 0;
int finished = 0;
int breakEarly = 0;
int lowerBound = 1;

int used[MAXV]; //if vertice is used(1) or not(0)
int positions[MAXV]; // position of vertice in a[]
int update_current_min2(int a[],int k,graph* g)
{
    int width;
    int ypos;
    int x = a[k];   //current vertice
    minBandthStack[k] = minBandthStack[k-1];
    //calculate the width of x to a node in a[], thinking next, hashmap
    edgenode* edgep = g->edges[x];

    while(edgep!=NULL){

        if( (ypos = positions[edgep->y]) !=-1){
            width = k - ypos;
            if(width>sol_minBandth) return 0;
            if(width >minBandthStack[k]){// update current banwidth
                minBandthStack[k] = width;
            }
        }
        edgep =(void*) edgep->next;
    }
    return 1;
}

void backtrack(int a[], int k, graph* g)
{
    int c[g->nvertices];    // candidates[]
    int ncandidates;    //number of candidates
    if (is_a_solution(a,k,g)){
        process_solution(a,k);
        if (sol_minBandth<=lowerBound){
            if(sol_minBandth<lowerBound)
                printf("banwidth(%d) < lowerBound(%d)\n",sol_minBandth,lowerBound );
            else
            finished = 1; // found perfect min bandwidth
        }
    }
    else {
        k = k+1;
        construct_candidates(a,k,g,c,&ncandidates);
        int i;
        for (i=0; i<ncandidates; i++) {
            if(i>0){
                used[a[k]] = 0; //restore
                positions[a[k]] = -1;
            }

            //candidate chosen
            a[k] = c[i];
            positions[a[k]] = k;

            used[a[k]] = 1;//this node is now used
            //check min banwidth
            if(update_current_min2(a,k,g) == 0){
                breakEarly =1;
                break;//TODO: break didnt work
            }
            backtrack(a,k,g);
            if (finished) return;
        }
        //new restore
        if (breakEarly)
        {
            used[c[i]]=0;
            breakEarly = 0;
        }
        else
            used[c[i-1]]=0;
        // old restore
        // used[c[ncandidates-1]]=0;

    }
}
int is_connected(int x,int y,graph* g){
    edgenode* edgep = g->edges[x];
    while(edgep!=NULL){
        if(y==edgep->y) return 1;
        edgep =(void*) edgep->next;
    }
    return 0;

}
int update_current_min(int a[],int k,graph* g)
{
    int width;
    int x = a[k];
    minBandthStack[k] = minBandthStack[k-1];
        //calculate the width of x to a node in a[], thinking next, hashmap
    for(int i=1;i<k;i++){
        if(is_connected(x,a[i],g) ){// if x is connect to the node
            width = k - i; //calculate width
            if(width>sol_minBandth) return 0;
            if(width >minBandthStack[k]){// update current banwidth
                minBandthStack[k] = width;
            }
        }

    }
    return 1;
}


void process_solution(int a[], int k)
{
    int i;
    if(minBandthStack[k]<sol_minBandth){
        sol_minBandth = minBandthStack[k];
        for (i=1; i<=k; i++) sol_seq[i] = a[i];
    }
}
int is_a_solution(int a[], int k,  graph* g)
{
    return (k == g->nvertices);
}


void construct_candidates(int a[], int k,  graph* g, int c[], int *ncandidates)
{
    int i;
    // for (int i=1; i<MAXV; i++) used[i] = 0; //initialize used[]
    // for (i=0; i<k; i++) used[ a[i] ] = 1;   //set used based on what is in a[]
    //TODO: might be able to improve here^
    *ncandidates = 0;
    for (i=1; i<=g->nvertices; i++)
        if (used[i] == 0) {
            c[ *ncandidates] = i;// add to candidate list if hasnt been used
            *ncandidates += 1;
        }
}

void find_min_bandth(graph* g)
{
    printf("start finding minBandth...\n");
    printf("please be patient, but note that I might never come back...\n");

    int i, nvertices = g->nvertices;
    for(i=1; i<=nvertices; i++) used[i] = 0; //initialize used[]
    for(i=1; i<=nvertices; i++) positions[i] = -1; // initialize positions[]
    sol_minBandth = nvertices-1;//initialize min banwidth
    for (i=1; i<=nvertices; i++) sol_seq[i] = i;//initialize sol seq[]
    minBandthStack[0]=0;//sentinal

    //determine lower bound
    int maxDegree = 0;
    for (i = 1; i <= nvertices; ++i)
    {
        int d = g->degree[i];
        if(d >maxDegree)
            maxDegree = d;
        // if(d!=nvertices-1)
            // completeG = 0;
    }
    lowerBound = (maxDegree+1)/2;
    int a[nvertices+1]; //permutation list
    backtrack(a,0,g);
    printf("min banwidth:%i\nSeq:",sol_minBandth );
    for(i=1; i<=nvertices; i++) printf(" %d",sol_seq[i]);
    printf("\n");

}
void find_min_bandth2(graph* g)
{
    printf("start finding minBandth...\n");
    printf("please be patient, but note that I might never come back...\n");

    int i, nvertices = g->nvertices;
    for(i=1; i<=nvertices; i++) used[i] = 0; //initialize used[]
    for(i=1; i<=nvertices; i++) positions[i] = -1; // initialize positions[]
    sol_minBandth = nvertices-1;//initialize min banwidth
    for (i=1; i<=nvertices; i++) sol_seq[i] = i;//initialize sol seq[]
    minBandthStack[0]=0;//sentinal

    //determine lower bound
    int maxDegree = 0;
    for (i = 1; i <= nvertices; ++i)
    {
        int d = g->degree[i];
        if(d >maxDegree)
            maxDegree = d;
        // if(d!=nvertices-1)
            // completeG = 0;
    }
    lowerBound = (maxDegree+1)/2;
    //bfs

    int a[nvertices+1]; //permutation list
    backtrack(a,0,g);
    printf("min banwidth:%i\nSeq:",sol_minBandth );
    for(i=1; i<=nvertices; i++) printf(" %d",sol_seq[i]);
    printf("\n");

}




