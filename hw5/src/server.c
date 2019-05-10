
#include "server.h"
#include "csapp.h"
#include "debug.h"
#include "protocol.h"
#include "player.h"
#include "client_registry.h"
/*
 * Thread function for the thread that handles a particular client.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.
 *
 * This function executes a "service loop" that receives packets from
 * the client and dispatches to appropriate functions to carry out
 * the client's requests.  It also maintains information about whether
 * the client has logged in or not.  Until the client has logged in,
 * only LOGIN packets will be honored.  Once a client has logged in,
 * LOGIN packets will no longer be honored, but other packets will be.
 * The service loop ends when the network connection shuts down and
 * EOF is seen.  This could occur either as a result of the client
 * explicitly closing the connection, a timeout in the network causing
 * the connection to be closed, or the main thread of the server shutting
 * down the connection as part of graceful termination.
 */

int debug_show_maze = 0;

void *mzw_client_service(void *arg)
{

    int fd = *(int *)arg;
    debug("fd = %d",fd);
    free(arg);
    debug("thread is = %ld",pthread_self());
    Pthread_detach(pthread_self());
    debug("thread is = %ld",pthread_self());
    creg_register(client_registry,fd);
    //creg_fini(client_registry);
    MZW_PACKET *pkt = malloc(sizeof(MZW_PACKET));
    debug("size of ptr = %ld",sizeof(void *));
    void **data = malloc(sizeof(void *));
    int login_success = 0;
    PLAYER *player;

    while(1)
    {
        debug("before proto_recv_packet");
        int result =  proto_recv_packet(fd,pkt,data);
        debug("result after proto call = %d",result);
        if(result == 1)
        {
            int close_result = close(fd);
            if(close_result == -1)
                debug("closed returned -1");
            if(login_success == 1)
            {
                debug("log out called in server");
                player_logout(player);
                //free(player);
            }

            debug("before call to logout = %d",fd);
            //player_logout(player);
            debug("after call to logout");
            //free(*data);
            free(data);
            free(pkt);


            //remember to remove this
            //creg_fini(client_registry);
            creg_unregister(client_registry,fd);
            debug("Executed");
            //creg_fini(client_registry);
            break;
        }
        if(result == 3)
        {
            debug("intrrupted by signal@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            player_check_for_laser_hit(player);
            //player_reset(player);
            //player_invalidate_view(player);
            //player_update_view(player);
            //break;
            continue;
        }
        int pkt_type = pkt->type;
        switch(pkt_type)
        {
            case MZW_LOGIN_PKT:
                debug("login packet");
                if(login_success == 0)
                {
                    debug("name = %s",*(char**)data);
                    debug("param1 = %d data = %s",pkt->param1,*(char **)data);
                    //int name_size = strlen(*(char **)data) +1;
                    //char *name = malloc(name_size);
                    //strcpy(name,*(char**)data);
                    //debug("name_size = %d name = %s",name_size,name);
                    player = player_login(fd,pkt->param1,*data);
                    debug("%p",player);
                    if(player == NULL)
                    {
                        pkt->type = MZW_INUSE_PKT;
                        pkt->size = 0;
                        proto_send_packet(fd,pkt,data);
                        //pthread_exit(NULL);
                    }
                    else
                    {
                        login_success = 1;
                        pkt->type = MZW_READY_PKT;
                        pkt->size = 0;
                        proto_send_packet(fd,pkt,data);
                        player_reset(player);
                    }
                }
                free(*data);
                break;
            case MZW_MOVE_PKT:
                debug("move packet");
                if(login_success == 1)
                {
                    player_move(player,pkt->param1);
                }
                break;
            case MZW_TURN_PKT:
                debug("turn packet");
                if(login_success == 1)
                {
                    player_rotate(player,pkt->param1);
                }
                break;
            case MZW_FIRE_PKT:
                debug("fire packet");
                if(login_success == 1)
                {
                    player_fire_laser(player);
                }
                break;
            case MZW_REFRESH_PKT:
                debug("refresh packet");
                if(login_success == 1)
                {
                    player_invalidate_view(player);
                    player_update_view(player);
                }
                break;
            case MZW_SEND_PKT:
                debug("send packet");
                if(login_success == 1)
                {
                   // creg_fini(client_registry);
                    /*
                    char *str = "terminate";
                    if((strcmp(str,*(char **)data) == 0))
                    {
                        debug("terminate the server by client");
                        debug("get pid = %d",getpid());
                        kill(getpid(),SIGHUP);
                    }
                    */
                    player_send_chat(player,*(char **)data,strlen(*(char **)data));
                    free(*data);
                }
                break;
        }
/*
        if(debug_show_maze == 1)
        {
            show_maze();
        }
        */

    }

    return NULL;
}