
CC = gcc
RM = rm -f shell parse *.o

make: shell.o
	$(CC) -o shell shell.o

shell.o: shell.c
	$(CC) -c shell.c

parse: parser_help.o
	$(CC) -o parse parser_help.o

parser_help.o: parser_help.c
	$(CC) -c parser_help.c

clean: 
	$(RM)
