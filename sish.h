#ifndef SISH_H_
#define SISH_H_
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "y.tab.h"
/* A process is a single process.  */
typedef struct task
{
  struct task *next;       /* next process in pipeline */
  char *command;
  char *in_file;
  char *out_file;
  char *append_file;
  int background;
} task;
typedef struct out_append
{
  char * out;
  char * append;
} out_append;
typedef struct redirect
{
  char * in;
  char * out;
  char * append;
} redirect;



void init();
void loop();
struct task *new_task(char*, char*, char*, char*,int, struct task*);
struct out_append *new_o_a(char *, char *);
struct redirect * new_redirect(char*, char*, char*);
void yyerror (char *);

#endif /* SISH_H_ */
