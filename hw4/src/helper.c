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
 {HELP,         "help",    0,      no_argument, NULL,
                  "Print valid command table."},
 {QUIT,         "quit",   0,      no_argument, NULL,
                  "Quit."},
 {TYPE,         "type",     0,        required_argument, "file_type",
                  "Declare type."},
 {PRTER,        "printer",    0,        required_argument, "printer_name file_type",
                  "Declare printer."},
 {CONVERSION,   "conversion", 0,        required_argument, "type1 type2 program [arg1 arg2 ...]",
                  "Declare conversion."},
 {PRINTERS,     "printers",     0,        no_argument, NULL,
                  "Print a report on the current status of the declared printers."},
 {JOBS,         "jobs",     0,        no_argument, NULL,
                  "Print a report on the status of the queued jobs."},
 {PRINT,        "print",    0,        required_argument, "file_name [printer1 printer2 ...]",
                  "Set up a job for printing file_name."},
 {CANCEL,       "cancel",    0,        required_argument, "job_number",
                  "Cancel an existing job."},
 {PAUSE,        "pause",    0,        required_argument, "job_number",
                  "Pause a job that is currently being processed."},
 {RESUME,       "resume",       0,      required_argument, "job_number",
                  "Resume a job that was previously paused."},
 {DISABLE,      "disable",    0,      required_argument, "printer_name",
                  "Set the state of a specified printer to disabled."},
 {ENABLE,       "enable",   0,      required_argument, "printer_name",
                  "Set the state of a specified printer to enabled."},
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
        fprintf(info.outfile, "Valid options are:\n");
        for(unsigned int i = 0; i < NUM_OPTIONS-1; i++) {
                opt = &option_table[i];
                char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
                if(opt->chr)
                  sprintf(optchr, "-%c, ", opt->chr);
                char arg[32];
                if(opt->has_arg)
                    sprintf(arg, " <%.40s>", opt->argname);
                else
                    sprintf(arg, "%.13s", "");
                fprintf(info.outfile, "%s--%-10s%-35s\t%s\n",
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
    //check if id is -1
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
    //check if id is -1
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
void printerReport(PRINTER* p){
    fprintf(info.outfile,"%s\n",
            imp_format_printer_status(p,(char*)info.buf,BUFSIZE));
}
void printerTable(){
        PRINTER* printerList = info.printerList;
        for (int i = 0; i < info.printerCount; ++i)
        {
            printerReport(&printerList[i]);
        }
}
void jobReport(JOB* j){
    fprintf(info.outfile,
        "%s\n",
        imp_format_job_status(j,(char*)info.buf,BUFSIZE));
}
void jobTable(){
    JOB* j = info.q.front;
    while(j!=NULL){
        jobReport(j);
        j=j->other_info;
    }
}
void updateJobChange_time(JOB* j){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    j->change_time=tv;
}
void updateJob(JOB* j,int status){
    j->status = status;
    updateJobChange_time(j);
    jobReport(j);

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
    jobReport(j);
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

void updatePrinter(PRINTER* p,int busy){
    p->busy=busy;
    printerReport(p);

}


void child_handler(int sig) {
    pid_t pid;
    int wstatus;
    // printf("signal\n");
    while((pid = waitpid(-1,&wstatus, WNOHANG|WUNTRACED | WCONTINUED))>0){
        // fprintf(stderr, "master %d interrupted\n",pid );
        JOB* j = info.q.front;
        while(j!=NULL){
            if(j->pgid ==pid)
                break;
            j=j->other_info;
        }
        //paused
         if(WIFSTOPPED(wstatus)){
            fprintf(stderr, "master %d PAUSED\n",pid );

            updateJob(j,PAUSED);
        }
        //resumed
        else if(WIFCONTINUED(wstatus)){
        fprintf(stderr, "master %d resumed\n",pid );

            updateJob(j,RUNNING);
        }
        //exited normally
        else if(WIFEXITED(wstatus)){
        fprintf(stderr, "master %d COMPLETED\n",pid );
            updateJob(j,COMPLETED);
            updatePrinter(j->chosen_printer,0);
            // j->chosen_printer->busy=0;
            // fprintf(stderr, "%s%d\n",j->chosen_printer->name,j->chosen_printer->busy);
            findJob(j->chosen_printer);
        }
        //terminated
        else{
        fprintf(stderr, "master %d terminated\n",pid );

            updateJob(j,ABORTED);
            updatePrinter(j->chosen_printer,0);

            // j->chosen_printer->busy=0;
            // findJob(j->chosen_printer);

        }
    }
}

void errorReport(char*msg,int exit){
    char* newMsg = imp_format_error_message(msg, (void*)info.buf, BUFSIZE);
    fprintf(info.outfile, "%s\n",newMsg );
    if(exit)
        _exit(EXIT_FAILURE);
}
int mydup2(int fd1,int fd2){
    if(dup2(fd1, fd2)<0){//stdin = in file
        snprintf((char*)info.buf, sizeof info, "cannot dup2 at %d to %d\n",fd1,fd2);

        // char* msg = "dup2 no work\n";
        errorReport((char*)info.buf,1);
    }
    return 1;
}
void directPrint(int in_fd,int out_fd){

    mydup2(in_fd, STDIN_FILENO);
    mydup2(out_fd, STDOUT_FILENO);
    char name[4] = "cat";
    char*args[2] = {name,NULL};
    execvp("cat",args);
    _exit(1);
}
void sig(int sig)
{
    printf("it went in here\n");
}
void convert(char* file_name,Conversion* conversion,PRINTER* p,JOB* j){

    //install handler
    signal(SIGCHLD, child_handler);
    //signal(SIGSTOP,sig);
    //block all signals
    // struct  sigaction sa;
    // sa.sa_handler = child_handler;
    // sigemptyset(&sa.sa_mask);
    // sigaction(SIGCHLD,&sa,0);
    sigset_t mask_all;
    sigemptyset(&mask_all);
    // sigfillset(&mask_all);
    sigaddset(&mask_all,SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask_all, NULL);
    int masterpid;
    if ((masterpid = fork()) < 0){
        char* msg = "fork master error\n";
        errorReport(msg,1);
    }

    /* master Child */
    if (masterpid == 0) {
        signal(SIGCHLD,SIG_DFL);
        setpgid(getpid(),getpid());
        // sigset_t empty;
        // sigemptyset(&empty);
        // sigprocmask(SIG_SETMASK, &empty, NULL);

        // fprintf(stderr, "master started:PID:%d,PGID:%d\n",getpgrp(),getpid());
        // sleep(2);

        int out_fd = imp_connect_to_printer(p, PRINTER_NORMAL);
        if(out_fd<0){
            char* msg = "cannot connect to printer\n";
            errorReport(msg,1);
        }
         int in_fd;
        if ((in_fd = open(file_name, O_RDONLY)) < 0) {
            char* msg = "cannot open in file\n";
            errorReport(msg,1);
        }
        //debug
        // fprintf(stderr, "%d,%dconversion->path_length:%d\n",
            // in_fd,out_fd,conversion->path_length );

        //path_length ==1 means no conversion needed,
        //the printer can do job directly
        if(conversion->path_length==1){
            directPrint(in_fd,out_fd);
        }
        //path_length ==2 means one conversion needed,
        //no pipe needed
        if(conversion->path_length==2){
            // fprintf(stderr, "%s\n","path2" );
            // printf("%s\n",conversion->name );
            mydup2(in_fd, STDIN_FILENO);
            mydup2(out_fd, STDOUT_FILENO);
            execvp(conversion->name,conversion->args);
            char* msg = "cannot execvp\n";
            errorReport(msg,1);
        }

        //PIPELINE
        int nchildren = conversion->path_length-1;
        pid_t pid[nchildren];
        int i,child_status;

        //my pipes
        int pipeList [2*nchildren];

        //ptr to pipe for swapping purpose

        // fprintf(stderr, "%s%d\n","master",nchildren );
        int * path = conversion->path;
        Conversion* c;
        // pipe(pipeList);
        int* writePipe;
        int* readPipe;
        for (i = 0; i < nchildren; ++i)
        {
             writePipe =&pipeList[2*i];
             if(i>0)
                readPipe =&pipeList[2*(i-1)];
            // int* writePipe = pipeList[2*(i+1)];
            if(pipe(writePipe) < 0) {
                char* msg = "cannot create pipe\n";
                errorReport(msg,1);
            }
            // fprintf(stderr,"%d:%d\n",path[i],path[i+1] );
            c = &info.conversionMatrix[path[i]][path[i+1]];
            //pipe takes array of 2 int, [0] for reading ,[1] for writing
            //close(array[0]) -> i want to write
            //close(array[1]) -> i want to readdd
            if ((pid[i] = fork()) == 0){
                //if first child
                if(i==0){
                    mydup2(in_fd, STDIN_FILENO);//stdin = in file
                    mydup2(writePipe[1], STDOUT_FILENO);// out = writepipe[1]
                    // close(readPipe[0]);//close read pipe, not using it
                }
                //if last child
                else if(i==nchildren-1){
                    mydup2(readPipe[0], STDIN_FILENO);// in = readpipe[0]
                    mydup2(out_fd, STDOUT_FILENO);//stdout = out file
                    close(writePipe[1]);//close write pipe, not using it
                    // fprintf(stdout, "%s\n","ggweg" );

                }
                else{
                    mydup2(readPipe[0], STDIN_FILENO);//stdin = readpipe[0]
                    // fprintf(stderr, "%p\n", );
                    mydup2(writePipe[1], STDOUT_FILENO);// out = writepipe[1]
                }
                if(i>0)
                    close(readPipe[1]); //close write end of read pipe
                close(writePipe[0]);//close read end of write pipe
                //call program
                execvp(c->name,c->args);
                char* msg = "cannot execvp\n";
                errorReport(msg,1);
            }
            if(i>0){
                close(readPipe[0]);
                close(readPipe[1]);
            }

            //close prev pipe
            //swap pipes
            // int * temp = readPipe;
            // readPipe = writePipe;
            // writePipe = temp;
        }
        // close both end for readPipe and write pipe
        //not used by master
        close(readPipe[0]);
        close(readPipe[1]);
        close(writePipe[0]);
        close(writePipe[1]);

        for (i = 0; i < nchildren; i++) {
            /* Parent */
            pid_t wpid;
            if((wpid=wait(&child_status))>0){
                // fprintf(stderr, "wpid:%d(should be %d:%d)\n",wpid,i,pid[i] );
                if (WIFEXITED(child_status))
                    printf("Child %d terminated with exit status %d\n",
                        wpid, WEXITSTATUS(child_status));
                else{
                    char* msg = "children terminate abnormally\n";
                    errorReport(msg,1);
                    // printf("Child %d terminate abnormally\n", wpid);
                }
            }
        }
        _exit(0);

    }
    j->pgid = masterpid;
    sigprocmask(SIG_UNBLOCK, &mask_all, NULL);
}
void runJob(JOB *j){

    char* file_name = j->file_name;
    PRINTER_SET set = j->eligible_printers;

    //find extension aka find str after .
    int typeID =  getFileTypeID(file_name);

    for (int i = 0; i < info.printerCount; ++i)
    {
        //printer is in the set or not
        if(set>>i & 1){
            PRINTER* p = &info.printerList[i];

            // if not busy and theres a path
            if(!p->busy && p->enabled){
                int toTypeID = findTypeID(p->type);
                //bfs to find path
                Conversion *c = &info.conversionMatrix[typeID][toTypeID];
                if(typeID ==toTypeID){
                    c->path_length = 1;

                }
                else{
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
                }
                //if path found
                if(c->path_length){
                    // for (int i = 0; i < c->path_length; ++i)
                    // {
                    //     fprintf(stderr,"%d ",c->path[i]);
                    // }
                    // fprintf(stderr,"\n");
                    updatePrinter(p,1);
                    j->chosen_printer = p;
                    updateJob(j,RUNNING);
                    convert(file_name,c,p,j);
                }
            }
        }
    }
    //if printer is found,print it
}
void findJob(PRINTER*p){
    JOB* j = info.q.front;
    while(j!=NULL){
        if(j->status ==QUEUED && j->eligible_printers & (0x1 << p->id)){
            // fprintf(stderr, "eligible_printers%d\n",p->id );
            runJob(j);
        }

        j=j->other_info;
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
    runJob(j);

}
JOB* getJobFromID(int jobID){
    if(jobID>=info.jobCount){
        char* msg = "invalid jobID";
        errorReport(msg,0);
        return NULL;
    }
    JOB* j = info.q.front;
    while(j!=NULL){
        if(j->jobid==jobID)
            break;
        j=j->other_info;
    }
    if(j==NULL){
        char* msg = "invalid jobID";
        errorReport(msg,0);
        return NULL;
    }
    return j;
}

int jobKill(int id){
    JOB*j = getJobFromID(id);
    // fprintf(stderr, "Cancel job:%d\n",id );
    // char * s = imp_format_job_status(j, (char*)info.buf, BUFSIZE);
    // fprintf(stderr, "%s\n",s );
    if(j->status==ABORTED){
        char* msg = "job already ABORTED\n";
        errorReport(msg,0);
        return -1;
    }
    if(j->status==QUEUED){
        j->status = ABORTED;
    }
    else if(j->status == PAUSED){
        // fprintf(stderr, "kiling paused process:pgid:%d\n",j->pgid);
        kill(j->pgid,SIGCONT);
        kill(j->pgid,SIGTERM);
    }
    else{
        // fprintf(stderr, "kiling running process:pgid:%d\n",j->pgid);
        kill(j->pgid,SIGTERM);
    }
    return 1;

}
int jobPause(int id){
    JOB*j = getJobFromID(id);
    if(j->status==QUEUED){
        char* msg = "cannot pause a unstarted job\n";
        errorReport(msg,0);
        return -1;
    }
    if(j->status==ABORTED){
        char* msg = "job already ABORTED\n";
        errorReport(msg,0);
        return -1;
    }
    if(j->status==PAUSED){
        char* msg = "job already PAUSED\n";
        errorReport(msg,0);
        return -1;
    }
    else{
        // fprintf(stderr, "pausing:%d:%s:%d\n",j->jobid,j->file_name,j->pgid);
        kill(j->pgid,SIGSTOP);
    }
    return 1;

}
int jobCont(int id){
    JOB*j = getJobFromID(id);
    if(j->status==QUEUED){
        char* msg = "job not unstarted\n";
        errorReport(msg,0);
        return -1;
    }
    if(j->status==ABORTED){
        char* msg = "job already ABORTED\n";
        errorReport(msg,0);
        return -1;
    }
    if(j->status==RUNNING){
        char* msg = "job already RUNNING\n";
        errorReport(msg,0);
        return -1;
    }
    else{
        // fprintf(stderr, "resuming:%d:%s:%d\n",j->jobid,j->file_name,j->pgid);
        kill(j->pgid,SIGCONT);
    }
    return 1;

}
int printerSetEnable(char* name,int enable){
    int printerID = findPrinterID(name);
    if(printerID<0){
        char* msg = "invalid printerID";
        errorReport(msg,0);
        return -1;
    }
    info.printerList[printerID].enabled = enable;
    return 1;
}
// void deleteJob(JOB *head_ref, int key)
// {
//     // Store head node
//     struct Node* temp = *head_ref, *prev;

//     // If head node itself holds the key to be deleted
//     if (temp != NULL && temp->data == key)
//     {
//         *head_ref = temp->next;   // Changed head
//         free(temp);               // free old head
//         return;
//     }

//     // Search for the key to be deleted, keep track of the
//     // previous node as we need to change 'prev->next'
//     while (temp != NULL && temp->data != key)
//     {
//         prev = temp;
//         temp = temp->next;
//     }

//     // If key was not present in linked list
//     if (temp == NULL) return;

//     // Unlink the node from linked list
//     prev->next = temp->next;

//     free(temp);  // Free memory
//     }
void deqOldJob(){
    //remove job from the queue after 1 min of completion or abortion
    // fprintf(stderr, "checking job end time\n");
    JOB* j = info.q.front;
    JOB* prev_j = NULL;
    while(j!=NULL){
         struct timeval tv;
        gettimeofday(&tv,NULL);

        if((j->status==COMPLETED || j->status==ABORTED)
         && tv.tv_sec - j->change_time.tv_sec  >=60){
            // fprintf(stderr, "deqing\n" );
            if(prev_j==NULL){
                if(info.q.front==info.q.rear)
                    info.q.front=info.q.rear=NULL;
                else{
                    info.q.front=j->other_info;
                }
            }
            else{
                prev_j->other_info=j->other_info;
            }
            free(j);
            info.jobCount--;
        }
        else
            prev_j=j;
        j=j->other_info;
    }
}
int excuteCommand(char* line)
{
    //word list
    // dynamically allocated
    int wordList_size = 100;
    char* wordList[wordList_size * sizeof(char*)];//argv
    // fprintf(stderr, "%p\n",wordList);
    //arg count = len of list?

    // char* buff = malloc(256);
    int wordCount = 0;//argc

    char* word = strtok(line, " ");
    while( word !=NULL){

        // if(wordCount>=wordList_size){
        //     wordList_size*=2;
        //     wordList = realloc(wordList,sizeof(char*)*(wordList_size));
        // }
        wordList[wordCount++] = word;
        // free(word);
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
        // printInfo();
        return 0;
        // exit(0);
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
    else if(wordCount >=4 && strcmp(wordList[0],"conversion")==0)
    {
        declareConversion(wordList,wordCount);

    }
    //The printers command prints a report on the current status of the declared printers,
    else if(wordCount ==1 && strcmp(wordList[0],"printers")==0 ){
        printerTable();
    }

    // The jobs command prints a similar status report for the print jobs that have
    // been queued
    else if(wordCount ==1 && strcmp(wordList[0],"jobs")==0 ){
        deqOldJob();
        jobTable();
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
    else if(wordCount ==2 && strcmp(wordList[0],"cancel")==0  ){
        // fprintf(stderr, "canceling:%d\n",atoi(wordList[1]));
        jobKill(atoi(wordList[1]));
        // jobCommand(atoi(wordList[1]),SIGTERM);
    }
    else if(wordCount ==2 && strcmp(wordList[0],"pause")==0  ){
        jobPause(atoi(wordList[1]));
        // jobCommand(atoi(wordList[1]),SIGSTOP);
    }
    else if(wordCount ==2 && strcmp(wordList[0],"resume")==0  ){
        jobCont(atoi(wordList[1]));
        // jobCommand(atoi(wordList[1]),SIGCONT);
    }
    else if(wordCount ==2 && strcmp(wordList[0],"disable")==0  ){
        printerSetEnable(wordList[1],0);
    }
    else if(wordCount ==2 && strcmp(wordList[0],"enable")==0  ){
        printerSetEnable(wordList[1],1);
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
        errorReport(msg,0);
        return -1;
    }
    return 1;

}

// void freeStorage(){
//     for(int i=0;i<info.typeCount;i++){
//         free(info.typeList[i].name);
//     }
//     for (int i = 0; i < info.printerCount; ++i)
//     {
//         fre(info.printerList[i].name);
//         free(info.printerList[i].type);
//     }
// }