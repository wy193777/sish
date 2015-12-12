/*
 * sish.c
 *
 *  Created on: Dec 9, 2015
 *      Author: Shenghan
 */
#include "sish.h"

extern int to_stderr;
extern int f_given_c;
int token_position;
int token_size;
char *input_error;

void init() {
    pid_t shell_pgid;
    struct termios shell_tmodes;
    int shell_terminal;

    /* See if we are running interactively.  */
    shell_terminal = STDIN_FILENO;

    if(signal(SIGINT, SIG_IGN) == SIG_ERR) {
    	perror("reset SIGINT");
		exit(EXIT_FAILURE);
    }
    if(signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
    	perror("reset SIGQUIT");
		exit(EXIT_FAILURE);
    }
    if(signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
    	perror("reset SIGTSTP");
		exit(EXIT_FAILURE);
    }
    if(signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
    	perror("reset SIGCHLD");
		exit(EXIT_FAILURE);
    }

    /* Put ourselves in our own process group.  */
    shell_pgid = getpid ();
    if (setpgid (shell_pgid, shell_pgid) < 0) {
        perror ("Couldn't put the shell in its own process group");
        exit (1);
    }

//    /* Grab control of the terminal.  */
//    tcsetpgrp (shell_terminal, shell_pgid);
//
//    /* Save default terminal attributes for shell.  */
//    tcgetattr (shell_terminal, &shell_tmodes);
}

char* getinput() {
	char *buf;
	int ch;
	int cur = 0;
	int input_size = BUFSIZE;

	if((buf = malloc(sizeof(char) * input_size)) == NULL) {
		perror("malloc buf");
		exit(EXIT_FAILURE);
	}

	while(1) {
		ch = getchar();

		/* stop when encouter EOF and '\n' */
		if(ch == EOF || ch == '\n') {
			buf[cur] = '\0';
			return buf;
		}
		/* add whitespace for operators */
		else if(ch == '&' || ch == '|' || ch == '<' || ch == '>') {
			/* deal with ">>" */
			if(ch == '>' && cur > 0 && buf[cur-1] == '>'){
				buf[cur++] = ch;
				if(cur >= input_size) {
					input_size += BUFSIZE;
					if ((buf = realloc(buf, input_size)) == NULL) {
						perror("buf realloc");
						exit(EXIT_FAILURE);
					}
				}
				buf[cur++] = ' ';
			}
			/* deal with other operators */
			else {
				buf[cur++] = ' ';
				if(cur >= input_size) {
					input_size += BUFSIZE;
					if ((buf = realloc(buf, input_size)) == NULL) {
						perror("buf realloc");
						exit(EXIT_FAILURE);
					}
				}
				buf[cur++] = ch;
				if(cur >= input_size) {
					input_size += BUFSIZE;
					if ((buf = realloc(buf, input_size)) == NULL) {
						perror("buf realloc");
						exit(EXIT_FAILURE);
					}
				}
				if(ch != '>')
					buf[cur++] = ' ';
				
			}
		}
		/* non-operator character */
		else
			buf[cur++] = ch;

		/* reallocate memory if exceed */
		if(cur >= input_size) {
			input_size += BUFSIZE;
			if ((buf = realloc(buf, input_size)) == NULL) {
				perror("buf realloc");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void split_input(char *line, char ** tokens) {
	char* tmp_token;
	token_position = 0;

	tokens[token_position] = strtok(line, "\t ");
	while(tokens[token_position] != NULL) {
		token_position++;
		if(token_position >= token_size) {
			token_size += BUFSIZE;
			if((tokens = realloc(tokens, sizeof(char*) * token_size)) == NULL) {
				perror("tokens realloc");
				exit(EXIT_FAILURE);
			}
		}

		/* split ">" operator */
		if(strcmp(tokens[token_position-1], ">>") != 0 && 
				strcmp(tokens[token_position-1], ">") != 0 &&
						tokens[token_position-1][0] == '>') {
			if((tokens[token_position] = malloc(sizeof(tokens[token_position-1]))) == NULL) {
				perror("tokens[token_position] malloc");
				exit(EXIT_FAILURE);
			}
			strcpy(tokens[token_position], &tokens[token_position-1][1]);
			tokens[token_position-1][1] = '\0';
			token_position++;
			if(token_position >= token_size) {
				token_size += BUFSIZE;
				if((tokens = realloc(tokens, sizeof(char*) * token_size)) == NULL) {
					perror("tokens realloc");
					exit(EXIT_FAILURE);
				}
			}
		}
		
		tokens[token_position] = strtok(NULL, "\t ");
	}
}


void loop() {
    char * line;
    char ** tokens;
    size_t size = 0;
    int i;

    while (1) {
        printf("sish$ ");
        line = getinput();
        if(strcmp(line, "exit") == 0) {
        	exit(EXIT_SUCCESS);
        }
        if(strlen(line) == 0) {
        	continue;
        }
        else {
        	token_size = BUFSIZE;
        	if((tokens = malloc(sizeof(char*) * token_size)) == NULL) {
				perror("tokens malloc");
				exit(EXIT_FAILURE);
			}
        	split_input(line, tokens);
        	for(i = 0; i < token_position; i++) {
        		printf("%s\n", tokens[i]);
        	}
        }
    }
}
