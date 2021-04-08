# Project 2: Command Line Shell

See: https://www.cs.usfca.edu/~mmalensek/cs521/assignments/project-2.html

## What this program do?
This program act as command line interface, creating a mini shell. This program supports all commands that can be executed using <i>execvp</i> and some other basic commands such as: cd, history, !num, !prefix, !!, exit, jobs, & and I/O redirection (<, >, >>).



## Implementation
There are three main parts on this project. Implementing ui.c, history.c and shell.c.

```ui.c``` is mainly implementing the interface of our shell command. This file handles the prompt of our shells that displays:
- Command number (starting from 1)
- User name and host name: ```(username)@(hostname)``` followed by ```:```
- The current working directory
- Process exit status

More importantly, it also check whether the shell is in scripting mode or normal mode and get the commands from the user. The main built in C library functions to implement this are: ```readline, getline``` and ```isatty```.

The second part of the implementation is ```history.c```. This file handles all the functions to store and get the commands from the history. This file is basically a wrapper to a circular array data structure that we are implementing in clist.c file.

The last part and also the driver of this program is the ```shell.c``` file. The biggest part of this program is tokenizing and checking whether the user input contains any executable commands, handling signals and I/O redirection. List of the main functions we use here are: ```execvp, getlogin, gethostname, getpwuid, getcwd, chdir, SIGCHLD, fork()``` and ```waitpid()```.


## Program options

| Commands  | Descriptions                              |
| --------- | ----------------------------------------- |
| cd        | To change the current working directory   |
| #         | Comment out the shell command             |
| history   | Listing the top 100 commands history      |
| !num      | Get history based on the index number     |
| !prefix   | Get history based on the prefix           |
| !!        | Get the last executed command             |
| exit      | To exit the shell                         |
| job       | Listing the current running jobs in the background|
| <         | Reading input from a file                 |
| >         | Writing/rewrite the output to a new file or existing file|
| >>        | Appending the output to an existing file  |
| &         | Running a command in the background       |


## List of files

* <b>shell.c :</b>  This is the main driver of the program. It is calling the ui.c and history.c repeatedly to get, tokenize and execute commands from the user.
* <b>ui.c :</b> This file is mainly responsible for the user interface of the shell.
* <b>history.c :</b> Main function of this file is to store and get command history.
* <b>elist.c :</b> A file that implements dynamic array similar to ArrayList in Java.
* <b>elist.c :</b> This file implements circular array data structure to store the commands from the user
* <b>Makefile</b>: Run and compile the program

## Unit testing
<b>List of tests:</b>
1.  Command Execution - Test whether our program is able to wait and prompt for user input
2.  Scripting - Test the scripting mode
3.  Comments - Test out the comment feature
4.  cd - This test is testing the cd command
5.  exit - This test is testing the exit command
6.  SIGINT handler - Test signal handler. Preventing ctrl + c to kill our shell.
7.  Prompt - testing the prompt of the shell that is implemented in ui.c.
8.  IO Redirection - test I/O redirection (file descriptors)
9.  Small History List - test the history commands with a small sample size
10.  Large History List - testing the history commands with a large data
11.  History !num - Test the program to print the command from the history based on the index
12.  History !prefix - Test the program to print the command from the history based on the prefix
13.  History !! - Test the program to print the last executed command
14.  Background Jobs - Test the shell to be able to execute commands in the background
15.  Job List - Test whether our pr
16.  Documentation - Test whether each files has proper documentation explaining their functionality
17.  Static Analysis - Check if there are any redundant in our code
18.  Leak Check - Check if this program has memory leak

<b>Syntax used to test:</b>

- ```make test``` - To test the whole tests
- ```make test run=x``` - To test specific case of x, where x is the test number
- ```make test run={x, y, ..., z}``` - To test few specific cases where {x, y, ...., z} is the test number
