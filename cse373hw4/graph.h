// code from Steven Skiena lecture
// modified by Wei Chen

#define MAXV 100
typedef struct {
int y;
int weight;
struct Edgenode *next;
} Edgenode;

typedef struct {
Edgenode *edges[MAXV+1];
int degree[MAXV+1];
int nvertices;
int nedges;
int directed;
} Graph;


void initialize_graph(Graph *g, int  directed);

void insert_edge(Graph *g, int x, int y, int  directed);
void build_graph(Graph *g, int directed,FILE* infile);

