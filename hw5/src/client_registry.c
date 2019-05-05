#include "client_registry.h"
#include "debug.h"
#include <stdlib.h>
#include "csapp.h"
#include <sys/socket.h>

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

typedef struct NODE{
    int *fd;
    struct NODE *next;
}NODE;

typedef struct client_registry{
    int num_connected;
    NODE *registry;
    sem_t mutex;
    sem_t blocking;
}CLIENT_REGISTRY;


CLIENT_REGISTRY *client_registry;

/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry.
 */
CLIENT_REGISTRY *creg_init()
{
    debug("initalization of client registry");
    CLIENT_REGISTRY *new = malloc(sizeof(CLIENT_REGISTRY));
    new->registry = NULL;
    new->num_connected = 0;
    Sem_init(&new->mutex,0,1);
    Sem_init(&new->blocking,0,0);
    return new;
}

/*
 * Finalize a client registry.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr)
{

    debug("num_connected = %d",cr->num_connected);
    NODE *temp = cr->registry;
    int count = 0;
    while(temp != NULL)
    {
        if(count % 40 == 0)
            printf("\n");
        printf("%d ",*(temp->fd));
        temp = temp->next;
        count++;
    }
    //free(cr);
    //cr = NULL;
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 */
void creg_register(CLIENT_REGISTRY *cr, int fd)
{
    //debug("registered called");

    P(&cr->mutex);
    cr->num_connected = cr->num_connected + 1;
    NODE *prev = cr->registry;
    NODE *new = malloc(sizeof(NODE));
    cr->registry = new;
    new->fd = malloc(sizeof(int));
    *(new->fd) = fd;
    new->next = prev;
    V(&cr->mutex);
    //V(&cr->blocking);
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
   // debug("unregistered called");
   // P(&cr->blocking);
    P(&cr->mutex);
    cr->num_connected = cr->num_connected -1;
    NODE *temp = cr->registry;
    NODE *prev = NULL;
    int found = 0;
    while(temp != NULL)
    {
        int current_fd = *(temp->fd);
        if(current_fd == fd)
        {
            //remove it from registry
            free(temp->fd);

            //first item in list
            if(temp == cr->registry)
            {
                cr->registry = temp->next;
                free(temp);
            }
            //last item in list
            else if(temp->next == NULL)
            {
                prev->next = NULL;
                free(temp);
            }
            //middle item in list
            else
            {
                prev->next = temp->next;
                free(temp);
            }
            found = 1;
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    if(cr->num_connected == 0)
    {
        debug("num_connected is now 0");
        V(&cr->blocking);
    }
    if(found == 0)
        debug("did not find fd");

    V(&cr->mutex);
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
    debug("wait for empty called");
    P(&cr->blocking);

    debug("wait for empty ended");
}

/*
 * Shut down all the currently registered client file descriptors.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
    debug("shutdown all called");
    NODE *temp = cr->registry;
    while(temp != NULL)
    {
        int sockfd = *(temp->fd);
        creg_unregister(cr,sockfd);
        /*
        int shutdown_result = shutdown(sockfd,SHUT_RD);
        if(shutdown_result == -1)
        {
            fprintf(stderr, "%s\n","Error in shutdown()" );
        }
        */
        temp = cr->registry;
    }
    //V(cr->blocking);
}
