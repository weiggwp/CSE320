
#include "graph.h"
// typedef struct {
// int v;
// struct Node *next;
// } Node;

typedef struct {
    int     nodes[MAXV+1][MAXV+1];//1 list per component
    int     indices[MAXV+1];
    Graph*  graphs[MAXV+1];
    int     ncomponents;
} Components;


typedef struct {
    int front, rear, size;
    int array[MAXV];
} Queue;

void connected_components(Graph *g,Components* c);

void bfs(Graph *g, int start,Components* c);

void initialize_search(Graph *g);

int dequeue(Queue* q);

void enqueue(Queue* q,int v);

int empty_queue(Queue*q);

void  init_queue(Queue* q);

