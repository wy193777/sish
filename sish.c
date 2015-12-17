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
//    if (setpgid (shell_pgid, shell_pgid) < 0) {
//        perror ("Couldn't put the shell in its own process group");
//        exit (1);
//    }

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
    /*char *curdir;
    if((curdir = malloc(BUFSIZE * sizeof(char))) == NULL) {
    	perror("malloc curdir");
    	return;
    }
    getcwd(curdir,BUFSIZE);
    printf("current working directory: %s\n", curdir);*/
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

void makeTask(taskNode *taskHead, taskNode *cur, int *f_background) {

	int command_pos = 0;
	int i;

   	cur->out_method = OUT_STD;
   	cur->next = NULL;

   	//debug information

    for(i = 0; i < token_position; i++) {
       	//non-operator token
       	if(strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0 ||
       		strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], "&") == 0 ||
       		 strcmp(tokens[i], "|") == 0) {
       			//when encounter pipe, create a new task node
       			if(strcmp(tokens[i], "|") == 0) {
       				cur->command[command_pos] = NULL;
       				command_pos = 0;
					taskNode *newNode;
					if((newNode = malloc(sizeof(taskNode))) == NULL) {
						perror("can't malloc");
						exit(CANNOT_EXECUTE);
					}
					newNode->out_method = OUT_STD;
					newNode->next = NULL;
					cur->next = newNode;
					if(taskHead == NULL) {
						taskHead = cur;
					}
					cur = cur->next;
       			}
       			//update input file
       			else if(strcmp(tokens[i], "<") == 0) {
       				cur->in_file = tokens[++i];
       			}
       			//update output file
       			else if(strcmp(tokens[i], ">") == 0) {
       				cur->out_method = OUT_FILE;
       				cur->out_file = tokens[++i];
       			}
       			//update append file
       			else if(strcmp(tokens[i], ">>") == 0) {
       				cur->out_method = APPEND_FILE;
       				cur->append_file = tokens[++i];
       			}
       			//background operator
       			else {
       				if(i != token_position-1) {
       					printf("background operator & is in wrong position\n");
       					exit(CANNOT_EXECUTE);
       				}
       				else {
       					*f_background = 1;
       				}
       			}
       		}
       		else {
       			cur->command[command_pos++] = tokens[i];
       		}
        }
       	if(cur->command[command_pos] != NULL)
       		cur->command[command_pos] = NULL;
       	if(taskHead == NULL)
        		taskHead = cur;
        cur = taskHead;
//        while(cur) {
//        	if(cur->command[0] == NULL) {
//        		printf("shell: parse error\n");
//        		exit(CANNOT_EXECUTE);
//        	}
//        	printf("\ncommand: ");
//        	int i;
//        	for(i = 0;cur->command[i];i++)
//        		printf("%s ", cur->command[i]);
//        	printf("\nout_method: %d", cur->out_method);
//        	printf("\nout_file: %s", cur->out_file);
//        	printf("\nin_file: %s", cur->in_file);
//        	printf("\nappend_file: %s\n\n", cur->append_file);
//
//        	cur = cur->next;
//        }
}


void loop() {
    char * line;
    //size_t size = 0;

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
       	//execute commands
       	else if(pid_exe == 0) {	//child
       		taskNode *taskHead = NULL;
       		taskNode *cur;
       		if((cur = malloc(sizeof(taskNode))) == NULL) {
   				perror("can't malloc");
   				exit(CANNOT_EXECUTE);
   			}
       		int f_background = 0;

       		makeTask(taskHead, cur, &f_background);

       		execvp(cur->command[0], cur->command);
        	fprintf(stderr, "couldn't execute %s: %s\n", taskHead->command[0], strerror(errno));
        	//handle(cur);
        	exit(EXIT_SUCCESS);


       	}
       	else { //parent
       		int status;
       		if(waitpid(pid_exe, &status, 0) < 0) {
       			perror("waitpid");
       		}
       		//get the exit status of last command
       		last_status = WEXITSTATUS(status);
       	}
    }
}

int handle(taskNode *curr) {
    int from_to[2];
    int in;
    while (curr->next != NULL) {
        printf("Handle");
        if (pipe(from_to)) {
            perror("pipe");
            exit(CANNOT_EXECUTE);
        }
        spawn_proc(in, from_to[1], curr->next);

        close(from_to[1]);
        in = from_to[0];
        curr = curr->next;
    }
    /* last command */
    if (in != STDIN_FILENO) dup2(in, STDIN_FILENO);
    return execvp(curr->command[0], curr->command);

}
int spawn_proc (int in, int out, taskNode *curr)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != STDIN_FILENO)
        {
          dup2 (in, STDIN_FILENO);
          close (in);
        }

      if (out != STDOUT_FILENO)
        {
          dup2 (out, STDOUT_FILENO);
          close (out);
        }

      return execvp (curr->command[0], curr->command);
    }
  else if(pid < 0) {
      exit(CANNOT_EXECUTE);
  }

  return pid;
}
