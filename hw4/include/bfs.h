#include "graph.h"
typedef struct {
    int front, rear, size;
    int array[MAXV];
} Queue;

void bfs(Graph *g, int start,int target);

void initialize_search(Graph *g);

int dequeue(Queue* q);

void enqueue(Queue* q,int v);

int empty_queue(Queue*q);

void  init_queue(Queue* q);

int findShortestPath(Graph*g, int start,int target,int ntypes,int* path);