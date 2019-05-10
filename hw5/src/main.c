#include <stdlib.h>

#include "client_registry.h"
#include "maze.h"
#include "player.h"
#include "debug.h"
#include "server.h"

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "csapp.h"


static void terminate(int status);

static char *default_maze[] = {
  "******************************",
  "***** %%%%%%%%% &&&&&&&&&&& **",
  "***** %%%%%%%%%        $$$$  *",
  "*           $$$$$$ $$$$$$$$$ *",
  "*##########                  *",
  "*########## @@@@@@@@@@@@@@@@@*",
  "*           @@@@@@@@@@@@@@@@@*",
  "******************************",
  NULL
};


int string_to_int(char *s)
{
    int num = 0;
    while(*s != '\0')
    {
        if(*s >= '0' && *s <= '9')
        {
            //shifts digits to the left by one decimal place and add the current digit
            num = num *10 + *s - '0';
        }
        else
            return 0;
        ++s;
    }
    return num;
}
/*
typedef struct NODE{
    int *fd;
    struct NODE *next;
}NODE;

typedef struct client_registry{
    int num_connected;
    NODE *registry;
    sem_t mutex;
}CLIENT_REGISTRY;*/


void *testing_func(void *args)
{
  for( int i = 0 ;i < 800;i++)
    creg_register(client_registry,i);
  return NULL;
}

void *test2(void *args)
{
  debug("before wait call in test");
  creg_wait_for_empty(client_registry);
  debug("after wait call in test");
  //debug("num connected in test2 = %d",client_registry->num_connected);
  return NULL;
}

void terminate_handler()
{
  terminate(0);
}
void sig_pipe_ignore()
{
  //debug("sig_pipe executed");
  return;
}

int main(int argc, char* argv[]){



    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    int port = 0;
    int port_entered = 0;
    char *map_template = NULL;


    extern char *optarg;
    int c;
    while((c = getopt(argc,argv,"p:t:")) != -1)
    {
      switch(c)
      {
        case 'p':
          port_entered = 1;
          port = string_to_int(optarg);
          break;
        case 't':
          map_template = optarg;
          break;
        case '?':
          //fprintf(stderr, "%s\n", "Error with processing arguments.(invalid flags)");
          //terminate(EXIT_FAILURE);
          exit(1);
          break;
      }
    }

    if(port_entered == 0)
    {
      //fprintf(stderr, "%s\n","A port number was not specify" );
      //terminate(EXIT_FAILURE);
      exit(1);
    }

    char **input_template;
    if(map_template != NULL)
    {
      FILE *fp = fopen(map_template,"r");
      if(fp == NULL)
      {
        //fprintf(stderr, "%s\n", "ERROR OPENING TEMPLATE FILE");
        exit(1);
      }
      input_template = malloc(1000 * sizeof(char*));

      int buf_size = 250;
      char *line = malloc(buf_size *sizeof(char));
      long unsigned int size_line = 250;
      int getLine;
      char **temp_input = input_template;
      while( (getLine = getline(&line,&size_line,fp)) != -1)
      {
        //debug("getLine = %d",getLine);
        //debug("strlen = %ld",strlen(line));
        //debug("the character %c",*(line+30));
        char *newline;
        if(*(line+getLine-1) == '\n')
        {
          debug("executed");
          char *temp = line+getLine-1;
          *temp = '\0';
          newline = malloc(getLine *sizeof(char));
        }
        //char *newline = malloc(getLine *sizeof(char));
        else
        {
          newline = malloc(getLine + 1 *sizeof(char));
        }
        strcpy(newline,line);
        debug("%s",line);
        //debug("%p",*temp_input);
        *temp_input = newline;
        temp_input++;
      }
      *temp_input = NULL;
      fclose(fp);
      free(line);
      /*
      debug("");
      char **input_template2 = input_template;
      for(int i = 0; *input_template2 != NULL; i++)
      {
        debug("%s",*input_template2);
        debug("%ld",strlen(*input_template2));
        input_template2++;
      }
      */

    }

    // Perform required initializations of the client_registry,
    // maze, and player modules.
    creg_init();
    if(map_template == NULL)
      maze_init(default_maze);
    else
    {
      maze_init(input_template);

      char **input_template2 = input_template;
      while(*input_template2 != NULL)
      {
        debug("free executed");
        free(*input_template2);
        input_template2++;
      }
      free(input_template);
     // close(fp);
    }
    player_init();
    debug_show_maze = 1;  // Show the maze after each packet.


    maze_get_rows();
    maze_get_cols();


    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function mzw_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    //installing SIGHUP handler
    struct sigaction act;
    struct sigaction act2;
    memset(&act,0,sizeof(struct sigaction));
    memset(&act2,0,sizeof(struct sigaction));
    act.sa_handler = terminate_handler;
    act2.sa_handler = sig_pipe_ignore;

    int sig_result = sigaction(SIGHUP,&act,NULL);
    int sig_result2 = sigaction(SIGPIPE,&act2,NULL);
    if(sig_result == -1 || sig_result2 == -1)
    {
      //fprintf(stderr, "%s\n","sigaction function failed" );
      //terminate(EXIT_FAILURE);
      exit(1);
    }

    //setting up server socket and loop to accept connections and a thread started to run
    int listenfd;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    int *connfd;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(port);

   // int count = 0;
    while(1)
    {

      debug("inside loop");
      connfd = Malloc(sizeof(int));
      *connfd = Accept(listenfd,(SA *) &clientaddr,&clientlen);
      debug("accepted socket");
      //debug("thread is =%ld",tid);
      Pthread_create(&tid,NULL,mzw_client_service,connfd);
     // count++;
    }



   // fprintf(stderr, "You have to finish implementing main() "
	  //  "before the MazeWar server will function.\n");

   // terminate(1);

}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    debug("terminated called = %d",status);
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    //creg_fini(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    player_fini();
    maze_fini();

    debug("MazeWar server terminating");
    exit(status);
}
