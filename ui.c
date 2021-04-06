#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <pwd.h>

//#include "next_token.c"
#include "history.h"
#include "logger.h"
#include "ui.h"

static const char *good_str = "😌";
static const char *bad_str  = "🤯";

static int readline_init(void);
static bool scripting = false;

int status_num;

static char cwd[PATH_MAX];
static char hostnm[HOST_NAME_MAX];

//static char *hostnm;
//static char *cwd;

int cmd_num = 0;



//struct passwd *pw;

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

}

char *prompt_line(void)
{
    const char *status = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%d", prompt_cmd_num());

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

//    free(hostnm);
//    free(cwd);
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
    getcwd(cwd, PATH_MAX);

    // get the home directory
    char *home_directory = getenv("HOME");

    // true if the current directory contains home directory
    bool home = true;

    // check if there is home directory in the cwd
    // if there is, replace it with ~
    int i;
    for (i = 0; i < strlen(home_directory); i++) {
        if (home_directory[i] == cwd[i]) {
            //LOG("home: %c\n", home_directory[i]);
            //LOG("cwdhome: %c\n", cwd[i]);
            continue;
        } else {
            home = false;
        }
    }

    // if home directory exist then start printing from ~
    if(home) {
        cwd[i - 1] = '~';
        char *p = &cwd[i - 1];
        strcpy(cwd, p);
    }

    return cwd;
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
    return cmd_num;
}

char *read_command(void)
{
    if (scripting == false) {
        cmd_num++;
        char *prompt = prompt_line();
        char *command = readline(prompt);
        //free(cwd);
        free(prompt);
        return command;
    } else {
        char *line = NULL;
        size_t line_sz = 0;
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
    /* Modify the command entry text: */
    rl_replace_line("User pressed 'up' key", 1);

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    // TODO: step back through the history until no more history entries are
    // left. Once the end of the history is reached, stop updating the command
    // line.
    
//    start = rl_point;

//    "\e[A"
//    previous-history (C-p)
//    for (int i = start; i >= start - 100; i--) {
//        rl_line_buffer[i];
//    }

//    int i;
//    for (i = cmd_num; i >= cmd_num - 100; i--) {
//        hist_search_cnum(i);
//    }

    return 0;
}

int key_down(int count, int key)
{
    /* Modify the command entry text: */
    rl_replace_line("User pressed 'down' key", 1);

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    // TODO: step forward through the history (assuming we have stepped back
    // previously). Going past the most recent history command blanks out the
    // command line to allow the user to type a new command.

    return 0;
}
