// #include <stdint.h>
// #include <stdlib.h>

#include "bfs.h"
#include "job.h"


#define TPYEMAX  64
#define PRINTERMAX 32
#define BUFSIZE 1024
#define JOBMAX 32
typedef struct
{   int id;
    char* name;
}TypePrinter;

typedef struct
{
    int declared;
    int from; //typeID
    int to;
    char* name; //name of conversion progream
    int num_arg;
    char** args;//args for conversion progream
    int path_length;
    int path[TPYEMAX];
}Conversion;

typedef struct
{
    int typeCount;
    TypePrinter typeList [sizeof(TypePrinter)*TPYEMAX];

    int printerCount;
    PRINTER printerList [sizeof(PRINTER)*PRINTERMAX];

    Conversion conversionMatrix[TPYEMAX][TPYEMAX];

    FILE* outfile;
    char* buf[BUFSIZE];

    // JOB jobList [sizeof(JOB)*JOBMAX];
    int jobCount;
    JobQueue q;

}Storage;
Storage info;
void initInfo();


char* readWord(char* wordp,char* linep);

int excuteCommand(char* line);

void findJob(PRINTER*p);