all:
	lex parser.l
	cc -Wall -g sish.c main.c lex.yy.c

clean:
	rm lex.yy.c
