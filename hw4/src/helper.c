#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <imprimer.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <fcntl.h>

/* Not technically required, but needed on some UNIX distributions */
#include <sys/types.h>
#include <sys/stat.h>

#include "helper.h"


#define HELP        0
#define QUIT        1
#define TYPE        2
#define PRTER       3
#define CONVERSION  4
#define PRINTERS    5
#define JOBS        6
#define PRINT       7
#define CANCEL      8
#define PAUSE       9
#define RESUME      10
#define DISABLE     11
#define ENABLE      12

static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {HELP,         "help",    'h',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {QUIT,         "quit",   'q',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {TYPE,         "type",     0,        required_argument, NULL,
                  "Print frequency tables."},
 {PRTER,        "printer",    0,        required_argument, NULL,
                  "Print quantile information."},
 {CONVERSION,   "conversion", 0,        required_argument, NULL,
                  "Print quantile summaries."},
 {PRINTERS,     "printers",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {JOBS,         "jobs",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {PRINT,        "print",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {CANCEL,       "cancel",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {PAUSE,        "pause",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {RESUME,       "resume",       'a',      no_argument, NULL,
                  "Print all reports."},
 {DISABLE,      "disable",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."},
 {ENABLE,       "enable",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {0,NULL, 0, 0, NULL, NULL}
 /*real BUG1: need Null termination so getopt_long knows when to stop when invalid args */

};

#define NUM_OPTIONS (14)
static char short_options[NUM_OPTIONS*3];
static struct option long_options[NUM_OPTIONS];

//initialize long_option using the option table
static void init_options() {
    unsigned int i = 0,j=0;
    for(i = 0; i < NUM_OPTIONS; i++) {
        //option info = address of current option from option table
        struct option_info *option_info_p = &option_table[i];
        struct option *op = &long_options[i];
        op->name = option_info_p->name;
        op->has_arg = option_info_p->has_arg;
        op->flag = NULL;
        op->val = option_info_p->val;

        if(option_info_p->chr!=0){
            *(short_options+j) = option_info_p->chr;

            if(option_info_p->has_arg == required_argument){
                j++;
                *(short_options+j) = ':';
            }
            j++;
        }



    }
}

void initInfo()
{
    info.outfile= stdout;
    info.typeCount = info.printerCount = 0;
    for (int i = 0; i < TPYEMAX; ++i)
        for (int j = 0; j < TPYEMAX; ++j){
            // info.conversionMatrix[i][j].declared = 0;
            info.conversionMatrix[i][j].path_length = 0;
        }
    init_jobqueue(&info.q);
}
void usage()
{
    struct option_info *opt;
        fprintf(stderr, "Valid options are:\n");
        for(unsigned int i = 0; i < NUM_OPTIONS-1; i++) {
                opt = &option_table[i];
                char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
                if(opt->chr)
                  sprintf(optchr, "-%c, ", opt->chr);
                char arg[32];
                if(opt->has_arg)
                    sprintf(arg, " <%.10s>", opt->argname);
                else
                    sprintf(arg, "%.13s", "");
                fprintf(stderr, "\t%s--%-10s%-13s\t%s\n",
                            optchr, opt->name, arg, opt->descr);
                opt++;
        }
}
void printPrinterList(){
    fprintf(stderr, "%s\n","printers:" );
    for (int i = 0; i < info.printerCount; ++i)
    {
        fprintf(stderr, "%s:%s\n", info.printerList[i].name,info.printerList[i].type);
    }
}
void printTypelist(){
    fprintf(stderr, "%s\n","types:" );

    for (int i = 0; i < info.typeCount; ++i)
    {
        fprintf(stderr, "%s\n", info.typeList[i].name);
    }
}
void printCM()
{
    fprintf(stderr, "%s\n","conversionMatrix:" );

    for (int i = 0; i < info.typeCount; ++i)
    {
        for (int j = 0; j < info.typeCount; ++j)
        {
        fprintf(stderr, "%d ", info.conversionMatrix[i][j].path_length);

        }
        fprintf(stderr, "\n" );
    }
}
void printInfo(){
    printTypelist();
    printPrinterList();
    printCM();
}
char* myStrCopy(char* string)
{
    char* space = malloc(strlen(string)+1);
    strncpy(space,string,strlen(string)+1);
    return space;
}
int findTypeID(char* type)
{
    TypePrinter* types  = info.typeList;
    for (int i = 0; i < info.typeCount; ++i)
    {
       if(strcmp(type,types[i].name)==0){
            return types[i].id;
       }
    }
    return -1;
}
int findPrinterID(char* name)
{
    PRINTER* printers  = info.printerList;
    for (int i = 0; i < info.printerCount; ++i)
    {
       if(strcmp(name,printers[i].name)==0){
            return printers[i].id;
       }
    }
    return -1;
}
int getFileTypeID(char* file_name)
{

    char currentChar;
    int i = 0, index=-1;
    while((currentChar = file_name[i]) !='\0' ){
        if(currentChar=='.'){
            index = i;
            break;
        }
        i++;
    }
    if(index<0)
        return -1;

    return findTypeID(&file_name[index+1]);
}
void declareType(char* type)
{
    if(info.typeCount>=TPYEMAX){
        char* msg = "Maximum amount of type reached";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        return;
    }

    //check if type already exist
    for (int i = 0; i < info.typeCount; ++i)
    {
        //if type found
        if( strcmp(type,info.typeList[i].name)==0){
            char* msg = "type already declared";
            imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
            return;
        }
    }
    //FIXME: might need to check if type t already exist
    int index = info.typeCount;
    TypePrinter* t = &info.typeList[index];
    t->id = index;
    char* space = myStrCopy(type);
    t->name=space;
    info.typeCount ++;
}
void declarePrinter(char* name,char* type)
{
    int type1ID = findTypeID(type);
    //TODO: check if id is -1
    if(type1ID<0){
        char* msg = "type not found";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        return;
    }
    if(info.printerCount == PRINTERMAX){
        char* msg = "Maximum amount of pritners reached";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        return;
    }
    //check if printer already exist
    for (int i = 0; i < info.printerCount; ++i)
    {
        //if type found
        if( strcmp(name,info.printerList[i].name)==0){
            char* msg = "name already declared";
            imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
            return;
        }
    }

    int index = info.printerCount;
    PRINTER* p = &info.printerList[index];
    p->id = index;
    // p->enabled = 1;//FIXME
    char* namecpy = myStrCopy(name);
    char* typecpy = myStrCopy(type);
    p->name=namecpy;
    p->type=typecpy;
    info.printerCount ++;
}
void declareConversion(char** wordList,int wordCount)
{
    int type1ID = findTypeID(wordList[1]);
    int type2ID = findTypeID(wordList[2]);
    //TODO: check if id is -1
    if(type1ID<0 || type2ID<0){
        char* msg = "type not found";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        return;
    }
    // conversion of same type is meaningless
    if(type1ID == type2ID) return;

    Conversion* c = & info.conversionMatrix[type1ID][type2ID];
    c-> declared = 1;
    c->from = type1ID;
    c->to = type2ID;
    char* namecpy = myStrCopy(wordList[3]);
    c->name = namecpy;
    c->num_arg = wordCount-3;

    c->args = malloc((c-> num_arg+1)*sizeof(char*));
    int i;
    for(i =0;i<c->num_arg;i++){
        char* argcpy = myStrCopy(wordList[i+3]);
        c->args[i] = argcpy;
    }
    c->args[i] = NULL;

    c->path_length = 2;
    c->path[0] = type1ID;
    c->path[1] = type2ID;
}
void printerReport(){

        PRINTER* printerList = info.printerList;

        for (int i = 0; i < info.printerCount; ++i)
        {
            fprintf(info.outfile,
                "%s\n",
                imp_format_printer_status(&printerList[i],(char*)info.buf,BUFSIZE));

        }
}
void jobReport(){

    JOB* j = info.q.front;
        while(j!=NULL){
            fprintf(info.outfile,
                "%s\n",
                imp_format_job_status(j,(char*)info.buf,BUFSIZE));
        }
    }
JOB* createJob(char* file_name,int typeID,PRINTER_SET set)
{
    // JOB* j = &info.jobList[info.jobCount];
    JOB* j = malloc(sizeof(JOB));
    j->jobid = info.jobCount++;
    j->status = QUEUED;
    char* file_nameCpy = myStrCopy(file_name);
    j->file_name = file_nameCpy;
    j->file_type = info.typeList[typeID].name;
    j->eligible_printers = set;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    j->creation_time = tv;
    j->change_time=tv;
    j->other_info = NULL;
    return j;
}
void build_graph(Graph *g, int directed,int nvertices)
{
    g ->directed = 1;
    initialize_graph(g,g->directed);

    g->nvertices = nvertices;

    for (int i = 0; i < nvertices; ++i)
        for (int j = 0; j < nvertices; ++j)
            if(info.conversionMatrix[i][j].path_length)
                insert_edge(g, i, j, g->directed);
}

volatile sig_atomic_t done;
int ccount = 0;
void child_handler(int sig) {
    int olderrno = errno;
    pid_t pid;
    int child_status;
    // for (i = 0; i < ; i++) {
// /* Parent */ pid_t wpid = wait(&child_status); if (WIFEXITED(child_status)) printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status)); else printf("Child %d terminate abnormally\n", wpid); } }

    while(ccount>0 && (pid = wait(&child_status))){

        if(pid < 0);
        --ccount;
        // Sio_puts("Handler reaped child ");
        // Sio_putl((long)pid);
        // Sio_puts(" \n");
        sleep(1);               /* Pretend cleanup work */
    }
    if(ccount == 0)
        done = 1;
    errno = olderrno;
}

// void fork14() {
//     pid_t pid[N];
//     int i;
//     ccount = N;
//     Signal(SIGCHLD, child_handler);
//     for (i = 0; i < N; i++) {
//         if ((pid[i] = Fork()) == 0) {
//             printf("Hi %d\n", (int) getpid());
//             Sleep(1);
//             exit(0);  /* Child exits */
//         }
//     }
//     while (!done) /* Parent spins */
//         ;
// }

void convert(char* file_name,Conversion* conversion,PRINTER* p){
    int masterpid;
    if ((masterpid = fork()) < 0)
    {
        char* msg = "fork error:\n";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        // fprintf(stderr, "fork error: %s\n", strerror/(errno));
        exit(0);
    }
    if (masterpid == 0) {
        /* master Child */
        int nchildren = conversion->path_length-1;
        pid_t pid[nchildren];
        int i,child_status;
        // ccount = nchildren;
        // signal(SIGKILL, kill_handler);

        //my pipes
        int pipe1 [2];
        int pipe2 [2];
        int* readPipe = pipe1;
        int* writePipe = pipe2;


        if(pipe(pipe1) < 0 || pipe(pipe2) < 0) {
            perror("Can't create pipe");
            exit(1);
        }
        int in_fd ;
        if ((in_fd = open(file_name, O_RDONLY)) < 0) {
                        perror("open");
                        //maybe send a signal to master
                        exit(1);
                    }
        int out_fd = imp_connect_to_printer(p, PRINTER_NORMAL);
        fprintf(stderr, "%s\n","master" );
        int * path = conversion->path;
        Conversion* c;

        // signal(SIGCHLD, child_handler);
        fprintf(stderr, "child nums:%d\n",nchildren );
        for (i = 0; i < nchildren; ++i)
        {
            c = &info.conversionMatrix[path[i]][path[i+1]];
            //pipe takes array of 2 int, [0] for reading ,[1] for writing
            //close(array[0]) -> i want to write
            //close(array[1]) -> i want to readdd
            // conversion = info.conversionMatrix[typeIDfrom][typeIDto];
            if ((pid[i] = fork()) == 0){
                //if first child
                if(i==0){
                    fprintf(stderr, "child number %d\n", i);

                    if(dup2(in_fd, STDIN_FILENO)<0)
                        fprintf(stderr, "dup2 no work at in fd to stdin\n" );

                    // close  both end for readPipe, not using it
                    if(dup2(writePipe[1],STDOUT_FILENO)<0)
                        fprintf(stderr, "dup2 no work at writepipe 1 to stdout\n" );

                    close(readPipe[0]);
                    close(readPipe[1]);
                    //close read end, aka writing
                    close(writePipe[0]);

                    fprintf(stderr, "%s\n",c->name );
                    int i = 0;
                    while(c->args[i]!=NULL){
                        fprintf(stderr,"%s  ",c->args[i++]);
                    }
                    //call program
                    int result = execvp(c->name,c->args);

                    fprintf(stderr, "ying is wrong %d\n",result );
                    _exit(EXIT_FAILURE);

                }
                //if last child
                else if(i==nchildren-1){

                    if(dup2(out_fd, STDOUT_FILENO)<0)
                        fprintf(stderr, "dup2 no work at out fd to stdout\n" );
                    if(dup2(readPipe[0],STDIN_FILENO)<0)
                        fprintf(stderr, "dup2 no work at readpipe 0 to stdin\n" );

                    //close write end, aka reading


                    close(readPipe[1]);
                    // close(readPipe[0]);

                    // close  both end for writePipe, not using it
                    close(writePipe[0]);
                    close(writePipe[1]);


                    fprintf(stdout, "%s\n",c->name );
                    //call program
                    int result = execvp(c->name,c->args);

                    fprintf(stderr, "ying is wrong %d\n",result );
                    _exit(EXIT_FAILURE);
                }
                else{

                    dup2(readPipe[0],STDIN_FILENO);
                    dup2(writePipe[1], STDOUT_FILENO);
                    //close write end, aka reading
                    close(readPipe[1]);
                    // close read end, aka writing
                    close(writePipe[0]);

                    //call program
                    execvp(c->name,c->args);

                }
                fprintf(stderr, "eixt chilld number %d\n",i );
                exit(0);
            }
            int * temp = readPipe;
            readPipe = writePipe;
            writePipe = temp;
        }
        // close  both end for readPipe and write pipe
        close(readPipe[0]);
        close(readPipe[1]);
        close(writePipe[0]);
        close(writePipe[1]);
        // /* Parent spins */
        // while (!done){
        //     fprintf(stderr, "%s ","not done" );
        // }
        for (i = 0; i < nchildren; i++) {
            /* Parent */
            pid_t wpid;
            while( (wpid=wait(&child_status))>0){
            if (WIFEXITED(child_status))
                printf("Child %d terminated with exit status %d\n",
                    wpid, WEXITSTATUS(child_status));
            else
                printf("Child %d terminate abnormally\n", wpid);
        }
        }

        exit(0);

    }

}
void print(char* wordList[],int wordCount)
{
        char* file_name = wordList[1];
        //find extension aka find str after .
        int typeID =  getFileTypeID(file_name);
        //if type not found
        if(typeID<0){
            char* msg = "type not found";
            imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
            return;
        }
        PRINTER_SET  set = 0;
        for (int i = 2; i < wordCount; ++i)
        {
            int printerID = findPrinterID(wordList[i]);
            if(printerID<0){
                 char* msg = "printer not found";
                imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
                return;
            }

                set|= 0x1<<printerID;
        }
        if(set==0)
            set=ANY_PRINTER;

        JOB* j = createJob(file_name,typeID,set);
        enqueue_job(&info.q,j);
        // eligible printer can only be used to print a job
        //     if there is a way to convert the file in the
        //     job to the type that can be printed by that printer.
        //     check
        // check if any printer is avaliable to do this job
        for (int i = 0; i < info.printerCount; ++i)
        {
            if(set>>i & 1){
                PRINTER* p = &info.printerList[i];

                // if not busy and theres a path
                if(!p->busy){
                    // findPath();
                    int toTypeID = findTypeID(p->type);
                    if(typeID ==toTypeID){
                        execv("bin/cat",(char**){NULL});
                        return;
                    }
                    //bfs to find path
                    Conversion *c = &info.conversionMatrix[typeID][toTypeID];
                    int path[TPYEMAX];
                    Graph g;
                    build_graph(&g,1, info.typeCount);
                    int path_length = findShortestPath(&g,typeID,toTypeID,info.typeCount,path);
                    freeEdges(&g);
                    if(path_length){
                        //set first node
                        int index = 0;
                        c->path[index++] = path[path_length-1];

                        for (int i = 1; i < path_length; ++i){
                                Conversion c1 = info.conversionMatrix[path[path_length-i]][path[path_length-i-1]];
                            for(int j = 1;j<c1.path_length;j++){
                                c->path[index++]=c1.path[j];
                            }
                        }
                        c->path_length = index;
                    }
                    //if path found
                    if(c->path_length){
                        // for (int i = 0; i < c->path_length; ++i)
                        // {
                        //     fprintf(stderr,"%d ",c->path[i]);
                        // }
                        // fprintf(stderr,"\n");
                        convert(file_name,c,p);
                    }
                }
            }
        }
        //if printer is found,print it
}
int excuteCommand(char* line)
{
    //word list
    // dynamically allocated
    int wordList_size = 10;
    char** wordList = malloc(wordList_size* sizeof(char*));//argv
    //arg count = len of list?

    // char* buff = malloc(256);
    int wordCount = 0;//argc

        char* word = strtok(line, " ");
    while( word !=NULL){
        if(wordCount>wordList_size){
            wordList_size*=2;
            wordList = realloc(wordList,sizeof(char*)*(wordList_size));
        }
        wordList[wordCount++] = word;
        word = strtok(NULL, " ");
    }
    init_options();
    //The help command takes no arguments, and it responds by printing a message
    //that lists all of the types of commands understood by the program.
    if(wordCount == 1 && strcmp(wordList[0],"help")==0){
        usage();
    }
    //The quit command takes no arguments and causes execution to terminate.
    else if(wordCount == 1 && strcmp(wordList[0],"quit")==0){
        printInfo();
        exit(0);
    }
    else if(wordCount == 2 && strcmp(wordList[0],"type")==0 ){
        //if type defined then ignore
        //if max type reached then ignore
        //save file_type at info
        //increase type count
        declareType(wordList[1]);
    }

    // The printer command declares the existence of a printer named printer_name,
    else if( wordCount == 3 &&strcmp(wordList[0],"printer")==0 ){
        declarePrinter(wordList[1],wordList[2]);
    }
    // The conversion command declares that files of type file_type1 can be
    // converted into file_type2 by running program conversion_program with any
    // arguments that have been indicated.
    else if(wordCount >=3 && strcmp(wordList[0],"conversion")==0)
    {
        declareConversion(wordList,wordCount);

    }
    //The printers command prints a report on the current status of the declared printers,
    else if(wordCount ==1 && strcmp(wordList[0],"printers")==0 ){
        printerReport();
    }

    // The jobs command prints a similar status report for the print jobs that have
    // been queued
    else if(wordCount ==1 && strcmp(wordList[0],"jobs")==0 ){
        jobReport();
    }
    // The print command sets up a job for printing file_name.
    // eligible printers for this job.  Only a printer in the set of eligible printers
    // for a job should be used for printing that jobs.  Moreover, an eligible printer
    // can only be used to print a job if there is a way to convert the file in the
    // job to the type that can be printed by that printer.
    // If no printer name is specified in the print command, then any declared
    // printer is an eligible printer.
    // fprintf(stderr, "%d\n",(wordCount >0 && strcmp(wordList[0],"print")==0) );
    else if(wordCount >0 && strcmp(wordList[0],"print")==0  ){
        // fprintf(stderr, "%s\n", "printing");
        print(wordList,wordCount);
    }

    // The cancel command cancels an existing job.  If the job is currently being
    // processed, then any processes in the conversion pipeline for that job
    // are terminated (by sending a SIGTERM signal to their process group).

    // The pause command pauses a job that is currently being processed.
    // Processes in the conversion pipeline for that job are stopped
    // (by sending a SIGSTOP signal to their process group).

    // The resume command resumes a job that was previously paused.
    // Processes in the conversion pipeline for that job are continued
    // (by sending a SIGCONT signal to their process group).

    // The disable command sets the state of a specified printer to "disabled".
    // This does not affect the status of any job currently being processed
    // by that printer, but a disabled printer is not eligible to accept any
    // further jobs until it has been re-enabled using the enable commnd.

    // The enable command sets the state of a specified printer to "enabled".
    // When a printer becomes enabled, if there is a pending job that can now be
    // processed by the newly enabled printer, then processing is immediately
    // started for one such job.
    else{
        char* msg = "invalid command";
        imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
        return -1;
    }

    if(word) free(word);
    if(wordList) free(wordList);
    return 0;

}

