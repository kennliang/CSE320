#include "player.h"
#include "maze.h"
#include "csapp.h"
#include "debug.h"
/*
 * The player module mantains a mapping from avatars to player state for
 * the players currently logged in to the game.
 */
typedef struct player_map{
    OBJECT avatar;
    PLAYER *player;
    struct player_map *next;
    sem_t mutex;
}MAP;

/*
 * Structure that defines the state of a player in the Maze War game.
 *
 * You will have to give a complete structure definition in player.c.
 * The precise contents are up to you.  Be sure that all the operations
 * that might be called concurrently are thread-safe.  Note that, due to
 * the pattern of calls made between the maze module and the player module,
 * it is necessary that any mutex used to protect a PLAYER object
 * have the PTHREAD_MUTEX_RECURSIVE attribute set.  This makes the mutex
 * recursive, which means that it can be locked again by a thread that
 * already holds the lock, without a deadlock occurring.  The reason
 * PLAYER objects have to have recursive mutexes is because functions on
 * PLAYER objects may result in calls on the maze, and these in turn may
 * result in an "up call" to player_update_view() to update the player's view.
 * Unless a recursive mutex is used, such an up call will deadlock when
 * the attempt is made to reaquire the PLAYER mutex that is already held.
 */
typedef struct player{
    char *name;
    OBJECT avatar;
    int fd;
    int row;
    int col;
    DIRECTION dir;
    int score;
    int reset_view;
    int hit;
    int reference;
    int valid_view;
    char view[16][3];
    pthread_t thread_id;
    sem_t mutex;
} PLAYER;

MAP *map = NULL;

void sigusr1_handler()
{
    debug("signal handler executed");
}

/*
 * Initialize the players module.
 * This must be called before doing calling any other functions in this
 * module.
 */
void player_init(void)
{
    map = calloc(1,sizeof(MAP));
    map->player = NULL;
    map->next = NULL;
    Sem_init(&map->mutex,0,1);

    struct sigaction act;
    memset(&act,0,sizeof(struct sigaction));
    act.sa_handler = sigusr1_handler;
    int sig_result = sigaction(SIGUSR1,&act,NULL);
    if(sig_result == -1)
        debug("error calling sigaction");
}

/*
 * Finalize the players module.
 * This should be called when the players module is no longer required.
 */
void player_fini(void)
{
    MAP *temp2 = map;
    int count = 0;
    while(temp2 != NULL )
    {
        count++;
        temp2 = temp2->next;
    }
    debug("num of things in map %d",count);

    MAP *temp = map;
    while(temp != NULL)
    {
       // if(temp->player != NULL)
        //{
            //debug("player not freed");
            //debug("player ref ctn = %d",temp->player->reference);
            //free(temp->player);
        //}
        debug("free map in player");
        MAP *current = temp;
        temp = temp->next;
        free(current);
    }
}

/*
 * Attempt to log in a player with a specified avatar.
 *
 * @param clientfd  The file descriptor of the connection to the client.
 * @param avatar  The avatar desired for the player.
 * @param name  The player's name, which is copied before being saved.
 * @return A pointer to a PLAYER object, in case of success, otherwise NULL.
 *
 * The login can fail if the specified avatar is already in use.
 * If the login succeeds then a mapping is recorded from the specified avatar
 * to a PLAYER object that is created for this client and returned as the
 * result of the call.  The returned PLAYER object has a reference count equal
 * to one.  This reference should be "owned" by the thread that is servicing
 * this client, and it should not be released until the client has logged out.
 */
PLAYER *player_login(int clientfd, OBJECT avatar, char *name)
{
    PLAYER *find;
    if((find = player_get(avatar)) == NULL)
    {
        find = calloc(1,sizeof(PLAYER));
        find->fd = clientfd;
        find->score = 0;
        find->dir = EAST;
        find->row = 0;
        find->col = 0;
        find->hit = 0;
        find->valid_view = 0;
        find->reset_view = 0;
        find->avatar = avatar;
        find->reference = 0;
        Sem_init(&find->mutex,0,1);
        find->thread_id = pthread_self();
        player_ref(find,"Newly created player");
        find->name = malloc(strlen(name) + 1 * sizeof(char));
        strcpy(find->name,name);

        int x;
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex login= %d",x);
        P(&map->mutex);

        if(map->player != NULL)
        {
            MAP *temp = map;
            MAP *new = calloc(1,sizeof(MAP));
            new->player = find;
            new->avatar = avatar;
            MAP *prev;
            while(temp != NULL)
            {
                prev = temp;
                temp = temp->next;
            }
            prev->next = new;
            //new->next = map;
            //map = new;
        }
        else
        {
            map->avatar = avatar;
            map->player = find;
        }

        V(&map->mutex);
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex login= %d",x);
/*

        MAP *temp = map;
        int count = 0;
        while(temp != NULL && temp->player != NULL)
        {
            count++;
            temp = temp->next;
        }
        debug("num of things in map %d",count);
        */
    }
    else
    {
        player_unref(find,"another player with same avatar found");
        return NULL;
    }
    return find;
}

/*
 * Log out a player.
 *
 * @param player  The player to be logged out.
 *
 * The specified player is removed from the maze and from the players map
 * and a SCORE packet with a score of -1 is sent to the client to cause the
 * player's score to be removed from the scoreboard area.
 * This function "consumes" one reference to the PLAYER object by calling
 * player_unref().  This will have the effect of causing the PLAYER object
 * to be freed as soon as any references to it currently held by other threads
 * have been released.
 */
void player_logout(PLAYER *player)
{
    debug("player logout");
    maze_remove_player(player->avatar,player->row,player->col);
    int x;
     sem_getvalue(&map->mutex,&x);
    debug("value of mutex logout= %d",x);

    P(&map->mutex);
    MAP *temp = map;
    MAP *prev;
    while(temp != NULL && temp->player != NULL)
    {
        if(temp->player == player)
        {
            //first player in map
            if(temp == map)
            {
                if(temp ->next != NULL)
                {
                    map = temp->next;
                    map->mutex = temp->mutex;
                    free(temp);
                }
                map->player = NULL;
               // free(temp);
                //break;
            }
            else if(temp->next == NULL)
            {
                prev->next = NULL;
                free(temp);
            }
            else
            {
                prev->next = temp->next;
                free(temp);
            }
            //free(temp);
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    V(&map->mutex);
     sem_getvalue(&map->mutex,&x);
    debug("value of mutex logout = %d",x);
    player_unref(player,"log out player");
}

/*
 * Reset a player to a random location in the maze.
 *
 * @param player  The player to be reset.
 *
 * The player's avatar is removed from its existing location and re-placed
 * at a randomly selected location in the maze.  If the re-placement operation
 * fails, the client connection is ungracefully shut down in order to force
 * termination of service.  If the re-placement operation succeeds, then
 * the player's scoreboard is refreshed by sending all other player's scores,
 * and all other players' scoreboards are updated with this player's score.
 */
void player_reset(PLAYER *player)
{
    //newly created player has initial location 0,0
    if(!player->row == 0 && !player->col == 0)
    {
        //
        maze_remove_player(player->avatar,player->row,player->col);
    }

    debug("reset new player executed");
    if((maze_set_player_random(player->avatar,&player->row,&player->col)))
    {
        shutdown(player->fd,SHUT_RD);
    }
    else
    {
        debug("In reset = player row = %d player col = %d",player->row,player->col);
        //player->score = 0;
        player->dir = EAST;
        player->hit = 0;
        //player->reset_view = 1;
        player_invalidate_view(player);
        //traverse through map and send packet to update score to every player
        int x;
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex reset = %d",x);
        P(&map->mutex);
        MAP *temp = map;
        while(temp != NULL && temp->player != NULL)
        {
            debug("executed");
            MZW_PACKET *pkt = calloc(1,sizeof(MZW_PACKET));
            pkt->type = MZW_SCORE_PKT;
            pkt->param1 = player->avatar;
            pkt->param2 = player->score;
            pkt->size = strlen(player->name);
            player_send_packet(temp->player,pkt,player->name);
            pkt->param1 = temp->avatar;
            pkt->param2 = temp->player->score;
            pkt->size = strlen(temp->player->name);
            player_send_packet(player,pkt,temp->player->name);
            player_update_view(temp->player);
            temp = temp->next;
            free(pkt);
        }
        V(&map->mutex);
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex reset = %d",x);

    }
    //player_update_view(player);

}

/*
 * Get the state object for the player with a specified avatar.
 *
 * @param avatar  The avatar of the player whose state is to be retrieved.
 * @return the PLAYER object containing the state of the player with the
 * specified avatar, if this avatar is currently in use, otherwise NULL
 * is returned.
 *
 * The reference count on any PLAYER object that is returned is incremented,
 * to account for the additional pointer to that object which has been
 * created.  It is the caller's responsibility to call player_unref() to
 * release this reference count when the pointer is no longer required.
 */
PLAYER *player_get(unsigned char avatar)
{
    PLAYER *player = NULL;
    int x;
     sem_getvalue(&map->mutex,&x);
    debug("value of mutex getbefore = %d",x);
    P(&map->mutex);
    MAP *temp = map;
    while(temp != NULL && temp->player != NULL)
    {
        if(temp->avatar == avatar)
        {
            player = temp->player;
            player_ref(player,"player_get called");
            V(&map->mutex);
            return player;
        }
        temp = temp->next;
    }
    V(&map->mutex);
     sem_getvalue(&map->mutex,&x);
    debug("value of mutex get after= %d",x);
    return NULL;
}

/*
 * Increase the reference count on a player by one.
 *
 * @param player  The PLAYER whose reference count is to be increased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 * @return  The PLAYER object that was passed as a parameter.
 */
PLAYER *player_ref(PLAYER *player, char *why)
{
    debug("player ref called = %s",why);
    player->reference = player->reference + 1;
    return player;
}

/*
 * Decrease the reference count on a player by one.
 *
 * @param player  The PLAYER whose reference count is to be decreased.
 * @param why  A string describing the reason why the reference count is
 * being increased.  This is used for debugging printout, to help trace
 * the reference counting.
 *
 * If after decrementing, the reference count has reached zero, then the
 * player and its contents are freed.
 */
void player_unref(PLAYER *player, char *why)
{
    debug("player unref called = %s",why);
    player->reference = player->reference -1;
    if(player->reference == 0)
    {
        free(player->name);
        free(player);
    }
}

/*
 * Send a packet to the client for a player.
 *
 * @param player  The PLAYER object corresponding to the client who should
 * receive the packet.
 * @param pkt  The packet to be sent.
 * @param data  Data payload to be sent, or NULL if none.
 *
 * Once a client has connected and successfully logged in, this function
 * should be used to send packets to the client, as opposed to the lower-level
 * proto_send_packet() function.  The reason for this is that the present
 * function will lock the player mutex before calling proto_send_packet().
 * The fact that the mutex is locked before sending means that multiple
 * threads can safely call this function to send to the client, and these
 * calls will be serialized by the mutex.  Note that when a change to the
 * state of the maze results from the actions of one player, then that
 * then that player's thread will attempt to update the views of all players.
 * Since this can happen concurrently, we need to synchronize access to
 * the network connection to that client.
 *
 * NOTE: This function will lock the mutex associated with the PLAYER
 * object passed, for the duration of the call.  Since PLAYER mutexes are
 * recursive, it is OK for a thread to call this function while holding
 * a lock on the same PLAYER object.  However, no other locks should be
 * held at the time of call, otherwise there is a risk of deadlock.
 */
int player_send_packet(PLAYER *player, MZW_PACKET *pkt, void *data)
{
    proto_send_packet(player->fd,pkt,data);
    return 0;
}

/*
 * Get the current maze location and gaze direction for a player.
 *
 * @param player  The player whose information is to be accessed.
 * @param rowp  Pointer to a variable to receive the row index.
 * @param colp  Pointer to a variable to receive the column index.
 * @param dirp  Pointer to a variable to receive the gaze direction.
 * @return 0 if the player currently has a valid location in the maze,
 * otherwise nonzero.
 */
int player_get_location(PLAYER *player, int *rowp, int *colp, int *dirp)
{

    *rowp = player->row;
    *colp = player->col;
    *dirp = player->dir;
    if(*rowp <= 0 && *rowp >= maze_get_rows())
        return 1;
    if(*colp <= 0 || *colp >= maze_get_cols())
        return 1;
    return 0;


    /*
    *rowp = 0;
    *colp = 0;
    int found = 0;
    char **temp = maze->maze_template;
    while(*temp != NULL)
    {
        char *s = temp;
        while(s != '\0')
        {
            if(*s == player->avatar)
            {
                found = 1;
                break;
            }
            s++;
            colp++;

        }
        if(found)
            break;
        temp = temp->next;
        *rowp = *rowp + 1;
    }
    debug("row = %d col = %d",*rowp,*colp);
    if(found)
        return 0;
    return 1;
    */
}

/*
 * Attempt to move the player's avatar in the maze one unit of distance
 * forward or back with respect to the current direction of gaze.
 *
 * @param player  The player whose avatar is to be moved.
 * @param sign  1 for "forward" motion, which is in the direction of gaze,
 * and -1 for "backward" motion in the direction opposite to the gaze.
 * @return zero if the movement succeeds, nonzero otherwise.
 *
 * This function finds the current location of the player and calculates
 * the direction of motion, then calls maze_move() to actually carry out
 * the motion.  Since maze_move() will update player views if it succeeds,
 * it is necessary that the player state be updated to reflect the result
 * of the motion before maze_move() is called.  However, should maze_move()
 * fail, then the player state has to be reverted to reflect the old
 * location, rather than the new one.  Motion can fail if it would cause
 * the player's avatar to leave the bounds of the maze, or to occupy a
 * location that is already occupied by another object.  In that case,
 * an ALERT packet is sent to the client so that the user interface can
 * provide an indication that motion was blocked.
 */
int player_move(PLAYER *player, int dir)
{

    debug("player moved called = %d",dir);
    int real_dir;
    if(dir == -1)
    {
        real_dir = REVERSE(player->dir);
    }
    else
        real_dir = player->dir;
    //int current_row = player->row;

    //int current_col = player->col;
    debug("player row = %d player col = %d",player->row,player->col);
    int result = maze_move(player->row,player->col,real_dir);
    debug("result of move = %d",result);
    if(result == 0)
    {
        switch(real_dir)
        {
            case NORTH:
                player->row = player->row -1;
                break;
            case SOUTH:
                player->row = player->row +1;
                break;
            case EAST:
                player->col = player->col +1;
                break;
            case WEST:
                player->col = player->col -1;
                break;
        }
        int x;
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex move = %d",x);
        P(&map->mutex);
        MAP *temp = map;
        while(temp != NULL && temp->player != NULL)
        {
            //player->reset_view = 0;
            player_update_view(player);
            temp = temp->next;
        }
        V(&map->mutex);
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex move = %d",x);
    }
    else
    {
        return 1;
    }

    return 0;
}

/*
 * Rotate the player's gaze 90 degrees in the specified direction.
 *
 * @param player  The player whose direction of gaze is to be rotated.
 * @param dir  The direction of rotation: 1 means counter-clockwise
 * and -1 means clockwise.
 *
 * As a rotation of the gaze is likely to cause a complete change in
 * the view, this function invalidates the current player view in order
 * to force a full update.  Note that since the maze is not involved
 * in this operation, it is necessary to call player_update_view()
 * to cause the player's view to be updated after the rotation.
 */
void player_rotate(PLAYER *player, int dir)
{
    if(dir == 1)
        player->dir = TURN_LEFT(player->dir);
    else
        player->dir = TURN_RIGHT(player->dir);
    player_invalidate_view(player);
    player_update_view(player);
}

/*
 * Fire the player's laser in the direction of gaze.
 *
 * @param player  The player whose laser is to be fired.
 *
 * The laser will score a hit on the first avatar, if any, encountered
 * along the corridor.  If a hit is scored, the thread serving the player
 * who has been hit will be notified by sending a SIGUSR1 signal to the
 * thread serving that player.  In addition the score of the player who
 * fired the laser will be incremented by one and all clients will be
 * notified of the new score.
 */
void player_fire_laser(PLAYER *player)
{
    OBJECT player_hit = maze_find_target(player->row,player->col,player->dir);
    if(player_hit != EMPTY)
    {
        player->score = player->score + 1;
        PLAYER *player_hitted = player_get(player_hit);
        debug("thread id = %ld",player_hitted->thread_id);
        player_hitted->hit = 1;
        int x;
        sem_getvalue(&map->mutex,&x);
        debug("value of mutex fire = %d",x);
        debug("before");
        P(&map->mutex);
        debug("after");
        MAP *temp = map;
        while(temp != NULL && temp->player != NULL)
        {
            MZW_PACKET *pkt = calloc(1,sizeof(MZW_PACKET));
            pkt->type = MZW_SCORE_PKT;
            pkt->param1 = player->avatar;
            pkt->param2 = player->score;
            pkt->size = strlen(player->name);
            //->param3 = player->name;
            player_send_packet(temp->player,pkt,player->name);

            //player_send_packet();
            temp = temp->next;
            free(pkt);
        }
        V(&map->mutex);
         sem_getvalue(&map->mutex,&x);
    debug("value of mutex fire= %d",x);
         pthread_kill(player_hitted->thread_id,SIGUSR1);
        player_unref(player_hitted,"done with player who was hit by laser");

    }
}

/*
 * Invalidate the current view of an player.
 *
 * @param the player whose view is to be invalidated.
 *
 * In order to avoid having to transmit the full view seen by a player
 * each time a small change occurs (such as the appearance of another player's
 * avatar in an otherwise unchanged scene), the view used for a preceding
 * update is saved to permit the next update to be performed using only
 * differences between the two views, rather than the complete view.
 * However, some operations are likely to or certain to completely change
 * a player's view.  In such cases, we want to invalidate any preceding
 * view in order to force the next update to be a full one, rather than
 * a differential one.  The purpose of this function is to perform such
 * invalidation.
 */
void player_invalidate_view(PLAYER *player)
{
    player->reset_view = 1;
}

/*
 * Update a player's view to correspond to the current maze contents and
 * direction of gaze.
 *
 * @param player  The player whose view is to be updated.
 *
 * This function will query the maze for the view to be shown to the player,
 * and then it will perform either an incremental or a full view update,
 * depending on whether there is a valid previous view for the player.
 * To avoid deadlock between multiple threads calling this function
 * concurrently, the player mutex should be locked and held while querying
 * the maze for the view to be shown.  Once the view to be shown has
 * been obtained, then the update is performed by sending a series of
 * packets to the client.  A full update is performed by sending a CLEAR
 * packet, followed by a SHOW packet for every cell in the view.
 * An incremental update is performed by just sending SHOW packets for
 * those cells in the view that have changed since the previous update.
 * Note that in an incremental update care must be taken if the depths of
 * the old and new views are different.
 */
void player_update_view(PLAYER *player)
{
    debug("In update view :player row =%d player col = %d",player->row,player->col);
    char x[16][3];
    int depth = maze_get_view(&x,player->row,player->col,player->dir,16);

    if(player->reset_view == 1 || player->valid_view == 0)
    {
        MZW_PACKET *clear = calloc(1,sizeof(MZW_PACKET));
        clear->type = MZW_CLEAR_PKT;
        clear->size = 0;
        player_send_packet(player,clear,NULL);
        free(clear);
    }
    for(int i = 0 ; i < depth; i++)
    {
        for(int j = 0 ; j < 3; j++)
        {
            if(player->reset_view == 0 && player->valid_view == 1)
            {
                if(x[i][j] == (player->view)[i][j])
                {
                    debug("equal@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
                    continue;
                }
            }
            MZW_PACKET *new = calloc(1,sizeof(MZW_PACKET));
            new->type = MZW_SHOW_PKT;
            new->param1 = x[i][j];
            new->param2 = j;
            new->param3 = i;
            new->size = 0;
            player_send_packet(player,new,NULL);
            free(new);
        }
    }
    player->valid_view = 1;
    player->reset_view = 0;
    memcpy(player->view,x,sizeof(char) *depth * 3);

}
/*
 * Check whether a player has received any laser hits and, if so,
 * respond appropriately.
 *
 * @param player  The player to be checked for laser hits.
 *
 * The server thread handling a particular player will be notified about
 * a laser hit by receiving a SIGUSR1 signal.  A handler for this signal
 * should be installed in order to record the occurrence of the laser hit
 * in the state for the player who took the hit.  The present function
 * should be called just before each attempt is made to read the next
 * packet from the client connection.  This will ensure that any hits
 * that have occurred are noticed by the player that took the hits
 * in a prompt fashion.
 */
void player_check_for_laser_hit(PLAYER *player)
{
    debug("player check for laser hit executed");
    if(player->hit == 1)
    {
        debug("player was hit");
        maze_remove_player(player->avatar,player->row,player->col);
        debug("executed");
        int x;
         sem_getvalue(&map->mutex,&x);
        debug("value of mutex check = %d",x);
        P(&map->mutex);
        debug("executed");
        MAP *temp = map;
        while(temp != NULL && temp->player != NULL)
        {
            player_update_view(temp->player);
            temp = temp->next;
        }
        V(&map->mutex);

        sem_getvalue(&map->mutex,&x);
        debug("value of mutex check = %d",x);
         MZW_PACKET *new = calloc(1,sizeof(MZW_PACKET));
        new->type = MZW_ALERT_PKT;
        new->size = 0;
        player_send_packet(player,new,NULL);
        free(new);
        sleep(3);
        player_reset(player);
    }
    else
        debug("player was not hit");
}
/*
 * Broadcast a chat message to all players.
 *
 * @param player  The player sending the chat message.
 * @param msg  The message to be sent (not null-terminated).
 * @param len  The length of the message.
 */
void player_send_chat(PLAYER *player, char *msg, size_t len)
{

    char *combine = malloc((strlen(player->name) + len + 5 )* sizeof(char));
    strcpy(combine,player->name);
    char *build = malloc(5 *sizeof(char));
    *build = '[';
    *(build+1) = player->avatar;
    *(build+2) = ']';
    *(build+3) = ' ';
    *(build+4) = '\0';


    strcat(combine,build);
    strcat(combine,msg);

    debug("combined string = %s",combine);
    debug("str length = %ld",strlen(combine));
    char *temp2 = combine;
    temp2 = temp2+strlen(combine);
    debug("char here is %c",*temp2);

    int x;
    sem_getvalue(&map->mutex,&x);
    debug("value of mutex send= %d",x);
    P(&map->mutex);
    MAP *temp = map;
    while(temp != NULL && temp->player != NULL)
    {
        MZW_PACKET *new = calloc(1,sizeof(MZW_PACKET));
        new->type = MZW_CHAT_PKT;
        new->size =  strlen(combine);
        player_send_packet(temp->player,new,combine);
        temp = temp->next;
        free(new);
    }
    V(&map->mutex);
    sem_getvalue(&map->mutex,&x);
    debug("value of mutex send= %d",x);
    free(combine);
    free(build);
}
