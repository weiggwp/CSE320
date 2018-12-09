#include <getopt.h>
#include <string.h>

#include "helper.h"
#include "csapp.h"
#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "server.h"

static void terminate(int status);
// /* Thread routine */
// void *thread(void *vargp)
// {
//     int* connfd = Malloc(sizeof(int));
//     *connfd = *((int *)vargp);
//     // Pthread_detach(pthread_self());

//     xacto_client_service(connfd);

//     // Free(vargp);
//     // printf("%d\n", connfd);
//     // Close(connfd);
//     return NULL;
// }

void hup_handler(int sig) {
    // printf("printttt fffff\n");
    terminate(EXIT_SUCCESS);
}

CLIENT_REGISTRY *client_registry;
int listenfd;
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char optval;
    FILE* infile;
    // initInfo();
    char hostname[256];
    char port[256];

    while(optind < argc) {
        if((optval = getopt(argc, argv, "h:p:q:")) != -1) {
            switch(optval) {
            case 'h':
                //TODO: handle -h
                // temp_arg = optarg;
                printf("%s\n", optarg);
                printf("%s\n", optarg);
                strncpy(hostname,optarg,strlen(optarg)+1);
            break;
            case 'p':
                //TODO: handle -p
                // printf("%s\n", optarg);
                // port = convert(optarg);
                strncpy(port,optarg,strlen(optarg)+1);

            break;
            case 'q':
                infile = fopen(optarg, "r");
                //open the next arg as file, if failed then exit
                if (infile==NULL)
                {
                    fprintf(stderr, "Usage: %s [-q <cmd_file>]\n", argv[0]);
                }

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

    // if(!validPortNumber(port_number)){
    //     fprintf(stderr, "Invalid port_number:%u\n",port_number);
    //     exit(EXIT_FAILURE);
    // }
    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.


    int*connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = Open_listenfd(port);
    Signal(SIGHUP,hup_handler);
    while (1)
    {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd,(SA *) &clientaddr, &clientlen);
        // debug("thread started");
        Pthread_create(&tid, NULL, xacto_client_service, connfdp);
    }



    // fprintf(stderr, "You have to finish implementing main() "
	   //  "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    shutdown(listenfd,SHUT_RD);
    close(listenfd);
    debug("Xacto server terminating");
    exit(status);
}
