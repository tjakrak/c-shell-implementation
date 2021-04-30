/**
* @file
* This ui.c file is implementing the user interface for mini shell
* It is showing the directory, username, bad/good status, command number and the host name
*/

#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <pwd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

// prompt status
static const char *good_str = "😌";
static const char *bad_str  = "🤯";

static int readline_init(void);
static bool scripting = false;

// status number indicating it is a valid command or not
int status_num;

// a variable to get the current working directory
static char curr_dir[PATH_MAX];
static char hostnm[HOST_NAME_MAX];

int cmd_number = 0;
unsigned int down_up_index = 1;
int current_down_up_index;

bool switch_up = false;
bool switch_down = false;

//struct passwd *pw;
//
char *line = NULL;
size_t line_sz = 0;

void init_ui(void)
{
    LOGP("Initializing UI...\n");

    char *locale = setlocale(LC_ALL, "en_US.UTF-8");
    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");


    if (isatty(STDIN_FILENO) == false) {
        LOGP("Entering scripting mode\n");
        scripting = true;
    }

    rl_startup_hook = readline_init;
}

void destroy_ui(void)
{
    // TODO cleanup code, if necessary

    free(line);
}

char *prompt_line(void)
{
    const char *status = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%u", prompt_cmd_num());

    char *user = prompt_username();
    char *host = prompt_hostname();
    char *cwd = prompt_cwd();

    char *format_str = ">>-[%s]-[%s]-[%s@%s:%s]-> ";

    size_t prompt_sz
        = strlen(format_str)
        + strlen(status)
        + strlen(cmd_num)
        + strlen(user)
        + strlen(host)
        + strlen(cwd)
        + 1;

    char *prompt_str =  malloc(sizeof(char) * prompt_sz);

    snprintf(prompt_str, prompt_sz, format_str,
            status,
            cmd_num,
            user,
            host,
            cwd);

    return prompt_str;
}

char *prompt_username(void)
{
    char *user = getlogin();
    return user;
}

char *prompt_hostname(void)
{
    //hostnm = malloc(HOST_NAME_MAX);
    
    int success = gethostname(hostnm, HOST_NAME_MAX);

    if (success == 0) {
        return hostnm;
    } else {
        return "unknown_host";
    }
}

char *prompt_cwd(void)
{
    // getting the current directory
    getcwd(curr_dir, PATH_MAX);

    // get the home directory
    char *home_directory = getenv("HOME");

    // true if the current directory contains home directory
    bool home = true;

    // check if there is home directory in the cwd
    // if there is, replace it with ~
    int i;
    for (i = 0; i < strlen(home_directory); i++) {
        if (home_directory[i] == curr_dir[i]) {
            //LOG("home: %c\n", home_directory[i]);
            //LOG("cwdhome: %c\n", cwd[i]);
            continue;
        } else {
            home = false;
        }
    }

    // if home directory exist then start printing from ~
    if(home) {
        curr_dir[i - 1] = '~';
        char *p = &curr_dir[i - 1];
        strcpy(curr_dir, p);
    }

    return curr_dir;
}

int set_status(int status) {
    status_num = status;
    LOG("status: %d\n", status_num);
    return 0;
}

int prompt_status(void)
{
    return status_num;
}

unsigned int prompt_cmd_num(void)
{
    return cmd_number;
}

char *read_command(void)
{
    // if it is not scripting mode
    if (scripting == false) {
        cmd_number++;
        char *prompt = prompt_line();
        char *command = readline(prompt);
        free(prompt);
        return command;

    // if this is scripting mode
    } else {
        ssize_t read_sz = getline(&line, &line_sz, stdin);

        if (read_sz == -1) {
            perror("getline");
            return NULL;
        }
        line[read_sz - 1] = '\0';

        return line;
    }
}

int readline_init(void)
{
    rl_bind_keyseq("\\e[A", key_up);
    rl_bind_keyseq("\\e[B", key_down);
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    return 0;
}

int key_up(int count, int key)
{
    char *prev_command;
    
    if (switch_down) {
        current_down_up_index = hist_last_cnum() - down_up_index;
    } else {
        current_down_up_index = hist_last_cnum() + 1 - down_up_index;
    }
    prev_command = (char *)hist_search_cnum(current_down_up_index);

    /* Modify the command entry text: */
    rl_replace_line(prev_command, 1);
    down_up_index++;

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    switch_up = true;
    switch_down = false;

    // TODO: step back through the history until no more history entries are
    // left. Once the end of the history is reached, stop updating the command
    // line.

    return 0;
}

int key_down(int count, int key)
{
    char *next_command;
    
    if (switch_up) {
        down_up_index = down_up_index - 2;
    } else {
        down_up_index = down_up_index - 1;
    }
    
    if (down_up_index < hist_last_cnum() - 1) {
        current_down_up_index = hist_last_cnum() + 1 - down_up_index;
        next_command = (char *)hist_search_cnum(current_down_up_index);
    }

    if (next_command[0] != '\0') {
        /* Modify the command entry text: */
        rl_replace_line(next_command, 1);
    }

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    switch_down = true;
    switch_up = false;

    // TODO: step forward through the history (assuming we have stepped back
    // previously). Going past the most recent history command blanks out the
    // command line to allow the user to type a new command.

    return 0;
}
