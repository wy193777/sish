/*
 * sish.h
 *
 *  Created on: Dec 9, 2015
 *      Author: Shenghan
 */

#ifndef SISH_H_
#define SISH_H_
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define BUFSIZE 1024

void init();
void loop();
char* getinput();
void split_input(char *line);
void builtins_cd();
void builtins_echo();


#endif /* SISH_H_ */
