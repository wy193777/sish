/*
 * sish.h
 *
 *  Created on: Dec 9, 2015
 *      Author: Shenghan
 */

#ifndef SISH_H_
#define SISH_H_
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096
#define OUT_STD 7
#define OUT_FILE 8
#define APPEND_FILE 9
#define CANNOT_EXECUTE 127

typedef struct taskNode_ taskNode;

struct taskNode_{
	char *command[BUFSIZE];
	int out_method;
	char *in_file;
	char *out_file;
	char *append_file;
	taskNode *next;
};

void init();
void loop();
char* getinput();
void split_input(char *line);
void builtins_cd();
void builtins_echo();
int handle(taskNode *curr);
int spawn_proc (int, int, taskNode*);


#endif /* SISH_H_ */
