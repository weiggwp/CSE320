#include <stdlib.h>
#include <stdio.h>
#include<limits.h>

#include "bfs.h"
int processed[MAXV];
int discovered[MAXV];
int parent[MAXV];



void initialize_search(Graph *g)
{
    int i;
    for (i=0; i<=g-> nvertices; i++) {
    processed[i] = discovered[i] = 0;
    parent[i] = -1;
    }
}

void process_vertex(int v)
{
    printf("processed vertex %d ",v);
}

void  init_queue(Queue* q)
{
    q-> front = q-> size = 0;
    q-> rear = MAXV - 1;
}
int empty_queue(Queue*q)
{
    return q-> size == 0;
}
void enqueue(Queue* q,int v)
{
    if(q->size ==MAXV)
        return;
    q->rear = (q->rear + 1)%MAXV;
    q->array[q->rear] = v;
    q->size = q->size + 1;
    // printf("%d enqueued to queue\n", v);
}

int dequeue(Queue* q)
{
    if( empty_queue(q)){
        printf("empty queue\n");
        return INT_MIN;
    }
    int v = q->array[q->front];
    q->front = (q->front + 1)%MAXV;
    q->size = q->size - 1;
    return v;
}

// void insert_node(Components* cp, int v)
// {
//     Node *p;
//     p = malloc(sizeof(Node));
//     p -> v = v;
//     p -> next = (void*) cp -> nodes[cp->ncomponents];
//     cp -> nodes[cp->ncomponents] = p;
// }

// int insert_node(Components* cp, int v)
// {
//     //current vertex in this graph component
//     int index = ++(cp->graphs[cp->ncomponents]->nvertices);

//     cp->nodes[cp->ncomponents][index] = v;
//     cp->indices[v] = index;
//     // vertex inserted
//     return index;
// }

// void process_vertex_early(int v,Components* cp){
//     insert_node(cp,v);
// }
// void process_edge(int x,int y, Components* c)
// {
//     int yindex;
//     //if y has been discovered, don't insert y into nodes, but get index for y
//     if(discovered[y]){
//         yindex = c->indices[y];
//     }
//     else{
//         yindex = insert_node(c,y);
//     }
//     insert_edge(c->graphs[c->ncomponents],c->indices[x],yindex, 0);
// }
void bfs(Graph *g, int start,int target)
{
    Queue q;
    int v,y;
    Edgenode *p;

    init_queue(&q);
    enqueue(&q,start);
    discovered[start] = 1;
    // process_vertex_early(start,c);
    while (empty_queue(&q) == 0) {  //while q not empty
        v = dequeue(&q);
        //process verex early
        processed[v] = 1;
        p = g ->  edges[v];
        while (p  != NULL) {
            y = p ->  y;    //the other vertex
            // if ((processed[y] == 0) || g ->  directed)
                // process_edge(v,y,c);
            if (discovered[y] == 0){
                enqueue(&q,y);
                discovered[y] = 1;
                parent[y] = v;
            }
            p =(void*) p ->  next;
        }
        // process_vertex_late(v);
    }
}


int findShortestPath(Graph*g,int start,int target,int ntypes,int* path)
{
    initialize_search(g);

    bfs(g,start,target);
    int i=0;
    //use parent list to find the path
    while(target!=start){
        path[i++] = target;
        target = parent[target];
        if(target == -1)
            return 0;
    }
    path[i++] = start;

    return i;

}