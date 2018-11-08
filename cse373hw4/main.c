// The bandwidth problem takes as input a graph G, with n vertices and m edges (ie. pairs of vertices).
// The goal is to find a permutation of the vertices on the line which minimizes the maximum length of any
// edge. This is better understood through an example. Suppose G consists of 5 vertices and the edges (v1, v2),
// (v1, v3), (v1, v4), and (v1, v5). We must map each vertex to a distinct number between 1 and n. Under the
// mapping vi → i, the last edge spans a distance of four (ie. 5-1). However, the permutation {2, 3, 1, 4, 5} is
// better, for none of the edges spans a distance of more than two. In fact, it is a permutation which minimizes
// the maximum length of the edges in G, as is any permutation where 1 is in the center


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "connectedComponents.h"

int main(int argc, char *argv[])
{
    char* filenames[5]  = {"g-3-3","g-p-4-3","g-bt-10-9","g-bt-11-10","g-bt-12-11"};
    for(int i =0;i<sizeof(filenames);i++){

    clock_t begin = clock();
    //moon
    // FILE* infile = fopen("g-bt-19-18", "r");

    // min banwidth:3
    // Seq: 6 12 3 13 7 1 14 15 2 5 10 4 11 8 9 16 17 18
    //      6 12 3 13 7 1 14 15 2 5 10 4 11 9 8 17 18 16
    // Total time:90.016377
    // FILE* infile = fopen("g-bt-18-17", "r");

    // min banwidth:3
    // Seq: 4 8 9 2 16 5 1 10 11 3 6 12 7 13 14 15
    // Total time:6.948156
    FILE* infile = fopen(filenames[i], "r");

    // min banwidth:3
    // Seq: 4 8 2 9 1 5 10 3 11 6 7 12 13 14
    // Total time:1.518603
    // FILE* infile = fopen("g-bt-14-13", "r");

    //     min banwidth:2
    // Seq: 8 9 4 10 2 5 1 11 3 7 6 12 13
    // Total time:0.149996

    // FILE* infile = fopen("g-bt-13-12", "r");

    //Total time:1.488817
    // FILE* infile = fopen("g-bt-12-11", "r");

    //.36
    // FILE* infile = fopen("g-bt-11-10", "r");
    //0.053067
    // FILE* infile = fopen("g-bt-10-9", "r");
    // FILE* infile = fopen("g-3-3", "r");
    // FILE* infile = fopen("g-p-4-3", "r");

    if (infile == NULL)
    {
        printf("Can't open file for reading.\n");
        exit(0);
    }

    // char* line = malloc(32);
    // fgets(line, sizeof(line), infile); // nvetices
    // int nvetices = line[0];
    // fgets(line, sizeof(line), infile); // nedges
    // int nedges = line[0];


    graph* g = malloc(sizeof(graph));
    build_graph(g,0,infile);
    // connected_components(g);
    find_min_bandth(g);


    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total time:%f\n",time_spent);
    }
    exit(0);
}

