#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <readline/readline.h>
#include <signal.h>
#include "imprimer.h"
#include "helper.h"

/*
 * "Imprimer" printer spooler.
 */
// typedef char* TypePrinter;

int main(int argc, char *argv[])
{

    char optval;
    FILE* infile;
    initInfo();

    while(optind < argc) {
    	if((optval = getopt(argc, argv, "i:")) != -1) {
    	    switch(optval) {
            case 'i':
                infile = fopen(optarg, "r");
                //open the next arg as file, if faied then exit
                if (infile ==NULL)
                {
                    fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                if(optind<argc)
                    info.outfile = fopen(optarg, "w");
                //open the next arg as file, if failed then exit
                if (info.outfile==NULL )
                {
                    fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                //opened infile and/or outfile successfully
                //TODO: read each line as command
                char * line = NULL;
                size_t len = 0;
                ssize_t read;
                while((read = getline(&line, &len, infile)) != -1)
                {
                    if(line[strlen(line)-2] == '\r' )
                        line[strlen(line)-2] = 0;
                    else if(line[strlen(line)-1] == '\n' )
                        line[strlen(line)-1] = 0;
                    excuteCommand(line);
                    // free(line);
                }
                // while(fgets(line, sizeof(line), infile)){
                //     excuteCommand(line);
                // }
            break;
    	    case '?':
    		fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
    		exit(EXIT_FAILURE);
    		break;
    	    default:
    		break;
    	    }
    	}
    }
    // for(int i=0;i<info.typeCount;i++){
    //     free(info.typeList[i].name);
    // }
    // for (int i = 0; i < info.printerCount; ++i)
    // {
    //     free(info.printerList[i].name);
    //     free(info.printerList[i].type);
    // }
    // freeStorage();
    // exit(EXIT_SUCCESS);
    int cont = 1;
    while(cont){
        char* line = readline("imp> ");
        if(line !=NULL && strcmp(line,"")!=0 && strcmp(line,"\n")!=0 )
            cont = excuteCommand(line);
        if(line!=NULL)
            free(line);
    }

    // kill(-1,SIGKILL);
    printf("%s\n","Bye" );
    exit(EXIT_SUCCESS);
}
