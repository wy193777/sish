all:
	yacc -d -v -t scanner_tokens.y
	lex parser.l
	cc -Wall -g sish.c main.c lex.yy.c y.tab.c

clean:
	lex.yy.c
	y.tab.c
	y.tab.h
	y.output
