/*
 * sish.c
 *
 *  Created on: Dec 9, 2015
 */
#include "sish.h"

extern int f_to_stderr;
extern int f_given_c;
extern char * given_c;
int token_position;
int token_size;
char *input_error;
char ** tokens;
int last_status = 0;
int debug_ok = 0;
pid_t pid_exe;
taskNode *taskHead = NULL;
int f_background = 0;
pid_t current_pid;


void init() {
    //pid_t shell_pgid;

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        perror("reset SIGINT");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
        perror("reset SIGQUIT");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
        perror("reset SIGTSTP");
        exit(EXIT_FAILURE);
    }

    /* Put ourselves in our own process group.  */
    /*shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("Couldn't put the shell in its own process group");
        exit(1);
    }*/
}

char* getinput() {
    char *buf;
    int ch;
    int cur = 0;
    int input_size = BUFSIZE;
    int i = 0;

    if ((buf = malloc(sizeof(char) * input_size)) == NULL) {
        perror("malloc buf");
        return NULL;
    }

    while (1) {
    	if(f_given_c) {
    		ch = given_c[i];
    		i++;
    	}
    	else
        	ch = getchar();

        if(f_given_c && ch == '\0') {
        	buf[cur] = '\0';
        	return buf;
        }
        /* stop when encouter EOF or '\n' */
        if (ch == EOF || ch == '\n') {
            buf[cur] = '\0';
            return buf;
        }
        /* add whitespace for operators */
        else if (ch == '&' || ch == '|' || ch == '<' || ch == '>') {
            /* deal with ">>" */
            if (ch == '>' && cur > 0 && buf[cur - 1] == '>') {
                buf[cur++] = ch;
                if (cur >= input_size) {
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
                if (cur >= input_size) {
                    input_size += BUFSIZE;
                    if ((buf = realloc(buf, input_size)) == NULL) {
                        perror("buf realloc");
                        return NULL;
                    }
                }
                buf[cur++] = ch;
                if (cur >= input_size) {
                    input_size += BUFSIZE;
                    if ((buf = realloc(buf, input_size)) == NULL) {
                        perror("buf realloc");
                        return NULL;
                    }
                }
                if (ch != '>')
                    buf[cur++] = ' ';
            }
        }
        /* non-operator character */
        else
            buf[cur++] = ch;

        /* reallocate memory if exceed */
        if (cur >= input_size) {
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
    while (tokens[token_position] != NULL) {
        token_position++;
        if (token_position >= token_size) {
            token_size += BUFSIZE;
            if ((tokens = realloc(tokens, sizeof(char*) * token_size)) == NULL) {
                perror("tokens realloc");
                return;
            }
        }

        /* split ">" operator */
        if (strcmp(tokens[token_position - 1], ">>") != 0
                && strcmp(tokens[token_position - 1], ">") != 0
                && tokens[token_position - 1][0] == '>') {
            if ((tokens[token_position] = malloc(
                    sizeof(tokens[token_position - 1]))) == NULL) {
                perror("tokens[token_position] malloc");
                return;
            }
            //the ">" operator is in tokens[token_position-1]
            strcpy(tokens[token_position], &tokens[token_position - 1][1]);
            tokens[token_position - 1][1] = '\0';
            token_position++;
            if (token_position >= token_size) {
                token_size += BUFSIZE;
                if ((tokens = realloc(tokens, sizeof(char*) * token_size))
                        == NULL) {
                    perror("tokens realloc");
                    return;
                }
            }
        }

        tokens[token_position] = strtok(NULL, "\t ");
    }
}

void builtins_cd() {
    int i;
    if (f_to_stderr) {
        fprintf(stderr, "+ ");
        for (i = 0; i < token_position; i++)
            fprintf(stderr, "%s ", tokens[i]);
        fprintf(stderr, "\n");
    }

    if (token_position > 2) {
        printf("cd: too many arguments\n");
        return;
    }

    char *dir;
    char *user;

    if ((dir = malloc(sizeof(char) * BUFSIZE * 4)) == NULL) {
        perror("malloc dir");
        last_status = CANNOT_EXECUTE;
        return;
    }

    /*
     *  when [dir] is not specified or equals to "~",
     *  change directory to the user's home directory
     */
    if (token_position == 1
            || (token_position == 2
                    && strcmp(tokens[token_position - 1], "~") == 0)) {
        strcpy(dir, "/home/");
        if ((user = getlogin()) == NULL) {
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
    else if (tokens[token_position - 1][0] == '~') {
        strcpy(dir, "/home/");
        if (tokens[token_position - 1][1] == '/') {
            if ((user = getlogin()) == NULL) {
                perror("get username");
                last_status = CANNOT_EXECUTE;
                return;
            }
            strcat(dir, user);
        }
        strcat(dir, &tokens[token_position - 1][1]);
    }
    /* change directory to [dir] */
    else {
        strcpy(dir, tokens[token_position - 1]);
    }
    if (chdir(dir) == -1) {
        perror("cd");
        last_status = CANNOT_EXECUTE;
        return;
    }

    last_status = 0;
}

void builtins_echo(taskNode *cur) {
    int i, j;

    for (i = 1; cur->command[i]; i++) {
        int len = strlen(cur->command[i]);
        for (j = 0; j < len; j++) {
            if (cur->command[i][j] == '$' && j < len - 1) {
                if (cur->command[i][j + 1] == '$') {
                    printf("%llu", (unsigned long long) current_pid);
                    j++;
                } else if (cur->command[i][j + 1] == '?') {
                    printf("%d", last_status);
                    j++;
                } else
                    printf("%c", cur->command[i][j]);
            } else
                printf("%c", cur->command[i][j]);
        }
        printf(" ");
    }
    printf("\n");
    last_status = 0;
    exit(EXIT_SUCCESS);
}

int makeTask(taskNode *cur) {

    int command_pos = 0;
    int i;

    cur->out_method = OUT_STD;
    cur->next = NULL;

    //debug information

    for (i = 0; i < token_position; i++) {
        //non-operator token
        if (strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0
                || strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], "&") == 0
                || strcmp(tokens[i], "|") == 0) {
            //when encounter pipe, create a new task node
            if (strcmp(tokens[i], "|") == 0) {
                cur->command[command_pos] = NULL;
                if (cur->command[0] == NULL) {
                    printf("shell: syntax error near '|'\n");
                    last_status = CANNOT_EXECUTE;
                    return -1;
                }
                command_pos = 0;
                taskNode *newNode;
                if ((newNode = malloc(sizeof(taskNode))) == NULL) {
                    perror("can't malloc");
                    last_status = CANNOT_EXECUTE;
                    return -1;
                }
                newNode->out_method = OUT_STD;
                newNode->next = NULL;
                cur->next = newNode;
                if (taskHead == NULL) {
                    taskHead = cur;
                }
                cur = cur->next;
            }
            //update input file
            else if (strcmp(tokens[i], "<") == 0) {
                cur->in_file = tokens[++i];
            }
            //update output file
            else if (strcmp(tokens[i], ">") == 0) {
                cur->out_method = OUT_FILE;
                cur->out_file = tokens[++i];
            }
            //update append file
            else if (strcmp(tokens[i], ">>") == 0) {
                cur->out_method = APPEND_FILE;
                cur->append_file = tokens[++i];
            }
            //background operator
            else {
                if (i != token_position - 1) {
                    printf("shell: syntax error near '&'\n");
                    last_status = CANNOT_EXECUTE;
                    return -1;
                } else {
                    f_background = 1;
                }
            }
        } else {
            cur->command[command_pos++] = tokens[i];
        }
    }
    //if(cur->command[command_pos] != NULL) {
    cur->command[command_pos] = NULL;
    if (cur->command[0] == NULL) {
        printf("shell: syntax error\n");
        last_status = CANNOT_EXECUTE;
        return -1;
    }
    //}
    if (taskHead == NULL)
        taskHead = cur;
    cur = taskHead;

    /* -x option */
    while (cur && f_to_stderr) {
        fprintf(stderr, "+ ");
        for (i = 0; cur->command[i]; i++)
            fprintf(stderr, "%s ", cur->command[i]);
        fprintf(stderr, "\n");
        cur = cur->next;
    }
    cur = taskHead;
    return 0;
}

void loop() {
    char * line;
    current_pid = getpid();
    /*if (f_given_c) {
        line = given_c;
    }*/
    int i;

    while (1) {
        //print a command-line prompt
        if (f_given_c == 0)
            printf("sish$ ");

        //get an input line
        if ((line = getinput()) == NULL) {
        	if(f_given_c)
        		break;
            continue;
        }

        token_size = BUFSIZE;
        if ((tokens = malloc(sizeof(char*) * token_size)) == NULL) {
            perror("tokens malloc");
            if(f_given_c)
            	break;
            continue;
        }

        //split input into tokens
        split_input(line);

        //no non-empty tokens
        if (token_position == 0) {
        	if(f_given_c)
        		break;
            continue;
        }

        //builtins: exit
    	if (strcmp(tokens[0], "exit") == 0) {
    		if (f_to_stderr) {
    		    fprintf(stderr, "+ ");
    		    for (i = 0; i < token_position; i++)
    		        fprintf(stderr, "%s ", tokens[i]);
    		    fprintf(stderr, "\n");
    		}
        	if (token_position == 1) {
        	    last_status = 0;
        	    exit(EXIT_SUCCESS);
        	} else {
        	    printf("exit: too many arguments\n");
        	    last_status = CANNOT_EXECUTE;
        	    if(f_given_c)
        	    	break;
        	    continue;
        	}
		}

		//builtins: cd
    	if (strcmp(tokens[0], "cd") == 0) {
    	    builtins_cd();
    	    if(f_given_c)
    	    	break;
    	    continue;
    	}

        //cur will point to taskHead
        taskNode *cur;
        if ((cur = malloc(sizeof(taskNode))) == NULL) {
            perror("can't malloc");
            exit(CANNOT_EXECUTE);
        }
        f_background = 0;
        taskHead = NULL;
        if (makeTask(cur) == -1) {
        	if(f_given_c)
        		break;
            continue;
        }
        taskNode * head = cur;
        if ((pid_exe = fork()) == -1) {
            perror("fork error");
            last_status = CANNOT_EXECUTE;
            if(f_given_c)
            	break;
            continue;
        }
        //execute commands
        else if (pid_exe == 0) {	//child
            if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
                perror("reset SIGINT");
                exit(EXIT_FAILURE);
            }
            if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
                perror("reset SIGQUIT");
                exit(EXIT_FAILURE);
            }
            if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
                perror("reset SIGTSTP");
                exit(EXIT_FAILURE);
            }

            if (f_background) {
                setpgid(0, 0);
            }

            handle(cur);
        } else { //parent
            int status;
            if (f_background == 0) {
                if (waitpid(pid_exe, &status, 0) < 0) {
                    perror("waitpid");
                }
            }
            //get the exit status of last command
            last_status = WEXITSTATUS(status);
        }

        if (f_given_c)
            break;
        else {
            gc(head);
            free(line);
        }
    }
}

void handle(taskNode *curr) {

    int from_to[2];
    int in;
    int infile_fd;
    int outfile_fd;
    while (curr->next != NULL) {
        //printf("Handle");
        if (pipe(from_to)) {
            perror("pipe");
            exit(CANNOT_EXECUTE);
        }
        spawn_proc(in, from_to[1], curr);

        close(from_to[1]);
        in = from_to[0];
        curr = curr->next;
    }
    /* last command */
    /*deal with input file*/
    if (curr->in_file != NULL) {
        if ((infile_fd = open(curr->in_file, O_RDONLY)) == -1) {
            fprintf(stderr, "Unable to open %s: %s\n", curr->in_file,
                    strerror(errno));
            exit(CANNOT_EXECUTE);
        }
        dup2(infile_fd, STDIN_FILENO);
        close(infile_fd);
    }
    /*redirect to pipe read end*/
    else {
        if (in != STDIN_FILENO)
            dup2(in, STDIN_FILENO);
    }

    /*deal with output and append file*/
    if (curr->out_method != OUT_STD) {
        switch (curr->out_method) {
        case OUT_FILE:
            if ((outfile_fd = open(curr->out_file, O_CREAT | O_WRONLY | O_TRUNC,
                    0666)) == -1) {
                fprintf(stderr, "can't create file %s: %s\n", curr->out_file,
                        strerror(errno));
                exit(CANNOT_EXECUTE);
            }
            break;
        case APPEND_FILE:
            if ((outfile_fd = open(curr->append_file,
                    O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
                fprintf(stderr, "can't create file %s: %s\n", curr->out_file,
                        strerror(errno));
                exit(CANNOT_EXECUTE);
            }
            break;
        default:
            break;
        }
        dup2(outfile_fd, STDOUT_FILENO);
        close(outfile_fd);
    }
    
    //builtins: echo
    if (strcmp(curr->command[0], "echo") == 0) {
        builtins_echo(curr);
        exit(CANNOT_EXECUTE);
    }
    else {
    	execvp(curr->command[0], curr->command);
    	fprintf(stderr, "%s: command not found: %s\n", curr->command[0],
    	        strerror(errno));
    	exit(CANNOT_EXECUTE);
	}
}

void spawn_proc(int in, int out, taskNode *curr) {
    pid_t pid;
    int infile_fd;
    int outfile_fd;

    if ((pid = fork()) == 0) {		//child
        /*deal with input file*/
        if (curr->in_file != NULL) {
            if ((infile_fd = open(curr->in_file, O_RDONLY)) == -1) {
                fprintf(stderr, "Unable to open %s: %s\n", curr->in_file,
                        strerror(errno));
                exit(CANNOT_EXECUTE);
            }
            dup2(infile_fd, STDIN_FILENO);
            close(infile_fd);
        }
        /*redirect to pipe read end*/
        else {
            if (in != STDIN_FILENO) {
                dup2(in, STDIN_FILENO);
                close(in);
            }
        }

        /*deal with output and append file*/
        if (curr->out_method != OUT_STD) {
            switch (curr->out_method) {
            case OUT_FILE:
                if ((outfile_fd = open(curr->out_file,
                        O_CREAT | O_WRONLY | O_TRUNC, 0666)) == -1) {
                    fprintf(stderr, "can't create file %s: %s\n",
                            curr->out_file, strerror(errno));
                    exit(CANNOT_EXECUTE);
                }
                break;
            case APPEND_FILE:
                if ((outfile_fd = open(curr->append_file,
                        O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
                    fprintf(stderr, "can't create file %s: %s\n",
                            curr->out_file, strerror(errno));
                    exit(CANNOT_EXECUTE);
                }
                break;
            default:
                break;
            }
            dup2(outfile_fd, STDOUT_FILENO);
            close(outfile_fd);
        }
        /*rediret to pipe write end*/
        else {
            if (out != STDOUT_FILENO) {
                dup2(out, STDOUT_FILENO);
                close(out);
            }
        }

        //builtins: echo
        if (strcmp(curr->command[0], "echo") == 0) {
            builtins_echo(curr);
            exit(CANNOT_EXECUTE);
        }
        else {
    	    execvp(curr->command[0], curr->command);
    	    fprintf(stderr, "%s: command not found: %s\n", curr->command[0],
    	            strerror(errno));
    	    exit(CANNOT_EXECUTE);
   		}
    } else if (pid < 0) {
        exit(CANNOT_EXECUTE);
    } else {	//parent
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
        }
        //get the exit status of last command
        last_status = WEXITSTATUS(status);
    }
}

// free memory
void gc(taskNode *head) {
    taskNode * curr = head;
    taskNode * next;

    free(tokens);

    while (curr) {
        next = curr->next;
        curr->next = NULL;
        free(curr);
        curr = next;
    }
}
