
#include "graph.h"
typedef struct {
    int front, rear, size;
    int array[MAXV];
} queue;

void connected_components(graph *g);

void bfs(graph *g, int start,int c);

int dequeue(queue* q);

void enqueue(queue* q,int v);


int empty_queue(queue*q);

void  init_queue(queue* q);

void initialize_search(graph *g);
