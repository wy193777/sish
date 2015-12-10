/*
 * referenced
 * http://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html#Implementing-a-Shell
 * and
 * http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/
 * as tutorials.
 */

#include "sish.h"
int to_stderr = 0;
void usage() {
    printf(
        "sish [ -x][ -c command]\n"
        "-c command Execute the given command\n"
        "-x Enable tracing: Write each command to standard error, preceeded by +\n");
}
int main(int argc, char * argv[]) {
	int ch;
	char * given_c;
    while ((ch = getopt(argc, argv, "c:x")) != -1) {
        switch(ch) {
        case 'c':
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
    init();
    loop();
	return EXIT_SUCCESS;
}
