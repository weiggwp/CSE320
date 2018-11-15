
#include <stdlib.h>
#include <stdio.h>

#include "graph.h"

void initialize_graph(Graph *g, int  directed){
    int i;
    g -> nvertices = 0;
    g -> nedges = 0;
    g -> directed = directed;
    for (i=0; i<=MAXV; i++) g -> degree[i] = 0;
    for (i=0; i<=MAXV; i++) {g -> edges[i] = NULL;}
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

// void build_graph(Graph *g, int directed,void***matrix,int nvertices)
// {
//     g ->directed = 1;

//     initialize_graph(g,g->directed);

//     g->nvertices = nvertices;

//     for (int i = 0; i < nvertices; ++i)
//         for (int j = 0; j < nvertices; ++j)
//             if(matrix[i][j]!=NULL)
//                 insert_edge(g, i, j, g->directed);
// }





