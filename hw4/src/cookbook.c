#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "cookbook.h"
#include "debug.h"
#include <sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>

#include <unistd.h>
#include <signal.h>

typedef struct state_info {
    int num_complete;
    int required_num;
    pid_t pid;
}STATE_INFO;

 RECIPE_LINK *work_queue = NULL;
// RECIPE_LINK *recipe_req = NULL;
 RECIPE_LINK *finished = NULL;

 int work_count = 0;
 int num_process = 0;


int add_queue(RECIPE_LINK **header,RECIPE *recipe)
{
    //no item in queue
    RECIPE_LINK *head = *header;
    if(head == NULL)
    {
        head = malloc(sizeof(RECIPE_LINK));
        if(head == NULL)
            return 1;
        head->name = recipe->name;
        head->recipe = recipe;
        head->next = NULL;
        *header = head;
        return 0;
    }
    //1 item or more in queue
    RECIPE_LINK *temp = head;
    while(temp->next != NULL)
    {
        temp = temp->next;
    }
    RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
    if(new == NULL)
        return 1;
    new->name = recipe->name;
    new->recipe = recipe;
    new->next = NULL;

    temp->next = new;
    return 0;
}


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
    if(root == NULL){
        return;
    }
    RECIPE_LINK *depend_list = root->this_depends_on;

    if(visited == NULL)
    {
        visited = malloc(sizeof(RECIPE_LINK));
        if(visited == NULL)
        {
            fprintf(stderr, "%s\n","malloc failed" );
            exit(1);
        }
        visited->name = root->name;
        visited->recipe = root;
        visited->next = NULL;
    }
    else
    {
        RECIPE_LINK *temp = visited;
        while(temp != NULL)
        {
           //debug("%s",temp->name);
            if(!strcmp(temp->name,root->name))
            {
                fprintf(stderr, "%s\n","A cycle was found" );
                exit(1);
            }
            temp = temp->next;
        }
       // printf("\n\n\n");
        RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
        if(new == NULL)
        {
            fprintf(stderr, "%s\n","malloc failed" );
            exit(1);
        }
        new->name = root->name;
        new->recipe = root;
        new->next = visited;
        visited = new;
    }


    while(depend_list != NULL)
    {
        traverse_recipe(depend_list->recipe, visited);
        depend_list = depend_list->next;
    }

    free(visited);

    if(root->this_depends_on == NULL)
    {
        if(work_queue == NULL)
        {

            work_queue = malloc(sizeof(RECIPE_LINK));
            if(work_queue == NULL)
            {
                fprintf(stderr, "%s\n", "malloc failed");
                exit(1);
            }
            work_queue->name = root->name;
            work_queue->recipe = root;
            work_queue->next = NULL;
        }
        else
        {
            RECIPE_LINK *temp = work_queue;
            int found = 1;
            while(temp != NULL)
            {
                if(strcmp(temp->name,root->name) == 0)
                {
                    found = 0;
                    break;
                }
                temp = temp->next;
            }
            if(found)
            {
                RECIPE_LINK *new = malloc(sizeof(RECIPE_LINK));
                if(new == NULL)
                {
                    fprintf(stderr, "%s\n", "malloc failed");
                    exit(1);
                }
                new->name = root->name;
                new->recipe = root;
                new->next = work_queue;
                work_queue = new;
            }

        }
       // debug("size of state info %ld",sizeof(STATE_INFO));
        if(root->state == NULL)
        {
            root->state = malloc(sizeof(STATE_INFO));
            if(root->state == NULL)
            {
                fprintf(stderr, "%s\n","malloc failed" );
                exit(1);
            }
            ((STATE_INFO *)root->state)->pid = 0;
        }

       // debug("leaf_recipe = %s address %p",root->name,root);
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
        if(root->state == NULL)
        {
            root->state = malloc(sizeof(STATE_INFO));
            if(root->state == NULL)
            {
                fprintf(stderr, "%s\n","malloc failed" );
                exit(1);
            }
            STATE_INFO *info = root->state;
            info->num_complete = 0;
            info->pid = 0;
            info->required_num = num_depend;
        }
/*
        if(recipe_req == NULL)
        {

            recipe_req = malloc(sizeof(RECIPE_LINK));
            if(recipe_req == NULL)
            {
                fprintf(stderr, "%s\n","malloc failed" );
                exit(1);
            }
            recipe_req->name = root->name;
            recipe_req->recipe = root;
            recipe_req->next = NULL;
        }
        else
        {
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
                if(new == NULL)
                {
                    fprintf(stderr, "%s\n","malloc failed" );
                    exit(1);
                }
                new->name = root->name;
                new->recipe = root;
                new->next = recipe_req;
                recipe_req = new;
            }

        }*/
    }
}


void process_end()
{
    debug("handler called");
    int status = 0;
    pid_t pid;

    while( (pid = wait(&status)) > 0)
    {
        if(WEXITSTATUS(status))
        {
            fprintf(stderr, "%s\n","CHILD PROCESS DID NOT NORMALLY TERMINATED" );
            exit(1);
        }

        num_process--;

        RECIPE_LINK *temp = finished;
        //traverse through finished recipes until the recipe corresponding to the pid is found
        while(temp != NULL)
        {
            //debug("finished recipe = %s",temp->name);
            if( ((STATE_INFO *)temp->recipe->state)->pid == pid )
            {

                //traverse through all recipes that depend on this recipe
                RECIPE_LINK *depend = temp->recipe->depend_on_this;
                while(depend != NULL)
                {
                    STATE_INFO *state = ((STATE_INFO *)depend->recipe->state);
                    if(state != NULL)
                    {
                         //increment the number of completed recipes by 1 for recipes dependent on this recipe
                        state->num_complete = state->num_complete + 1;
                        //check for each dependent recipe if completed -- add to work_queue if completed
                        if(state->num_complete == state->required_num)
                        {
                            int result = add_queue(&work_queue,depend->recipe);
                            if(result)
                            {
                                fprintf(stderr, "%s\n","malloc call failed" );
                                exit(1);
                            }
                            work_count++;
                        }
                    }
                    depend = depend->next;
                }
                break;
            }
            temp = temp->next;
        }
        if(temp == NULL)
        {
            fprintf(stderr, "%s\n", "Could not find the finished recipe in the list");
            exit(1);
        }
    }
}


int process_recipe(COOKBOOK *cb,char *main_recipe_name,int max_cooks)
{


    RECIPE *main_recipe_ptr = cb->recipes;
    //search for recipe or use first recipe if no given recipe
    while(main_recipe_ptr != NULL && main_recipe_name != NULL)
    {
        //they are equal names
        if(strcmp(main_recipe_name,main_recipe_ptr->name) == 0)
        {
           // debug("found recipe = %s",main_recipe_ptr->name);
            break;
        }
        main_recipe_ptr = main_recipe_ptr->next;
    }

    //did not find recipe name in cookbook
    if(main_recipe_ptr == NULL)
    {
        fprintf(stderr, "%s\n","DID NOT FIND RECIPE NAME IN COOKBOOK");
        return 1;
    }
     debug("234work_count %d",work_count);

    //recursive traversal of cookbook (analysis)
    //left-right-root
    RECIPE_LINK *visited = NULL;
    traverse_recipe(main_recipe_ptr,visited);

    debug("234work_count %d",work_count);
    //main process loop
    int success = 1;

    RECIPE_LINK *temp = work_queue;
    while(temp != NULL)
    {
        debug("initial work_count = %d",work_count);
        work_count++;
         debug("initial work_count = %d",work_count);
        debug("work_queue = %s",temp->name);
        temp = temp->next;
    }
    /*temp = recipe_req;
    int required_recipe = 0;
    while(temp != NULL)
    {
        required_recipe++;
        temp = temp->next;
    }*/
   // debug("%d",work_count);

    //step up SEGCHILD handler with SIGACTION FUNCTION

    struct sigaction act;

    struct sigaction prev;

    memset(&prev,0,sizeof(struct sigaction));
    memset(&act,0,sizeof(struct sigaction));
    act.sa_handler = process_end;
   // debug("%d",SIGCHLD);

    int sig_result = sigaction(SIGCHLD,&act,&prev);
    if(sig_result == -1)
    {
        fprintf(stderr, "%s\n", "sigaction call failed");
        exit(1);
    }

    //step up mask
    sigset_t mask;
    int sigempty_result = sigemptyset(&mask);
    int sigadd_result = sigaddset(&mask,SIGCHLD);

    sigset_t empty;
    int sig_empt_result = sigemptyset(&empty);

    if(sigempty_result == -1 || sig_empt_result == -1 || sigadd_result == -1)
    {
        fprintf(stderr, "%s\n", "call to sigempty or sigaddset failed");
        exit(1);
    }


    while(1)
    {
        //no recipes left to process
        if(work_count == 0 && num_process == 0)
        {
            if(success)
                return 0;
            else
                return 1;
        }
        //cook should suspend --sigsuspend() waiting for the completion of one or more recipes
        int procmask_result = sigprocmask(SIG_BLOCK,&mask,NULL);
        if(procmask_result == -1)
        {
            fprintf(stderr, "%s\n", "call to sigprocmask failed");
            exit(1);
        }
        if(num_process == max_cooks)
        {
            debug("before suspend ");
            sigsuspend(&empty);
            procmask_result = sigprocmask(SIG_UNBLOCK,&mask,NULL);
            if(procmask_result == -1)
            {
                fprintf(stderr, "%s\n", "call to sigprocmask failed");
                exit(1);
            }
        }
        procmask_result = sigprocmask(SIG_UNBLOCK,&mask,NULL);
        if(procmask_result == -1)
        {
            fprintf(stderr, "%s\n", "call to sigprocmask failed");
            exit(1);
        }
        //debug("work_count = %d num_process = %d ",work_count,num_process);
        if(num_process < max_cooks && work_count != 0 )
        {
            procmask_result = sigprocmask(SIG_BLOCK,&mask,NULL);
            if(procmask_result == -1)
            {
                fprintf(stderr, "%s\n", "call to sigprocmask failed");
                exit(1);
            }

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
            debug("work_count = %d",work_count);
           // debug("processing_recipe = %s",removed->name);
            work_count--;
              debug("work_count = %d",work_count);
           // debug("work_count = %d",work_count);
            num_process++;

            pid_t pid = pid = fork_check();

             //child process code
            if( pid == 0)
            {
              //  debug("processing_recipe = %s",removed->name);
                procmask_result = sigprocmask(SIG_UNBLOCK,&mask,NULL);
                if(procmask_result == -1)
                {
                    fprintf(stderr, "%s\n", "call to sigprocmask failed");
                    exit(1);
                }

                // for each task set up a process --fork() -running concurrently
                TASK *tasks = removed->tasks;

                while(tasks != NULL)
                {
                    STEP *steps = tasks->steps;
                    int input = 0;
                    int output = 0;

                    if(tasks->input_file != NULL)
                    {
                        input = open(tasks->input_file,O_RDONLY);
                        debug("input file = %s",tasks->input_file);
                    }
                    if(tasks->output_file != NULL)
                    {
                        output = open(tasks->output_file,O_WRONLY | O_TRUNC | O_CREAT ,  S_IRWXU );
                        debug("output file = %s",tasks->output_file);
                    }
                    if(input == -1 || output == -1)
                    {
                        fprintf(stderr, "%s\n", "ERROR WITH OPEN FUNCTION");
                        exit(1);
                    }


                    int fd[2];
                    int fd2[2];
                    int pipe_result = pipe(fd);
                    int pipe_result2 = pipe(fd2);

                    if(pipe_result2 == -1 || pipe_result == -1)
                    {
                        fprintf(stderr, "%s\n", "pipe function failed");
                        exit(1);
                    }


                    int process_num = 0;
                    int pid_step = 0;

                    while(steps != NULL)
                    {

                        if(process_num != 0 && process_num % 2 == 0)
                        {
                            int close_result = close(fd[0]);
                            int close_result2 = close(fd[1]);

                            if(close_result == -1 || close_result2 == -1)
                            {
                                fprintf(stderr, "%s\n", "close function failed");
                                exit(1);
                            }

                           // debug(" old pipe = fd[0] = %d fd[1] = %d",fd[0],fd[1]);
                            pipe_result = pipe(fd);
                            if(pipe_result == -1)
                            {
                                fprintf(stderr, "%s\n", "pipe function failed");
                                exit(1);
                            }
                           // debug(" new pipe = fd[0] = %d fd[1] = %d",fd[0],fd[1]);
                        }
                        else if(process_num != 1 && process_num % 2 == 1)
                        {
                            int close_result = close(fd2[0]);
                            int close_result2 = close(fd2[1]);
                            if(close_result == -1 || close_result2 == -1)
                            {
                                fprintf(stderr, "%s\n", "close function failed");
                                exit(1);
                            }
                          //  debug(" old pipe = fd2[0] = %d fd2[1] = %d",fd2[0],fd2[1]);
                            pipe_result = pipe(fd2);
                            if(pipe_result == -1)
                            {
                                fprintf(stderr, "%s\n", "pipe function failed");
                                exit(1);
                            }
                         //   debug("new pipe = fd2[0] = %d fd2[1] = %d",fd2[0],fd2[1]);
                        }

                        sigaction(SIGCHLD,&prev,0);


                        pid_step = fork_check();



                        if(pid_step == 0)
                        { //child code
                            int dup_result = 0;
                            if(process_num == 0)
                            {
                                debug("1");
                                if(input != 0)
                                {
                                    dup_result = dup2(input,0);
                                    if(dup_result == -1)
                                    {
                                        fprintf(stderr, "%s\n", "dup2 function failed");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    dup_result = dup2(fd2[0],0);
                                    if(dup_result == -1)
                                    {
                                        fprintf(stderr, "%s\n", "dup2 function failed");
                                        exit(1);
                                    }
                                }
                                if(steps->next != NULL)
                                {
                                    debug("Executed");
                                    dup_result = dup2(fd[1],1);
                                    if(dup_result == -1)
                                    {
                                        fprintf(stderr, "%s\n", "dup2 function failed");
                                        exit(1);
                                    }
                                }
                                else
                                {
                                    if(output != 0)
                                    {
                                        debug("output = %d",output);
                                        dup_result = dup2(output,1);
                                        if(dup_result == -1)
                                        {
                                            fprintf(stderr, "%s\n", "dup2 function failed");
                                            exit(1);
                                        }
                                    }
                                }
                            }
                            else if(process_num % 2 == 1)
                            {
                                debug("2");
                                dup_result = dup2(fd[0],0);
                                if(dup_result == -1)
                                {
                                    fprintf(stderr, "%s\n", "dup2 function failed");
                                    exit(1);
                                }
                                if(steps->next != NULL)
                                {
                                   // debug("inner pipe1");
                                    dup_result = dup2(fd2[1],1);
                                    if(dup_result == -1)
                                    {
                                        fprintf(stderr, "%s\n", "dup2 function failed");
                                        exit(1);
                                    }
                                }
                                else
                                {  // debug("output1 = %d",output);
                                    if(output != 0){
                                        dup_result = dup2(output,1);
                                        if(dup_result == -1)
                                        {
                                            fprintf(stderr, "%s\n", "dup2 function failed");
                                            exit(1);
                                        }
                                    }
                                }
                            }
                            else if(process_num % 2 == 0)
                            {
                                debug("3");
                                dup_result = dup2(fd2[0],0);
                                if(dup_result == -1)
                                {
                                    fprintf(stderr, "%s\n", "dup2 function failed");
                                    exit(1);
                                }
                                if(steps->next != NULL)
                                {
                                   // debug("inner pipe0");
                                    dup_result = dup2(fd[1],1);
                                    if(dup_result == -1)
                                    {
                                        fprintf(stderr, "%s\n", "dup2 function failed");
                                        exit(1);
                                    }

                                }
                                else
                                {
                                    //debug("output0 = %d",output);
                                    if(output != 0){

                                        dup_result = dup2(output,1);
                                        if(dup_result == -1)
                                        {
                                            fprintf(stderr, "%s\n", "dup2 function failed");
                                            exit(1);
                                        }
                                    }
                                }
                            }
                            int close1 = close(fd[1]);
                            int close2 = close(fd[0]);
                            int close3 = close(fd2[0]);
                            int close4 = close(fd2[1]);
                            if(close1 == -1 || close2 == -1 || close3 == -1 || close4 == -1)
                            {
                                fprintf(stderr, "%s\n","close function failed" );
                                exit(1);
                            }

/*
                            if( strcmp(*steps->words,"echo") == 0)
                            {

                                char b[100] = "echo";

                                execvp(b,steps->words);
                            }
                            else
                            {*/

                                char s[100] = "util/";
                                strcat(s,*steps->words);
                                int rez = execvp(s,steps->words);
                                if( rez == -1 )
                                {
                                    //perror(*steps->words);
                                    int rez2 = execvp(*steps->words,steps->words);
                                    if(rez2 == -1)
                                        {
                                            fprintf(stderr, "%s%s\n","unable to find executable file named: ", *(steps->words));
                                            exit(1);
                                        }

                                }
                            //}
                           // exit(1);

                        }
                        process_num++;
                        //debug("arguments  %s %s",*steps->words,*((steps->words)+1));
                        steps = steps->next;
                    }
                    int status_task;
                    int task_pid = 0;

                    while( (task_pid = wait(&status_task))   > 0)
                    {
                        if(WEXITSTATUS(status_task))
                        {
                            fprintf(stderr, "%s\n","PROCESS DID NOT NORMALLY TERMINATED" );
                            exit(1);
                        }
                    }
                    tasks = tasks->next;
                }
                exit(0);
            }
            //fork returns pid of child to parent
            //parent process code
            else
            {
                ((STATE_INFO *)removed->state)->pid = pid;
                int procmask_result = sigprocmask(SIG_UNBLOCK,&mask,NULL);
                if(procmask_result == -1)
                {
                    fprintf(stderr, "%s\n", "sigprocmask call failed");
                    exit(1);
                }
            }

        }
    }
    return 0;
}

