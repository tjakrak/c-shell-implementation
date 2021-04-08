/**
* @file
* This file is the driver.
* It is calling history.c to retrieve the history from the commands
* and ui.c which implement the UI for this mini shell
* It supported some basic commands such as: cd, !prefix, !num, !!, exit, history and job.
* The main function is calling the ui function and repeatedly
* reading the command, tokenize and execute it either in background or foreground
*/

#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"
#include "next_token.c"
#include "elist.h"

bool background = false;
struct passwd *pw;

// The struct to contain children that are running in the background
struct jobs_list {
    pid_t pid;
    char job_command[256];
} bg_job;

struct elist *ls_job; 

/**
 * This is sigchildhandler function which get the pid of the children
 * that are running in background finish executing. Then removing them from the elist
 */
void jobs_sigchld (int sig_num)
{
    int status;
    pid_t child_pid;
    child_pid = waitpid(-1, &status, WNOHANG);
    
    //struct jobs_list *j = malloc(sizeof(struct jobs_list));
    int job_index = 0;

//    void *temp;
//    while ((temp = elist_get(ls_job, job_index)) != NULL || job_index < elist_size(ls_job)) {
//        struct jobs_list *job = temp;
//        if (child_pid == job->pid) {
//            elist_remove(ls_job, job_index);
//        }
//        job_index++;
//    }

    struct jobs_list *job = elist_get(ls_job, job_index);
    while (job != NULL) {
        if (child_pid == job->pid) {
            elist_remove(ls_job, job_index);
        }
        job_index++;
        job = elist_get(ls_job, job_index);
    }
    
}


/**
 * Retrieves the command from the user, tokenize the commands
 * and execute the commands based on the user input
 * status will return 0 if success
 *
 * @return status
 */

int process_command(char *command, struct elist *list) 
{
        // boolean to check whether it will run built-in command or not. If it is, it wont run execvp
        bool built_in = false;
        int status; // if 0 then success
        char copy_command[strlen(command) + 1]; // the copy of the original command
        strcpy(copy_command, command);
        
//        LOG("HERE IS %c\n", copy_command[0]);
        if (strlen(command) > 0 && copy_command[0] != '!') {
            hist_add(command);
        }

        LOG("Input command: %s\n", command);
        LOG("list size: %zu\n", elist_size(list));

        elist_clear(list);
        

        char *next_tok = command;
        char *curr_tok;
        // getting all the tokens from command separated by \t\n
        while ((curr_tok = next_token(&next_tok, " \t\n")) != NULL) {
            
            // if we find # then we will igore the rest of the commands after it
            if (strchr(curr_tok, '#') != NULL) {
                break;
            }
            
            elist_add(list, &curr_tok);
        }


        LOG("Input command after: %s\n", command);

        // add null pointer to the end of the list
        char *null = (char *) 0;
        elist_add(list, &null);

        LOG("Processed %zu tokens\n", elist_size(list));

        if (elist_size(list) == 1) {
            LOGP("Empty command\n");
        }

        // pointer to the command      
        char **built_in_cmd = elist_get(list, 0);
        char **input_dir;

        if (elist_size(list) > 2) {
            input_dir = elist_get(list, 1);
            long int idx = elist_size(list) - 2;
            if (strcmp(built_in_cmd[idx], "&") == 0) {
                background = true;
                elist_set(list, idx, &null);
            }
        }
   
        //LOG("CHAR? %c\n", *built_in_cmd[0]);
        //LOG("CHAR? %c\n", *(*built_in_cmd + 1));
        //LOG("CHAR? %c\n", *(built_in_cmd[1]));
        //LOG("BUILTINCMD: %s\n", *built_in_cmd);
        
        // check if the input of the user is built in command (cd, history, !, !!, !23, !prefix, exit, jobs)
        if (elist_size(list) != 1 && (strcmp(built_in_cmd[0], "cd") == 0 || strcmp(*built_in_cmd, "history") == 0 ||
                strchr(*built_in_cmd, '!') != NULL || strcmp(*built_in_cmd, "exit") == 0 || strcmp(*built_in_cmd, "jobs") == 0)) {

            // handling change directory    
            if (strcmp(built_in_cmd[0], "cd") == 0) {
                // if the user only type "cd" and didn't specify any specific directory             
                if (elist_size(list) == 2) {
                    pw = getpwuid(getuid());
                    char *directory = pw->pw_dir;
                    chdir(directory);
                // if the user put specific directory
                } else {
                    chdir(*input_dir);
                    if (chdir(*input_dir) == -1) {
                        perror("directory didn't exist");
                    }
                }
    
            // handling history command (showing the list of 100 history)
            } else if (strcmp(*built_in_cmd, "history") == 0) {
                hist_print();
                fflush(stdout);

            // handling executing the last line of command
            } else if (*(*built_in_cmd) == '!' && *(*built_in_cmd + 1) == '!') {
                // getting the index of the last history
                int idx = (int)hist_last_cnum();
                // getting the last command
                const char *hist_val = hist_search_cnum(idx);
                
                if (hist_val != NULL) {
                    char *copy_hist_val = strdup(hist_val);
                    status = process_command(copy_hist_val, list);
                    set_status(status);
                }
                return 0;

            // handling getting the history based on index and prefix    
            } else if (strchr(*built_in_cmd, '!') != NULL) {
                
                // getting the number after the 1st char (!)
                char *start_ptr = *built_in_cmd + 1;
                // converting the input string value from user to integer (considering using strtol or atoi)
                int val = atoi(start_ptr);
                // the command from history
                const char *hist_val;

                // if next character after ! is numeric
                if (val != 0) {
              
                    // get the command from history according to the index (return NULL if idx does not exist)
                    hist_val = hist_search_cnum(val);
                
                } else { // if next character after ! is string/prefix
                
                    // get the command from history according to the prefix
                    hist_val = hist_search_prefix(start_ptr);
                }

                if (hist_val != NULL) {
                    // strdup to cp the hist_val to the memory in heap
                    char *copy_hist_val = strdup(hist_val);
                    // send the command to process_command() function to be tokenized and run
                    status = process_command(copy_hist_val, list);
                    set_status(status);
                }
                return 0;
            
            // handling jobs command (listing all the background jobs that are currently running)
            } else if (strcmp(*built_in_cmd, "jobs") == 0) {
                int job_index = 0;
                struct jobs_list *job = elist_get(ls_job, job_index);

                while (job != NULL) {
                    printf("%s\n", job->job_command);
                    job_index++;
                    job = elist_get(ls_job, job_index);
                }

            } 

            // handling exit command
            else if (strcmp(*built_in_cmd, "exit") == 0) {
                exit(0);
            } 

            // update boolean flag
            built_in = true;
            status = 0;
            set_status(status);
        }


        // Here are the flags and method to handle I/O redirection
        int open_flags_write = O_RDWR | O_CREAT | O_TRUNC;
        int open_flags_append = O_RDWR | O_CREAT | O_APPEND;
        bool redirect = false;
        bool redirstdin = false;
        int fdout;
        int fdin;

        if (elist_size(list) > 1) {
            
            int num_words;

            if (background) {
                num_words = elist_size(list) - 2;
            } else {
                num_words = elist_size(list) - 1;
            }
            int op_perms = 0666;
            int i;
            for (i = 0; i < num_words; i++) {
                LOG("counter: %i\n", i);
                LOG("BUILTINCMDCHAR: %s\n", built_in_cmd[i]);
                if (strcmp(built_in_cmd[i], ">") == 0) {
                    LOG("filename: %s\n", built_in_cmd[i + 1]);
                    redirect = true;
                    fdout = open(built_in_cmd[i + 1], open_flags_write, op_perms);
                    elist_set(list, i, &null);

                } else if (strcmp(built_in_cmd[i], "<") == 0) {
                    redirect = true;
                    redirstdin = true;
                    fdin = open(built_in_cmd[i + 1], O_RDONLY, op_perms);
                    elist_set(list, i, &null);
                    
                } else if (strcmp(built_in_cmd[i], ">>") == 0) {
                    redirect = true;
                    fdout = open(built_in_cmd[i + 1], open_flags_append, op_perms);
                    elist_set(list, i, &null);
                }
            }
        }


        LOG("stdout fileno %d\n", fileno(stdout));

        if (!built_in) {
            // creating a new process
            pid_t child = fork(); // child will consist of the number of pid
            if (child == -1) {
                perror("fork");
            } else if (child == 0) {
                char **args = elist_get(list, 0);
                if (redirect) {
    
                    // if we are reading input from file
                    if (redirstdin == true) {
                        dup2(fdin, fileno(stdin));
                    }
                    
                    // if we are printing an output to a file
                    dup2(fdout, fileno(stdout));
                }
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                    close(fileno(stdin));
                    close(fileno(stdout));
                    close(fileno(stderr));
                    elist_destroy(list);
                    exit(1);
                }
            } else {
                // if the children not running in the background then the parrent will have to wait
                if (!background) {
                    // wait(&status);
                    waitpid(child, &status, 0);
                    set_status(status);
                    LOG("%d\n", status);
                    elist_clear(list);

                // if the children running in the background then we want to store them in struct and put it in elist    
                } else {

                    // checking if the command is not null
                    if (copy_command[0] != '\0') {
                        bg_job.pid = child;
                        strcpy(bg_job.job_command, copy_command);
                        LOG("JOB COMMAND: %s\n", bg_job.job_command);
                    }
                    
                    //LOG("%i\n", bg_job.pid);
                    elist_add(ls_job, &bg_job);
                    LOG("size of ls_job %zu\n", elist_size(ls_job));
                    background = false;
                }
                if (redirect) {
                    if (redirstdin) {
                        close(fdin);
                    }
                    close(fdout);
                }
            }
        }       

        /* We are done with command; free it */
        set_status(status);
        free(command);
        return 0;
}

/**
 * The main driver of the shell program. Keep looping and calling
 * process_command function to read the user inputs.
 */
int main(void)
{
    hist_init(100);
    init_ui();

    struct elist *args = elist_create(0, sizeof(char **));
    ls_job = elist_create(0, sizeof(struct jobs_list));

    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, jobs_sigchld);

    while (true) {
        char *command = read_command();
        if (command == NULL) {
            break;
        }
        process_command(command, args);
    }

    //destroy_ui();
    hist_destroy();
    elist_destroy(args);
    elist_destroy(ls_job);
    return 0;
}










