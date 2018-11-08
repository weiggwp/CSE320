// code from Steven Skiena lecture
// modified by Wei Chen

#define MAXV 100
typedef struct {
int y;
int weight;
struct edgenode *next;
} edgenode;

typedef struct {
edgenode *edges[MAXV+1];
int degree[MAXV+1];
int nvertices;
int nedges;
int directed;
} graph;


void initialize_graph(graph *g, int  directed);

void insert_edge(graph *g, int x, int y, int  directed);
void build_graph(graph *g, int directed,FILE* infile);

void backtrack(int a[], int k,  graph* g);

int is_a_solution(int a[], int k,  graph* g);

void process_solution(int a[], int k);

void construct_candidates(int a[], int k,  graph* g, int c[], int *ncandidates);

int update_current_min(int a[],int k,graph* g);

void find_min_bandth(graph* g);