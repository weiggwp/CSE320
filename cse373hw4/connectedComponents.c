// code from Steven Skiena lecture
// modified by Wei Chen

#include <stdlib.h>
#include <stdio.h>
#include<limits.h>

#include "connectedComponents.h"
int processed[MAXV];
int discovered[MAXV];
int parent[MAXV];



void initialize_search(graph *g)
{
    int i;
    for (i=1; i<=g-> nvertices; i++) {
    processed[i] = discovered[i] = 0;
    parent[i] = -1;
    }
}

void process_vertex(int v)
{
    printf("processed vertex %d ",v);
}
void process_edge(int x, int y)
{
printf("processed edge (%d,%d) ",x,y);
}

void  init_queue(queue* q)
{
    q-> front = q-> size = 0;
    q-> rear = MAXV - 1;
}
int empty_queue(queue*q)
{
    return q-> size == 0;
}
void enqueue(queue* q,int v)
{
    if(q->size ==MAXV)
        return;
    q->rear = (q->rear + 1)%MAXV;
    q->array[q->rear] = v;
    q->size = q->size + 1;
    printf("%d enqueued to queue\n", v);
}

int dequeue(queue* q)
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

void bfs(graph *g, int start,int c)
{
    queue q;
    int v,y;
    edgenode *p;

    init_queue(&q);
    enqueue(&q,start);
    discovered[start] = 1;
    while (empty_queue(&q) == 0) {  //while q not empty
        v = dequeue(&q);
        printf("%d ",v); // process_vertex_early(v);
        processed[v] = 1;
        p = g ->  edges[v];
        while (p  != NULL) {
            y = p ->  y;    //the other vertex
            if ((processed[y] == 0) || g ->  directed)
                process_edge(v,y);
            if (discovered[y] == 0) {
                enqueue(&q,y);
                discovered[y] = 1;
                parent[y] = v;
            }
            p =(void*) p ->  next;
        }
        // process_vertex_late(v);
    }
}


int components[MAXV];
void connected_components(graph *g)
{
    int i,c;
    initialize_search(g);
    c = 0;
    for(i=1; i<=g-> nvertices; i++)
        if (discovered[i] == 0) {
        c = c+1;
        printf("Component %d:",c);
        bfs(g,i,c);
    }
}