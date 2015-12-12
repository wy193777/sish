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

#define PIPE   100
#define CD     200
#define EXIT   300
#define AECHO  400
#define OUT    500
#define APPEND 600
#define IN     700
#define AFILE  800
#define BG     900

void init();
void loop();

#endif /* SISH_H_ */
