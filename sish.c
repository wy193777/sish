/*
 * sish.c
 *
 *  Created on: Dec 9, 2015
 *      Author: Shenghan
 */
#include "sish.h"

extern int to_stderr;
extern int f_given_c;
extern char * given_c;
int token_position;
int token_size;
char *input_error;
char ** tokens;
int last_status = 0;
pid_t pid_exe;


void init() {
    pid_t shell_pgid;
    //struct termios shell_tmodes;
    //int shell_terminal;

    /* See if we are running interactively.  */
    //shell_terminal = STDIN_FILENO;

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
    /*if(signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
    	perror("reset SIGCHLD");
		exit(EXIT_FAILURE);
    }*/

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
		return NULL;
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
						return NULL;
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
						return NULL;
					}
				}
				buf[cur++] = ch;
				if(cur >= input_size) {
					input_size += BUFSIZE;
					if ((buf = realloc(buf, input_size)) == NULL) {
						perror("buf realloc");
						return NULL;
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
				return NULL;
			}
		}
	}
}

void split_input(char *line) {
	token_position = 0;

	tokens[token_position] = strtok(line, "\t ");
	while(tokens[token_position] != NULL) {
		token_position++;
		if(token_position >= token_size) {
			token_size += BUFSIZE;
			if((tokens = realloc(tokens, sizeof(char*) * token_size)) == NULL) {
				perror("tokens realloc");
				return;
			}
		}

		/* split ">" operator */
		if(strcmp(tokens[token_position-1], ">>") != 0 && 
				strcmp(tokens[token_position-1], ">") != 0 &&
						tokens[token_position-1][0] == '>') {
			if((tokens[token_position] = malloc(sizeof(tokens[token_position-1]))) == NULL) {
				perror("tokens[token_position] malloc");
				return;
			}
			//the ">" operator is in tokens[token_position-1]
			strcpy(tokens[token_position], &tokens[token_position-1][1]);
			tokens[token_position-1][1] = '\0';
			token_position++;
			if(token_position >= token_size) {
				token_size += BUFSIZE;
				if((tokens = realloc(tokens, sizeof(char*) * token_size)) == NULL) {
					perror("tokens realloc");
					return;
				}
			}
		}
		
		tokens[token_position] = strtok(NULL, "\t ");
	}
}

void builtins_cd(){

	if(token_position > 2) {
    	printf("usage: cd [dir]\n");
    	return;
    }

    char *dir;
    char *user;

	if((dir = malloc(sizeof(char) * BUFSIZE * 4)) == NULL) {
    	perror("malloc dir");
    	last_status = CANNOT_EXECUTE;
    	return;
    }

    /* 
	 *  when [dir] is not specified or equals to "~",
	 *  change directory to the user's home directory
     */
    if(token_position == 1 || 
    	(token_position == 2 && strcmp(tokens[token_position-1], "~") == 0)) {
    	strcpy(dir, "/home/");
    	if((user = getlogin()) == NULL) {
    		perror("get username");
    		last_status = CANNOT_EXECUTE;
    		return;
    	}
    	strcat(dir, user);
    }
    /*
	 *  when [dir] is "~username",
	 *  change directory to "/home/username"
     */
    else if(tokens[token_position-1][0] == '~') {
    	strcpy(dir, "/home/");
    	if(tokens[token_position-1][1] == '/') {
    		if((user = getlogin()) == NULL) {
    			perror("get username");
    			last_status = CANNOT_EXECUTE;
    			return;
    		}
    		strcat(dir, user);
    	}
    	strcat(dir, &tokens[token_position-1][1]);
    }
    /* change directory to [dir] */
    else{
    	strcpy(dir, tokens[token_position-1]);
    }
    if(chdir(dir) == -1) {
    	perror("cd");
    	last_status = CANNOT_EXECUTE;
    	return;
    }

    last_status = 0;

    //debug information
    char *curdir;
    if((curdir = malloc(BUFSIZE * sizeof(char))) == NULL) {
    	perror("malloc curdir");
    	return;
    }
    getcwd(curdir,BUFSIZE);
    printf("current working directory: %s\n", curdir);
}

void builtins_echo() {
	pid_t current_pid = getpid();
	int i, j;
	for(i = 1; i < token_position; i++) {
		int len = strlen(tokens[i]);
		for(j = 0; j < len; j++) {
			if (tokens[i][j] == '$' && j < len-1) {
				if(tokens[i][j+1] == '$') {
					printf("%llu", (unsigned long long)current_pid);
					j++;
				}
				else if (tokens[i][j+1] == '?') {
					printf("%d", last_status);
					j++;
				}
				else
					printf("%c", tokens[i][j]);
			}
			else
				printf("%c", tokens[i][j]);
		}
		printf(" ");
	}
	printf("\n");
	last_status = 0;
}


void loop() {
    char * line;
    //size_t size = 0;
    int i;

    while (1) {
    	//print a command-line prompt
        printf("sish$ ");

        //get an input line
        if((line = getinput()) == NULL) {
        	continue;
        }

        token_size = BUFSIZE;
        if((tokens = malloc(sizeof(char*) * token_size)) == NULL) {
			perror("tokens malloc");
			continue;
		}

		//split input into tokens
        split_input(line);

        //no non-empty tokens
        if(token_position == 0) {
        	continue;
        }

        //builtins: exit
        if(strcmp(tokens[0], "exit") == 0) {
        	if (token_position == 1) {
        		last_status = 0;
        		exit(EXIT_SUCCESS);
        	}
        	else {
        		printf("exit: too many arguments\n");
        		last_status = CANNOT_EXECUTE;
        		continue;
        	}
        }

        //builtins: cd
        if(strcmp(tokens[0], "cd") == 0) {
        	builtins_cd();
        	continue;
        }

        //builtins: echo
       	if(strcmp(tokens[0], "echo") == 0) {
       		builtins_echo();
       		continue;
       	}

       	if((pid_exe = fork()) == -1) {
       		perror("fork error");
       		last_status = CANNOT_EXECUTE;
       		continue;
       	}
       	else if(pid_exe == 0) {	//child
       		taskNode *taskHead;
       		if((taskHead = malloc(sizeof(taskNode))) == NULL) {
       			perror("malloc to data structure");
       			exit(EXIT_FAILURE);
       		}
       		taskHead->out_method = OUT_STD;

       		for(i = 0; i < token_position; i++) {
       			//non-operator token
       			if(strcmp(tokens[i], "<") != 0 && strcmp(tokens[i], ">") != 0 && 
       				strcmp(tokens[i], ">>") != 0 && strcmp(tokens[i], "&") != 0 && 
       				 strcmp(tokens[i], "|") != 0) {
       					taskHead->command[i] = tokens[i];
       			}
       			else {
       				taskHead->command[i] = NULL;
       				break;
       			}
        	}
        	execvp(taskHead->command[0], taskHead->command);
        	fprintf(stderr, "couldn't execute %s: %s\n", tokens[0], strerror(errno));
        	exit(CANNOT_EXECUTE);

       	}
       	else { //parent
       		int status;
       		if(waitpid(pid_exe, &status, 0) < 0) {
       			perror("waitpid");
       		}
       		last_status = WEXITSTATUS(status);
       	}   	
    }
}
