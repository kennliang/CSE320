#include "maze.h"
#include "string.h"
#include <stdlib.h>
#include "debug.h"

unsigned int next = 1;

//random number between 0-range-1
int rand_func(int range)
{
  next = next*1103515245 + 12345;
  return (unsigned int)(next/65536) % range;
}

typedef struct maze {
    char **maze_template;
}MAZE;

MAZE *maze = NULL;
/*
 * Initialize the maze.
 *
 * @param template  The template for the maze.
 *
 * This must be called before any other operations are performed on the maze.
 * A maze template is a NULL-terminated array of strings, all of which are the same
 * length.  This defines a rectangular array of characters.  Characters represent
 * either player avatars, solid objects, or empty space, as described above.
 */
void maze_init(char **template)
{
    maze = malloc(sizeof(MAZE));

    maze->maze_template = malloc(250 * sizeof(char*));

    char **template_ref= template;
    char **maze_temp = maze->maze_template;
    int str_length = strlen(*template);
   // debug("str_length = %d",str_length);
    while(*template_ref != NULL)
    {
        //debug("%d",str_length);
        *maze_temp = malloc(str_length +1 *sizeof(char));
        strcpy(*maze_temp,*template_ref);
        debug("%s",*template_ref);
        maze_temp++;
        template_ref++;
    }
    *maze_temp = NULL;

    //maze->maze_template = copy;
    /*
    char **temp = maze->maze_template;
    debug("before print");
    while(*temp != NULL)
    {
        debug("%s",*temp);
        temp++;
    }
    */
}

/*
 * Finalize the maze.
 * This should be called when the maze is no longer required.
 */
void maze_fini()
{
    char **temp = maze->maze_template;
    while(*temp != NULL)
    {
        free(*temp);
        temp++;
    }
    free(temp);
    free(maze);
}

/*
 * Get the number of rows of the maze.
 *
 * @return the number of rows of the maze.
 */
int maze_get_rows()
{
    debug("Get rows called");
    int num_rows = 0;
    char **temp = maze->maze_template;
    while(*temp != NULL)
    {
        num_rows++;
        temp++;
    }
    debug("num rows = %d",num_rows);
    return num_rows;
}

/*
 * Get the number of columns of the maze.
 *
 * @return the number of columns of the maze.
 */
int maze_get_cols()
{
    debug("get cols called");
    int num_cols = 0;
    char **temp = maze->maze_template;
    num_cols = strlen(*temp);
    debug("num cols = %d",num_cols);
    return num_cols;
}

/*
 * Place a player's avatar in the maze at a specified row/column location.
 *
 * @param avatar  The avatar to be placed in the maze.
 * @param row  The row in which the avatar is to be placed.
 * @param col  The column in which the avatar is to be placed.
 * @return zero if the placement was successful, nonzero otherwise.
 *
 * Unsuccessful placement will occur if the specified location in the maze
 * is not empty.  If placement is successful, then the views of all
 * players will be updated.
 */
int maze_set_player(OBJECT avatar, int row, int col)
{
    debug("row = %d col = %d",row,col);
    char **temp = maze->maze_template;
    temp = temp + row -1;
    char *location = (*temp + col -1);
    if(IS_EMPTY(*location))
    {
        *location = avatar;
    }
    else
        return 1;
    return 0;
}

/*
 * Place a player's avatar in the maze at a random unoccupied location.
 *
 * @param avatar  The avatar to be placed in the maze.
 * @param row  Pointer to a variable into which will be stored the row
 * at which the avatar was placed.
 * @param col  Pointer to a variable into which will be stored the column
 * at which the avatar was placed.
 * @return zero if the placement was successful, nonzero otherwise.
 *
 * The placement can fail if after a large number of attempts an unoccupied
 * location has not been found.  If placement is successful, then the views
 * of all players will be updated.
 */
int maze_set_player_random(OBJECT avatar, int *rowp, int *colp)
{
    debug("maze_set_player_random called");
    int MAX_ATTEMPT = 100;
    int success = 0;
    int num_attempts = 0;
    while(success == 0 && num_attempts < MAX_ATTEMPT)
    {
        num_attempts++;
        int rand_row = rand_func(maze_get_rows());
        int rand_col = rand_func(maze_get_cols());
        debug("rand_row = %d rand_col = %d",rand_row,rand_col);
        char **temp = maze->maze_template;
        temp = temp + rand_row;
        debug("%s",*temp);
        char *location = *temp + rand_col;
      //  debug("%c",*location);
        if(IS_EMPTY(*location))
        {
            *location = avatar;
            /*
            int length = strlen(*temp);
            char *copy = malloc(length * sizeof(char));
            strcpy(copy,*temp);
            char *copy_loc = copy+ rand_col;
            *copy_loc = avatar;
            //free(*temp);
            *temp = copy;
            debug("%s",copy);
            *location = 'a';
            */

            *rowp = rand_row;
            *colp = rand_col;
            success = 1;
        }
    }
    if(success == 0)
        return 1;

    return 0;
}

/*
 * Remove a specified player's avatar from a specified location in the maze.
 *
 * @param avatar  The avatar to be removed from the maze.
 * @param row  The row from which the avatar is to be removed.
 * @param col  The column from which the avatar is to be removed.
 *
 * The views of all players are updated after the removal has been performed.
 */
void maze_remove_player(OBJECT avatar, int row, int col)
{
    debug("row = %d col = %d",row,col);
    debug("maze_remove_player called");
    char **temp = maze->maze_template;
    temp = temp + row;
    char *location = *temp + col;
    *location = EMPTY;
}

/*
 * Attempt to move a player's avatar at a specified location one unit
 * of distance in a specified direction.
 *
 * @param row  The row at which the avatar to be moved is located.
 * @param col  The column at which the avatar to be moved is located.
 * @param dir  The direction in which the avatar is to be moved.
 * @return zero if movement was successful, nonzero otherwise.
 *
 * Movement is not possible if it would cause the avatar to occupy
 * a location already occupied by some other object, or if it would
 * result in moving outside the bounds of the maze.
 * If movement is successful, then the views of all players are updated.
 */
int maze_move(int row, int col, int dir)
{

    //show_maze();
    //debug("maze_move called");
    debug("row = %d col = %d",row,col);

    //check for going out of bounds!!!!!!!!!!

    char **temp = maze->maze_template;
    temp = temp + row;
    char *current_location = *temp + col;
    char *location;
    if(IS_EMPTY(*current_location))
        debug("the avatar is not here");
    else
    {
        debug("temp = %s",*temp);
        debug("current loction = %s",current_location);
        debug("the avatar is here = %c",*current_location);
    }
    switch(dir)
    {
        case NORTH:
            debug("north");
            temp = temp -1;
            location = *temp + col;
            if(IS_EMPTY(*location))
            {

                debug("location empty = %c",*location);
                *location = *current_location;
                *current_location = EMPTY;
            }
            else
            {
                debug("location not empty");
                return 1;
            }
            break;
        case SOUTH:
            debug("south");
            temp = temp + 1;
            location = *temp + col;
            if(IS_EMPTY(*location))
            {
                debug("location empty = %c",*location);
                *location = *current_location;
                *current_location = EMPTY;
            }
            else
            {
                 debug("location not empty");
                return 1;
            }
            break;
        case EAST:
            debug("east");
            location = *temp + col +1;
            if(IS_EMPTY(*location))
            {
                  debug("location empty = %c",*location);
                *location = *current_location;
                *current_location = EMPTY;
            }
            else
            {
                 debug("location not empty");
                return 1;
            }
            break;
        case WEST:
            debug("west");
            location = *temp + col -1;
            if(IS_EMPTY(*location))
            {
                 debug("location empty = %c",*location);
                *location = *current_location;
                *current_location = EMPTY;
            }
            else
            {
                 debug("location not empty");
                return 1;
            }
            break;
    }

   // show_maze();
    return 0;
}

/*
 * Search from a specified target location in a specified direction,
 * and return the first avatar, if any, that is found.
 *
 * @param row  The starting row for the search.
 * @param col  The starting column for the search.
 * @param dir  The direction for the search.
 * @return the first avatar found, or EMPTY if the search terminated
 * without finding an avatar.
 *
 * The search terminates when a non-empty location is reached,
 * or the search would go beyond the maze boundaries.
 */
OBJECT maze_find_target(int row, int col, DIRECTION dir)
{
    debug("row = %d col = %d",row,col);
    debug("maze_find_target called");
    char **maze_ptr = maze->maze_template;
    char *location;

    switch(dir)
    {
        case NORTH:
            debug("north");
            maze_ptr = maze_ptr + row - 1;
            location = *maze_ptr+col;
            while(IS_EMPTY(*location))
            {
                maze_ptr--;
                location = *maze_ptr + col;
            }
            if(IS_AVATAR(*location))
                return *location;
            break;
        case SOUTH:
            debug("south");
            maze_ptr = maze_ptr + row + 1;
            location = *maze_ptr+col;
            while(IS_EMPTY(*location))
            {
                maze_ptr++;
                location = *maze_ptr + col;
            }
            if(IS_AVATAR(*location))
                return *location;
            break;
        case WEST:
            debug("west");
            maze_ptr = maze_ptr + row;
            location = *maze_ptr + col - 1;
            while(IS_EMPTY(*location))
            {
                location--;
            }
            if(IS_AVATAR(*location))
                return *location;
            break;
        case EAST:
            debug("east");
            maze_ptr = maze_ptr + row;
            location = *maze_ptr + col + 1;
            while(IS_EMPTY(*location))
            {
                location++;
            }
            if(IS_AVATAR(*location))
                return *location;
            break;
    }
    return EMPTY;
}

/*
 * Get the view from a specified location in the maze, with the gaze
 * in a specified direction.
 *
 * @param view  A pointer to a view of maximum depth that is to be filled
 * in as a result of the call.
 * @param row  Row of the maze that contains the view origin.
 * @param col  Column of the maze that contains the view origin.
 * @param gaze  Direction of gaze for the view.
 * @param depth  Maximum depth of the view.  This must be less than or
 * equal to the depth of the view that is passed.
 * @return the depth to which the view was filled in.
 *
 * The view array is populated with a "patch" of the maze contents,
 * as described above.  The returned value could be less than the
 * maximum depth, as described above.  Entries of the view at depths
 * greater than the returned depth should be regarded as invalid.
 */
int maze_get_view(VIEW *view, int row, int col, DIRECTION gaze, int depth)
{
    debug("row = %d col = %d depth = %d",row,col,depth);
    debug("maze_get_view called");
    VIEW *view_temp = view;

    char **maze_ptr = maze -> maze_template;
    maze_ptr = maze_ptr + row;
    char *location = *maze_ptr + col;
    if(IS_AVATAR(*location))
    {
        debug("avatar located here =%c",*location);
    }
    int actual_depth  = 1;
   // char *temp;
    char **maze_prev = maze_ptr -1;
    char **maze_next = maze_ptr +1;
    int index = 0;

    switch(gaze)
    {
        case NORTH:
            debug("north");
            maze_ptr = maze_ptr -1;
            location = *maze_ptr + col;
            while(!IS_WALL(*location))
            {
                actual_depth++;
                maze_ptr = maze_ptr -1;
                location = *maze_ptr + col;
            }
            actual_depth++;
            if(actual_depth > depth)
                actual_depth = depth;
            maze_ptr = maze->maze_template;
            maze_ptr = maze_ptr + row;
            for(int i = 0; i < actual_depth;i++)
            {
                *((**view)+index) = *(*maze_ptr + col - 1);
                index++;
                *((**view)+index) = *(*maze_ptr + col);
                index++;
                *((**view)+index) = *(*maze_ptr + col + 1);
                index++;
                maze_ptr = maze_ptr -1;
            }
            *((**view)+3 *actual_depth) = '\0';
            break;
        case SOUTH:
            debug("south");
            maze_ptr = maze_ptr +1;
            location = *maze_ptr + col;
            while(!IS_WALL(*location))
            {
                actual_depth++;
                maze_ptr = maze_ptr +1;
                location = *maze_ptr + col;
            }
            actual_depth++;
            if(actual_depth > depth)
                actual_depth = depth;
            maze_ptr = maze->maze_template;
            maze_ptr = maze_ptr + row;
            for(int i = 0; i < actual_depth;i++)
            {
                *((**view)+index) = *(*maze_ptr + col + 1);
                index++;
                *((**view)+index) = *(*maze_ptr + col);
                index++;
                *((**view)+index) = *(*maze_ptr + col - 1);
                index++;
                maze_ptr = maze_ptr +1;
            }
            *((**view)+3 *actual_depth) = '\0';
            break;
        case WEST:
            debug("west");
            location = *maze_ptr + col -1;
            while(!IS_WALL(*location))
            {
                actual_depth++;
                location = location -1;
            }
            actual_depth++;
            if(actual_depth > depth)
                actual_depth = depth;
            //char **maze_prev = maze_ptr -1;
            //char **maze_next = maze_ptr +1;
            //int index = 0;
            for(int i = 0 ; i <  actual_depth ; i++)
            {
                *((**view)+index) = *(*maze_next + col - i);
                index++;
                *((**view)+index) = *(*maze_ptr + col - i);
                index++;
                *((**view)+index) = *(*maze_prev + col - i);
                index++;
            }
             *((**view)+3 *actual_depth) = '\0';
            break;
        case EAST:
            debug("east");
            location = *maze_ptr + col +1;
            while(!IS_WALL(*location))
            {
                //debug("executed");
                actual_depth++;
                location = location +1;
            }
            actual_depth++;
            if(actual_depth > depth)
                actual_depth = depth;
            //char **maze_prev = maze_ptr -1;
            //char **maze_next = maze_ptr +1;
            //int index = 0;
            for(int i = 0 ; i <  actual_depth ; i++)
            {
                *((**view)+index) = *(*maze_prev + col + i);
                index++;
                *((**view)+index) = *(*maze_ptr + col + i);
                index++;
                *((**view)+index) = *(*maze_next + col + i);
                index++;
            }
             *((**view)+3 *actual_depth) = '\0';
            break;
    }
    debug("actual_depth = %d",actual_depth);
    debug("view_string =%s",**view_temp);

    return actual_depth;
}

void show_view(VIEW *view, int depth)
{
    debug("maze_show_view called");
}

void show_maze()
{
    char **display = maze->maze_template;
    while(*display != NULL)
    {
        fprintf(stderr, "%s\n", *display);
        display++;
    }
}