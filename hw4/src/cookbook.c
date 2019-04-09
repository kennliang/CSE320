#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "cookbook.h"
#include "debug.h"
#include <sys/types.h>
#include<sys/wait.h>

#include <unistd.h>
#include <signal.h>

typedef struct state_info {
    int num_complete;
    int required_num;
    pid_t pid;
}STATE_INFO;

 RECIPE_LINK *work_queue = NULL;
 RECIPE_LINK *recipe_req = NULL;
 RECIPE_LINK *finished = NULL;
 int work_count = 0;
 int num_process = 0;


/*
void add_queue(RECIPE_LINK **header,RECIPE *recipe)
{
    //no item in queue
    RECIPE_LINK *head = *header;
    if(head == NULL)
    {
        //printf("executed");
        head = malloc(sizeof(RECIPE_LINK));
        //printf("header_ptr %p head malloc ptr %p\n",header,head);
        head->name = recipe->name;
        head->recipe = recipe;
        head->next = NULL;
        *header = head;
        return;
    }
    //1 item or more in queue
    RECIPE_LINK *temp = head;
    while(temp->next != NULL)
    {
        temp = temp->next;
    }
   // printf("111\n");
    RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
    new->name = recipe->name;
    new->recipe = recipe;
    new->next = NULL;
    temp->next = new;
}
*/

pid_t fork_check()
{
    pid_t pid;
    if((pid = fork()) < 0)
    {
        fprintf(stderr,"fork failed");
        exit(1);
    }
    return pid;
}

void traverse_recipe(RECIPE *root, RECIPE_LINK *visited)
{
    if(root == NULL)
        return;
    RECIPE_LINK *depend_list = root->this_depends_on;
/*
    RECIPE_LINK *temp = visited;
    while(temp != NULL)
    {
        printf("%s\n",temp->name);
        if(!strcmp(temp->name,root->name))
        {

            printf("cycle was found %s",root->name);
            exit(1);

        }
        temp = temp->next;
    }
    add_queue(&visited,root);
    printf("\n\n\n");
    */


    if(visited == NULL)
    {
        visited = malloc(sizeof(RECIPE_LINK));
        visited->name = root->name;
        visited->recipe = root;
        visited->next = NULL;
    }
    else
    {
        RECIPE_LINK *temp = visited;
        while(temp != NULL)
        {
           // debug("%s\n",temp->name);
            if(!strcmp(temp->name,root->name))
            {
                printf("cycle was found %s",root->name);
                exit(1);

            }
            temp = temp->next;
        }
       // printf("\n\n\n");
        RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
        new->name = root->name;
        new->recipe = root;
        new->next = visited;
        visited = new;
    }


    while(depend_list != NULL)
    {
        traverse_recipe(depend_list->recipe, visited);
        //debug("depend_list = %s",depend_list->name);
        depend_list = depend_list->next;
    }


    if(root->this_depends_on == NULL)
    {
        /*
        debug("exerefarecuted");

        RECIPE_LINK *temp = analysis;
        int found = 1;
        while(temp != NULL)
        {
            if(!strcmp(temp->name,root->name))
            {
                found = 0;
                break;
            }
            temp = temp->next;
        }
        if(found){
            debug("%p   %p",&analysis,root);
            add_queue(&analysis,root);
        }
        */

        if(work_queue == NULL)
        {

            work_queue = malloc(sizeof(RECIPE_LINK));
            //debug("%p",analysis);
            work_queue->name = root->name;
            work_queue->recipe = root;
            work_queue->next = NULL;
        }
        else
        {
           // debug("22222222222222222222222222222222");
            RECIPE_LINK *temp = work_queue;
            int found = 1;
            while(temp != NULL)
            {
                if(!strcmp(temp->name,root->name))
                {
                    found = 0;
                    break;
                }
                temp = temp->next;
            }
            if(found)
            {
                RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
                new->name = root->name;
                new->recipe = root;
                new->next = work_queue;
                work_queue = new;
            }

        }
        root->state = malloc(sizeof(STATE_INFO));
        ((STATE_INFO *)root->state)->pid = 0;

        //debug("leaf_recipe = %s address %p",root->name,root);
    }
    else
    {
        int num_depend = 0;
        RECIPE_LINK *temp_p = root->this_depends_on;
        while(temp_p != NULL)
        {
            num_depend++;
            temp_p = temp_p->next;
        }
        //debug("num_depend %d",num_depend);
        root->state = malloc(sizeof(STATE_INFO));
        STATE_INFO *info = root->state;
        info->num_complete = 0;
        info->pid = 0;
        info->required_num = num_depend;

        if(recipe_req == NULL)
        {

            recipe_req = malloc(sizeof(RECIPE_LINK));
            //debug("%p",analysis);
            recipe_req->name = root->name;
            recipe_req->recipe = root;
            recipe_req->next = NULL;
        }
        else
        {
           // debug("22222222222222222222222222222222");
            RECIPE_LINK *temp = recipe_req;
            int found = 1;
            while(temp != NULL)
            {
                if(!strcmp(temp->name,root->name))
                {
                    found = 0;
                    break;
                }
                temp = temp->next;
            }
            if(found)
            {
                RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
                new->name = root->name;
                new->recipe = root;
                new->next = recipe_req;
                recipe_req = new;
            }

        }

        //int required = 1;
       // debug("executed");
        //root->state = &required;
        //debug("recipe_list = %s = %d",root->name,*(int *)root->state);

        //debug("recipe_list = %s",root->name);
    }


}
void process_end()
{
    int status;
    pid_t pid;
    while( (pid = wait(&status)) > 0)
    {
        num_process--;
        debug("child terminated = %d",pid);
    }
    if(pid < 0)
        fprintf(stderr, "%s\n", "WAIT HAS FAILED RETURNED -1");



    //traverse through depend_list to see if any recipe

}




int process_recipe(COOKBOOK *cb,char *main_recipe_name,int max_cooks)
{

    RECIPE *main_recipe_ptr = cb->recipes;
    //search for recipe or use first recipe if no given recipe
    while(main_recipe_ptr != NULL && main_recipe_name != NULL)
    {
        //they are equal names
        if(!strcmp(main_recipe_name,main_recipe_ptr->name))
        {
           // debug("found recipe = %s",main_recipe_ptr->name);
            break;
        }
        main_recipe_ptr = main_recipe_ptr->next;
    }
    //did not find recipe name in cookbook
    if(main_recipe_ptr == NULL)
    {
        debug("DID NOT FIND RECIPE NAME IN COOKBOOK");
        return 1;
    }
    /*
    RECIPE_LINK *link = main_recipe_ptr->depend_on_this;
    while(link != NULL)
    {
        debug("depend_recipes = %s",link->name);
        link = link->next;
    }
    */

    //recursive traversal of cookbook (analysis)
    //left-right-root

    RECIPE_LINK *visited = NULL;

    traverse_recipe(main_recipe_ptr,visited);


    //main process loop
    int success = 1;
    num_process = 0;

    work_count = 0;

    RECIPE_LINK *temp = work_queue;
    while(temp != NULL)
    {
        work_count++;
        //debug("main_leaf_recipe = %s",temp->name);
        temp = temp->next;
    }
    temp = recipe_req;
    int required_recipe = 0;
    while(temp != NULL)
    {
        required_recipe++;
       // debug("required_recipes = %s required_num = %d ",temp->name,((STATE_INFO *)(temp->recipe->state))->required_num);
        temp = temp->next;
    }
   // debug("%d",work_count);

    //step up SEGCHILD handler with SIGACTION FUNCTION

    struct sigaction act;
    act.sa_handler = process_end;
   // debug("%d",SIGCHLD);

    sigaction(SIGCHLD,&act,NULL);


    //step up mask
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);

    sigset_t empty;
    sigemptyset(&empty);




    while(1)
    {
        //no recipes left to process
        if(work_count == 0 && num_process == 0)
        {
            while(finished != NULL)
            {
                pid_t te = ((STATE_INFO *)finished->recipe->state)->pid;
                debug("pid id = %ld",(long)te);
                finished = finished->next;
            }

            if(success)
                return 0;
            else
                return 1;
            debug("%d",success);
        }
        //cook should suspend --sigsuspend() waiting for the completion of one or more recipes
       sigprocmask(SIG_BLOCK,&mask,NULL);
        if(num_process == max_cooks)
        {

            //debug("sleeping for 2");

            //after sigsuspend search for recipe to be added to queue
          // sigsuspend(&mask);
           //ile(1)
            //debug("before suspend");
            // debug("num processes = %d work count = %d",num_process,work_count);
           // sigprocmask(SIG_UNBLOCK,&mask,NULL);
            sigsuspend(&empty);
            sigprocmask(SIG_UNBLOCK,&mask,NULL);
            debug("after suspend");


        }
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        //remove the first recipe from work_queue and start processing --fork()
        //sigprocmask(SIG_BLOCK,&mask,NULL);
        //debug("work_count = %d num_process = %d ",work_count,num_process);
        if(num_process < max_cooks && work_count != 0 )
        {

            //child process code
            sigprocmask(SIG_BLOCK,&mask,NULL);
            RECIPE_LINK *first_work = work_queue;
            RECIPE *removed = work_queue->recipe;

            work_queue = work_queue->next;

            if(finished == NULL)
            {
                finished = first_work;
                finished->next = NULL;
            }
            else
            {
                RECIPE_LINK *prev = finished;
                finished = first_work;
                finished->next = prev;
            }

            //debug("processing_recipe = %s",removed->name);
            work_count--;
           // debug("work_count = %d",work_count);
            num_process++;


            pid_t pid = fork_check();

             //child process code
            if( pid == 0)
            {

                debug("processing_recipe = %s",removed->name);
                sigprocmask(SIG_UNBLOCK,&mask,NULL);


                // for each task set up a process --fork() -running concurrently
                //d



                //



                return 0;
            }
            //fork returns pid of child to parent
            else
            {
                ((STATE_INFO *)removed->state)->pid = pid;
                sigprocmask(SIG_UNBLOCK,&mask,NULL);
            }
            //printf("executed");
            //parent process code
        }
    }


    return 0;
}


