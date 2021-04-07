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


struct passwd *pw;

int process_command(char *command, struct elist *list) 
{
        //int status;
//        char copy_command[strlen(command) + 1];
//        strcpy(copy_command, command);
        
//        LOG("HERE IS %c\n", copy_command[0]);
//        if (strlen(command) > 0 && copy_command[0] != '!') {
//            hist_add(command);
//        }

        LOG("Input command: %s\n", command);
        LOG("list size: %zu\n", elist_size(list));

        elist_clear(list);
        char *next_tok = command;
        char *curr_tok;

        // getting all the tokens from command separated by \t\n
        while ((curr_tok = next_token(&next_tok, " \t\n")) != NULL) {
            //LOG("curr_tok: %s\n", curr_tok);

            if (strchr(curr_tok, '#') != NULL) {
                break;
            }
            
            elist_add(list, &curr_tok);
        }


        LOG("Input command after: %s\n", command);
//        LOG("Copy command: %s\n", x);
//        LOG("pointercopy: %s\n", y);

        // add null pointer to the end of the list
        char *null = (char *) 0;
        elist_add(list, &null);

        LOG("Processed %zu tokens\n", elist_size(list));

        if (elist_size(list) == 1) {
            LOGP("Empty command\n");
        }

        // This is the place to do pre-processing on the command line
        // if we have > character in the tokens
        //  open the file that came after the >
        
        char **built_in_cmd = elist_get(list, 0);
        char **input_dir;

        if (elist_size(list) > 2) {
            input_dir = elist_get(list, 1); 
        }


        LOG("BUILTINCMD: %s\n", *built_in_cmd);
        if (elist_size(list) != 1) {
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
    
            } else if (strcmp(*built_in_cmd, "history") == 0) {
                hist_print();
                fflush(stdout);
            } else if (strchr(*built_in_cmd, '!') != NULL) {
                char *start_ptr = *built_in_cmd + 1;
                char *end_ptr;
                LOG("the number is: %s\n", start_ptr);
                //long val;
                //val = strtol(start_ptr, end_ptr, 10);
                int val = atoi(start_ptr);

                //if (end_ptr == start_ptr) {
                //    break;
                //}

                const char *hist_val = hist_search_cnum(val);
                char *copy_hist_val = strdup(hist_val);
                LOG("HISTORY VAL: %s\n", copy_hist_val);
                if (hist_val != NULL) {
                    process_command(copy_hist_val, list);
                    return 0;
                }
                
//          } else if (strcmp(*built_in_cmd, "jobs") == 0) {
//              continue;
            } 
            else if (strcmp(*built_in_cmd, "exit") == 0) {
                exit(0);
            } 
        }

//        LOG("THISIS: %s\n", *cmd);
//        LOG("HAHAHA: %s\n", *built_in_cmd);
        int open_flags_write = O_RDWR | O_CREAT | O_TRUNC;
        int open_flags_append = O_RDWR | O_CREAT | O_APPEND;
        int open_perms = 0666;
        bool redirect = false;
        bool redirstdin = false;
        int fdout;
        int fdin;
        int i;
        int index_of_command = 0;
        if (elist_size(list) != 1) {
            for (i = 0; i < elist_size(list) - 1; i++) {
                LOG("counter: %i\n", i);
                LOG("BUILTINCMDCHAR: %s\n", built_in_cmd[i]);

                if (strcmp(built_in_cmd[i], ">") == 0) {
                    LOG("filename: %s\n", built_in_cmd[i + 1]);
                    redirect = true;
                    fdout = open(built_in_cmd[i + 1], open_flags_write, open_perms);
                    elist_set(list, i, &null);
                } else if (strcmp(built_in_cmd[i], "<") == 0) {
                    redirect = true;
                    redirstdin = true;
                    fdin = open(built_in_cmd[i + 1], O_RDONLY, open_perms);
                    elist_set(list, i, &null);
                    //fd = open(built_in_cmd[i - 1], open_flags_write, open_perms);
                    //index_of_command = i + 1;
                } else if (strcmp(built_in_cmd[i], ">>") == 0) {
                    redirect = true;
                    //fd = open(built_in_cmd[i + 1], open_flags_append, open_perms);
                    fdout = open(built_in_cmd[i + 1], open_flags_append, open_perms);
                    elist_set(list, i, &null);
                }
            }
        }

        LOG("stdout fileno %d\n", fileno(stdout));

        // creating a new process
        pid_t child = fork();
        if (child == -1) {
            perror("fork");
        } else if (child == 0) {
            char **args = elist_get(list, index_of_command);
            LOG("thisis: %s\n", *args);
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
            int status;
            wait(&status);
            set_status(status);
            LOG("%d\n", status);
            elist_clear(list);
        }

        /* We are done with command; free it */
        //elist_destroy(list);
        free(command);
        return 0;
}


int main(void)
{
    init_ui();
    //jobs_init();
    hist_init(100);

    struct elist *args = elist_create(0, sizeof(char **));

    signal(SIGINT, SIG_IGN);
    //signal(SIGCHLD, jobs_sigchld);

    while (true) {
        char *command = read_command();
        if (command == NULL) {
            break;
        }

        char copy_command[strlen(command) + 1];
        strcpy(copy_command, command);
        if (strlen(command) > 0 && copy_command[0] != '!') {
            hist_add(command);
        }

        // check if the length is not only ! too
//        char *start_ptr;
        //LOG("START PTR: %s\n", start_ptr);
//        if (copy_command[0] == '!' && strlen(copy_command) > 0) {
//            start_ptr = &copy_command[0] + 1;
//            LOG("START PTR: %s\n", start_ptr);
//            int val = atoi(start_ptr);
            
//            const char *hist_val = hist_search_cnum(val);
//            char *copy_hist_val = strdup(hist_val);
//            char *test = malloc(strlen(copy_hist_val + 1));
            
//            memcpy(test, copy_hist_val, strlen(copy_hist_val + 1));
//            command = test;
//            free(copy_hist_val);
//        }

        process_command(command, args);
}

    //destroy_ui();
    //jobs_destroy();
    hist_destroy();
    elist_destroy(args);

    return 0;
}










