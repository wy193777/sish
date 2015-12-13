/*
 * sish.c
 *
 *  Created on: Dec 9, 2015
 *      Author: Shenghan
 */
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sish.h"

void init() {
    pid_t shell_pgid;
    struct termios shell_tmodes;
    int shell_terminal;

    //* See if we are running interactively.  */
    shell_terminal = STDIN_FILENO;

    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    signal (SIGCHLD, SIG_IGN);

    /* Put ourselves in our own process group.  */
    shell_pgid = getpid ();
    if (setpgid (shell_pgid, shell_pgid) < 0)
      {
        perror ("Couldn't put the shell in its own process group");
        exit (1);
      }

//    /* Grab control of the terminal.  */
//    tcsetpgrp (shell_terminal, shell_pgid);
//
//    /* Save default terminal attributes for shell.  */
//    tcgetattr (shell_terminal, &shell_tmodes);
}

void loop() {
    char * line = NULL;
    size_t size = 0;
    while (1) {
        printf("sish$ ");
        if (getline(&line, &size, stdin) < 0) {
            perror ("getline:");
            exit(EXIT_FAILURE);
        }

        printf("%s", line);
    }
}

struct task *new_task(char *command, char *in_file, char *out_file, char* append_file,
  int isbg, struct task *next) {
    struct task *a_task = malloc(sizeof (struct task));
    if (a_task == NULL) return NULL;

    a_task->command = command;
    a_task->in_file = in_file;
    a_task->out_file = out_file;
    a_task->append_file = append_file;
    a_task->background = isbg;
    return a_task;
}
struct out_append *new_o_a(char * out, char *append)
{
  struct out_append *oas = malloc(sizeof (struct out_append));
  oas->out = out;
  oas->append = append;
  return oas;
}
struct redirect * new_redirect(char* in , char* out, char* append)
{
  struct  redirect *a_redirect = malloc(sizeof (struct redirect));
  a_redirect->in = in;
  a_redirect->out = out;
  a_redirect->append = append;
  return a_redirect;
}
void yyerror (char *s) {fprintf(stderr, "%s\n", s);}
