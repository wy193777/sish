/*
 * referenced
 * http://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html#Implementing-a-Shell
 * and
 * http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
 * as tutorials.
 */

#include "sish.h"

 /*
  *  option flags
  */
int to_stderr = 0;
int f_given_c = 0;
char * given_c;

void usage() {
    printf(
        "sish [-x] [-c command]\n"
        "-c command Execute the given command\n"
        "-x Enable tracing: Write each command to standard error, preceeded by +\n");
}

int main(int argc, char * argv[]) {
	int ch;
    while ((ch = getopt(argc, argv, "c:x")) != -1) {
        switch(ch) {
        case 'c':
        	f_given_c = 1;
            given_c = optarg;
            break;
        case 'x':
            to_stderr = 1;
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }
    //set SHELL enviroment variable
    char *sish_path;
    if((sish_path = malloc(BUFSIZE * sizeof(char))) == NULL) {
    	perror("malloc curdir");
    	exit(EXIT_FAILURE);
    }
    getcwd(sish_path, BUFSIZE);
    char env[BUFSIZE];
    sprintf(env, "SHELL=%s", sish_path);
    if((putenv(env)) != 0) {
        perror("putenv");
        exit(EXIT_FAILURE);
    }
    init();
    loop();
	exit(EXIT_SUCCESS);
}
