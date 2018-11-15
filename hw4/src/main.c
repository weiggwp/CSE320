#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <readline/readline.h>
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

    // info info;
    // //list of Types, 32 types at max
    // info.typeList =  calloc(TPYEMAX,sizeof(TypePrinter));
    // info.typeCount = 0;
    // //list of printers
    // info.printerList = calloc(PRINTERMAX, sizeof(PRINTER));
    // info.printerCount = 0;

    // //list of conversions
    // //thoughts: create a 2d array stores conversion function name (str*)
    // info.conversionMatrix = calloc(TPYEMAX,sizeof(int*));

    // for(int i = 0; i < TPYEMAX; i++) {
    //    info.conversionMatrix[i] = calloc(TPYEMAX, sizeof(int));
    // }
    // Then access array elements like matrix[i][j]
    while(optind < argc) {
    	if((optval = getopt(argc, argv, "i:")) != -1) {
    	    switch(optval) {
            case 'i':
                infile = fopen(optarg, "r");
                //open the next arg as file, if faied then exit
                if (infile <0)
                {
                    fprintf(stderr, "Usage: %s [-i <cmd_file>] [-o <out_file>]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                if(optind<argc)
                    info.outfile = fopen(optarg, "w");
                //open the next arg as file, if failed then exit
                if (info.outfile <0)
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
    int quit = 0;
    while(!quit){
        char* line = readline("imp> ");
        if(line !=NULL && strcmp(line,"")!=0 && strcmp(line,"\n")!=0 )
            quit = excuteCommand(line);
        free(line);
    }
    printf("%s\n","Hello" );
    exit(EXIT_SUCCESS);
}
