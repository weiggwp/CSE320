// code from Steven Skiena lecture
// modified by Wei Chen


#include <stdlib.h>
#include <stdio.h>

#include "graph.h"

void initialize_graph(Graph *g, int  directed){
    int i;
    g -> nvertices = 0;
    g -> nedges = 0;
    g -> directed = directed;
    for (i=1; i<=MAXV; i++) g -> degree[i] = 0;
    for (i=1; i<=MAXV; i++) {g -> edges[i] = NULL;}
}

void insert_edge(Graph *g, int x, int y, int  directed)
{
    Edgenode *p;
    p = malloc(sizeof(Edgenode));
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

void build_graph(Graph *g, int directed,FILE*infile)
{
    g ->directed = 0;

    initialize_graph(g,g->directed);

    fscanf(infile, "%d\n",& g->nvertices);

    fscanf(infile, "%d\n", & g->nedges);
    int x;
    int y;

    while(fscanf(infile, "%d %d\n", &x, &y)!=EOF){
        insert_edge(g, x, y, g->directed);
    }
}





