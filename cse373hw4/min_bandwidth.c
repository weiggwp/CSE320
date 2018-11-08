
#include <stdlib.h>
#include <stdio.h>

#include "min_bandwidth.h"

int sol_minBandth ;// minBandth solution
int sol_seq[MAXV+1];// solution sequence

int minBandthStack[MAXV+1]; // minBandth for current seq, default 0
int stackPos = 0;
int finished = 0;
int breakEarly = 0;
int lowerBound = 1;

int used[MAXV]; //if vertice is used(1) or not(0)
int positions[MAXV]; // position of vertice in a[]


int update_current_min2(int a[],int k,Graph* g)
{
    int width;
    int ypos;
    int x = a[k];   //current vertice
    minBandthStack[k] = minBandthStack[k-1];
    //calculate the width of x to a node in a[], thinking next, hashmap
    Edgenode* edgep = g->edges[x];

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

void backtrack(int a[], int k, Graph* g)
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

    }
}
int is_connected(int x,int y,Graph* g)
{
    Edgenode* edgep = g->edges[x];
    while(edgep!=NULL){
        if(y==edgep->y) return 1;
        edgep =(void*) edgep->next;
    }
    return 0;

}

int update_current_min(int a[],int k,Graph* g)
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
int is_a_solution(int a[], int k,  Graph* g)
{
    return (k == g->nvertices);
}


void construct_candidates(int a[], int k,  Graph* g, int c[], int *ncandidates)
{
    int i;
    // for (int i=1; i<MAXV; i++) used[i] = 0; //initialize used[]
    // for (i=0; i<k; i++) used[ a[i] ] = 1;   //set used based on what is in a[]
    //TODO: might be able to improve here^
    *ncandidates = 0;
    for (i=1; i<=g->nvertices; i++)
        if (used[i] == 0) {
            c[ *ncandidates] = i;//TODO: =list[i]// add to candidate list if hasnt been used
            *ncandidates += 1;
        }
}

void find_min_bandth(Graph* g,Components* c)
{

    // printf("please be patient, but note that I might never come back...\n");

    int i;
    minBandthStack[0]=0;//sentinal


    //components search
    int nc;
    int ncompo = c->ncomponents, seq_index=1;
    int final_sol_seq[g->nvertices+1];
    int final_minBandwith = 0;
    int nvertices;
    //for each component
    for (nc = 1; nc <= ncompo; ++nc)
    {
        stackPos = 0;
        finished = 0;
        breakEarly = 0;
        lowerBound = 1;
        //determine lower bound
        //check if Graph complete
        //check if Graph is simple path
        int maxDegree = 0, completeG = 1, simpleP = 1,d1Vertex;
        Graph* sub_g = c->graphs[nc];

        nvertices = sub_g->nvertices;
        for(i=1; i<=nvertices; i++) used[i] = 0; //initialize used[]
        for(i=1; i<=nvertices; i++) positions[i] = -1; // initialize positions[]
        sol_minBandth = nvertices-1;//initialize min banwidth
        for (i=1; i<=nvertices; i++) sol_seq[i] = i;//initialize sol seq[]

        for (i = 1; i <= nvertices; ++i)
        {
            int d = sub_g->degree[i];
            if(d >maxDegree)
                maxDegree = d;
            if(d!=nvertices-1)
                completeG = 0;
            if(d>2)
                simpleP = 0;
            if(d==1)
                d1Vertex =i;
        }
        //if Graph is complete
        if(completeG){
            //any permutation works
            for (i = 1; i <= nvertices; ++i)
            {
                sol_seq[i] = i;
            }
        }
        else if(simpleP){
            sol_minBandth = 1;
            sol_seq[1] = d1Vertex;
            int vertex = d1Vertex;
            int prevVertex=-1;
            //start with d1 vertex, construct path
            for (i = 2; i <= nvertices; ++i)
            {
                //go to next vertex: if
                if(sub_g->edges[vertex]->y == prevVertex){
                    if(sub_g->edges[vertex]->next==NULL){
                       sol_seq[i] = vertex;
                       break;
                    }
                    else{
                        prevVertex = vertex;
                        vertex = ((Edgenode*) sub_g->edges[vertex]->next)->y;
                    }
                }
                else{
                    prevVertex = vertex;
                    vertex = sub_g->edges[vertex]->y;
                }

                sol_seq[i] = vertex;// just gonna reuse d1Vertex for current vertex


            }

        }
        else{
            lowerBound = (maxDegree+1)/2;


            int a[nvertices+1]; //permutation list
            backtrack(a,0,sub_g);
        }
        //add partial sol seq to final sol seq

        for(int j=1; j<=nvertices; j++){
            final_sol_seq[seq_index++] = c->nodes[nc][sol_seq[j]];
        }
        //if new bandwith of subg > old, update:
        if(sol_minBandth>final_minBandwith)
            final_minBandwith = sol_minBandth;


    }
    printf("min banwidth:%i\nSeq:",final_minBandwith );
    for(i=1; i<=g->nvertices; i++) printf(" %d",final_sol_seq[i]);
    printf("\n");

}
void find_min_bandth2(Graph* g)
{
    printf("start finding minBandth...\n");
    printf("please be patient, but note that I might never come back...\n");
 stackPos = 0;
 finished = 0;
 breakEarly = 0;
 lowerBound = 1;
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