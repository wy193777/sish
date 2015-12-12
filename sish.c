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
