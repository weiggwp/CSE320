/*
 * A client registry keeps track of the file descriptors for clients
 * that are currently connected.  Each time a client connects,
 * its file descriptor is added to the registry.  When the thread servicing
 * a client is about to terminate, it removes the file descriptor from
 * the registry.  The client registry also provides a function for shutting
 * down all client connections and a function that can be called by a thread
 * that wishes to wait for the client count to drop to zero.  Such a function
 * is useful, for example, in order to achieve clean termination:
 * when termination is desired, the "main" thread will shut down all client
 * connections and then wait for the set of registered file descriptors to
 * become empty before exiting the program.
 */
#include <stdlib.h>
#include "csapp.h"

#define FDMAX 1024
// fd<1024 2^10
typedef struct client_registry {
    volatile int registryset[FDMAX];
}CLIENT_REGISTRY;
#include "client_registry.h"

pthread_mutex_t lock;
pthread_mutex_t lock2;

// volatile int empty;
volatile int num_client;
sem_t empty;


/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry.
 */
CLIENT_REGISTRY *creg_init()
{
    if (pthread_mutex_init(&lock, NULL) != 0 ||pthread_mutex_init(&lock2, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(EXIT_FAILURE);
    }

    sem_init(&empty, 0, 0);

    return Calloc(1,sizeof(CLIENT_REGISTRY));
}


 // * Finalize a client registry.
 // *
 // * @param cr  The client registry to be finalized, which must not
 // * be referenced again.

void creg_fini(CLIENT_REGISTRY *cr){
    free(cr);
    //TODO: do in need to destroy the lock
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 */
void  creg_register(CLIENT_REGISTRY *cr, int fd)
{
    pthread_mutex_lock(&lock);
    cr->registryset[fd] = 1;
    num_client++;
    pthread_mutex_unlock(&lock);
}

/*
 * Unregister a client file descriptor, alerting anybody waiting
 * for the registered set to become empty.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be unregistered.
 */
void creg_unregister(CLIENT_REGISTRY *cr, int fd)
{
    pthread_mutex_lock(&lock);
    cr->registryset[fd] = 0;
    num_client--;
    //TODO: alert waiting bodies, use symophone
    if(num_client==0)
        V(&empty); //post
    pthread_mutex_unlock(&lock);
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr)
{
    if(num_client)
        P(&empty); //wait
}

/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
    pthread_mutex_lock(&lock2);
    for (int i = 0; i < FDMAX; ++i)
    {
        if(cr->registryset[i]==1){
            shutdown(i,FDMAX);
            creg_unregister(cr,i);
        /* code */
        }
    }
    pthread_mutex_unlock(&lock2);
}

